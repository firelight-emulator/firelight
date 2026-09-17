// TODO: NEEDS REVIEW
#include "emulation_service_fixture.hpp"

#include <firelight/cheats/sqlite_cheat_repository.hpp>
#include <firelight/library/disc_set_service.hpp>
#include <firelight/library/entry_resolver.hpp>
#include <firelight/library/library_ingest_service.hpp>
#include <firelight/library/sqlite_user_library.hpp>
#include <firelight/library/user_library_service.hpp>
#include <firelight/saves/save_manager_impl.hpp>
#include <firelight/settings/settings_service.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>

#include <QString>
#include <QTemporaryDir>
#include <atomic>
#include <emulation/emulation_service.hpp>
#include <gtest/gtest.h>
#include <libretro/core_registry.hpp>
#include <thread>

namespace firelight::emulation {

// End-to-end flow driven entirely through EmulationService/EmulatorInstance with
// a FakeCore injected via the core factory: load -> initialize -> run frames ->
// save -> rewind (serialize/deserialize) -> reset -> teardown, with NO real
// libretro DLL (so no process-exit on core teardown)
class EmulatorInstanceE2ETest : public EmulationServiceFixture {};

TEST_F(EmulatorInstanceE2ETest, LoadRunSaveRewindResetTeardown) {
  const int entryId = ingestEntry();
  ASSERT_NE(entryId, -1);

  auto *instance = m_emulationService->loadEntry(entryId).get();
  ASSERT_NE(instance, nullptr);
  ASSERT_NE(m_fakeCore, nullptr);

  // Achievements/input services are unset and null-guarded, so initialize()
  // works with just a (null) video receiver
  ASSERT_TRUE(instance->initialize(nullptr));
  ASSERT_TRUE(instance->isInitialized());
  ASSERT_TRUE(m_fakeCore->initialized());

  // Cores get one shared directory for their own writes, held apart from the
  // per-game tree the save manager owns
  const auto expectedSaveDir = (m_saveDir.path() + "/shared").toStdString();
  EXPECT_EQ(m_fakeCore->savedSaveDirectory(), expectedSaveDir);

  // initialize() published EmulationStartedEvent, so the core's declared options
  // should now be cached (keyed by the platform's core) for the advanced editor
  const auto coreName = CoreRegistry::instance().defaultCoreForPlatform(3);
  const auto cachedOptions = m_coreOptionRepo->getCoreOptions(coreName);
  ASSERT_EQ(cachedOptions.size(), 2u);
  EXPECT_EQ(cachedOptions[0].key, "fake_opt_a");
  EXPECT_EQ(cachedOptions[0].defaultValue, "on");
  ASSERT_EQ(cachedOptions[0].values.size(), 2u);
  EXPECT_EQ(cachedOptions[0].values[1].value, "off");

  // Run frames — the fake advances a frame counter and touches SRAM
  for (int i = 0; i < 5; ++i) {
    instance->runFrame();
  }
  EXPECT_EQ(m_fakeCore->frameCount(), 5);

  // Save — SRAM (mutated by run) is persisted through the real SaveManager
  ASSERT_TRUE(instance->save().get());
  const auto sramAtSave = m_fakeCore->sram();
  const auto readBack = m_saveManager->readSaveData(m_hash, instance->getSaveSlotNumber());
  ASSERT_TRUE(readBack.has_value());
  EXPECT_EQ(readBack->getSaveRamData(), sramAtSave);

  // Rewind — capture a state, advance, then restore it
  const auto rewindPoint = instance->serializeState();
  for (int i = 0; i < 3; ++i) {
    instance->runFrame();
  }
  EXPECT_EQ(m_fakeCore->frameCount(), 8);
  instance->deserializeState(rewindPoint);
  EXPECT_EQ(m_fakeCore->frameCount(), 5); // restored to the rewind point

  // Reset
  instance->reset();
  EXPECT_EQ(m_fakeCore->frameCount(), 0);

  // Teardown — destroying the instance must NOT exit the process (no real DLL)
  m_emulationService->stopEmulation();
  EXPECT_EQ(m_emulationService->getCurrentEmulatorInstance(), nullptr);
}

TEST_F(EmulatorInstanceE2ETest, AppliesTypedCheatsOnLoad) {
  // A Game Genie cheat (core-applied) + a RAM cheat (Firelight poke) + a
  // disabled one that should be ignored
  cheats::Cheat gg{.contentHash = m_hash,
                   .name = "Infinite Lives",
                   .type = cheats::CheatType::GameGenie,
                   .rawCode = "SXIOPO",
                   .enabled = true};
  cheats::Cheat ram{.contentHash = m_hash,
                    .name = "Max HP",
                    .type = cheats::CheatType::GameShark,
                    .pokes = {{0x10u, 0x63u, 1, false}},
                    .enabled = true};
  cheats::Cheat off{.contentHash = m_hash,
                    .name = "Disabled",
                    .type = cheats::CheatType::GameGenie,
                    .rawCode = "AAAAAA",
                    .enabled = false};
  ASSERT_TRUE(m_cheatRepo->addCheat(gg));
  ASSERT_TRUE(m_cheatRepo->addCheat(ram));
  ASSERT_TRUE(m_cheatRepo->addCheat(off));

  const int entryId = ingestEntry();
  ASSERT_NE(entryId, -1);
  auto *instance = m_emulationService->loadEntry(entryId).get();
  ASSERT_NE(instance, nullptr);
  ASSERT_TRUE(instance->initialize(nullptr));

  // Core-applied cheats: reset + only the enabled Game Genie code handed over
  EXPECT_GE(m_fakeCore->cheatsClearedCount(), 1);
  ASSERT_EQ(m_fakeCore->cheatCalls().size(), 1u);
  EXPECT_EQ(m_fakeCore->cheatCalls()[0].code, "SXIOPO");

  // The RAM cheat is applied by Firelight each frame
  instance->runFrame();
  ASSERT_GT(m_fakeCore->systemRam().size(), 0x10u);
  EXPECT_EQ(m_fakeCore->systemRam()[0x10], 0x63);

  m_emulationService->stopEmulation();
}

// TODO
// The frame thread is the only one that touches the core: a reboot asked for elsewhere waits for it
TEST_F(EmulatorInstanceE2ETest, ResetGameIsACommandTheFrameThreadRuns) {
  const int entryId = ingestEntry();
  ASSERT_NE(entryId, -1);
  auto *instance = m_emulationService->loadEntry(entryId).get();
  ASSERT_NE(instance, nullptr);
  ASSERT_TRUE(instance->initialize(nullptr));

  for (int i = 0; i < 5; ++i) {
    instance->runFrame();
  }

  ASSERT_EQ(m_fakeCore->frameCount(), 5);

  m_emulationService->resetGame();
  EXPECT_EQ(m_fakeCore->frameCount(), 5);

  instance->drainCommands();
  EXPECT_EQ(m_fakeCore->frameCount(), 0);

  m_emulationService->stopEmulation();
}

TEST_F(EmulatorInstanceE2ETest, AControllerDeviceReachesTheCoreOnTheNextDrain) {
  const int entryId = ingestEntry();
  ASSERT_NE(entryId, -1);
  auto *instance = m_emulationService->loadEntry(entryId).get();
  ASSERT_NE(instance, nullptr);
  ASSERT_TRUE(instance->initialize(nullptr));
  const auto callsBefore = m_fakeCore->portDeviceCalls().size();

  instance->submitCommand({.type = EmulatorCommandType::SetControllerDevice, .port = 1, .coreDeviceId = 5});
  EXPECT_EQ(m_fakeCore->portDeviceCalls().size(), callsBefore);

  instance->drainCommands();

  ASSERT_EQ(m_fakeCore->portDeviceCalls().size(), callsBefore + 1);
  EXPECT_EQ(m_fakeCore->portDeviceCalls().back(), std::make_pair(1u, 5u));

  m_emulationService->stopEmulation();
}

TEST_F(EmulatorInstanceE2ETest, EmitRewindPointsGoesToTheSink) {
  const int entryId = ingestEntry();
  ASSERT_NE(entryId, -1);
  auto *instance = m_emulationService->loadEntry(entryId).get();
  ASSERT_NE(instance, nullptr);
  ASSERT_TRUE(instance->initialize(nullptr));

  std::vector<EmulatorCommandType> handedOn;
  instance->setCommandSink([&handedOn](const EmulatorCommand &command) { handedOn.push_back(command.type); });

  instance->submitCommand({.type = EmulatorCommandType::WriteRewindPoint});
  instance->submitCommand({.type = EmulatorCommandType::EmitRewindPoints});
  instance->drainCommands();

  ASSERT_EQ(handedOn.size(), 1u);
  EXPECT_EQ(handedOn[0], EmulatorCommandType::EmitRewindPoints);
  EXPECT_EQ(instance->getRewindPointPictures().size(), 1u);

  instance->setCommandSink(nullptr);
  m_emulationService->stopEmulation();
}

// TODO
// Frames on their own thread while every setter the GUI has is being called: nothing may tear
TEST_F(EmulatorInstanceE2ETest, FramesOnTheirOwnThreadWhileTheGuiChangesState) {
  const int entryId = ingestEntry();
  ASSERT_NE(entryId, -1);
  auto *instance = m_emulationService->loadEntry(entryId).get();
  ASSERT_NE(instance, nullptr);
  ASSERT_TRUE(instance->initialize(nullptr));

  std::atomic<bool> stop{false};
  std::atomic<int> framesRun{0};
  std::thread frames([&] {
    while (!stop.load()) {
      instance->drainCommands();
      instance->runFrame();
      framesRun.fetch_add(1);
    }
  });

  while (framesRun.load() == 0) {
    std::this_thread::yield();
  }

  for (int round = 0; round < 300; ++round) {
    instance->submitCommand({.type = EmulatorCommandType::WriteRewindPoint});
    instance->submitCommand(
        {.type = EmulatorCommandType::SetPlaybackMultiplier, .playbackMultiplier = round % 2 == 0 ? 2.0F : 1.0F});
    instance->setMuted(round % 2 == 0);
    instance->setAudioPlaybackRateRatio(1.0 + round * 0.001);
    instance->setPictureMode(round % 2 == 0 ? "sharp" : "smooth");
    instance->setSyncMethod(round % 3 == 0 ? "audio" : "monitor");
    instance->setAnalogPointerSpeed(0.01 * (round % 3));
    instance->setMouseControlsPointerDevices(round % 2 == 0);
    instance->setInstantReplayEnabled(round % 2 == 0);
    EXPECT_FALSE(instance->getPictureMode().empty());
    (void)instance->getSyncMethod();
    (void)instance->getRewindPointPictures();
    (void)instance->canUndoLoadSuspendPoint();

    if (round % 10 == 0) {
      instance->submitCommand({.type = EmulatorCommandType::LoadRewindPoint, .rewindPointIndex = 1});
    }

    if (round % 50 == 0) {
      m_emulationService->resetGame();
    }
  }

  stop.store(true);
  frames.join();

  EXPECT_GT(framesRun.load(), 0);
  instance->drainCommands();
  EXPECT_LE(instance->getRewindPointPictures().size(), EmulatorInstance::MAX_REWIND_POINTS);
  EXPECT_GE(instance->getRewindPointPictures().size(), 1u);

  m_emulationService->stopEmulation();
}

TEST_F(EmulatorInstanceE2ETest, SwapDiscIsACommandTheFrameThreadRuns) {
  auto *instance = loadGame();
  ASSERT_NE(instance, nullptr);
  ASSERT_TRUE(instance->initialize(nullptr));
  m_fakeCore->setDiscCount(2);

  EXPECT_TRUE(instance->swapDisc(1));
  EXPECT_EQ(m_fakeCore->getCurrentDiskIndex(), 0u);

  instance->drainCommands();
  EXPECT_EQ(m_fakeCore->getCurrentDiskIndex(), 1u);
  EXPECT_FALSE(instance->swapDisc(5));

  m_emulationService->stopEmulation();
}

TEST_F(EmulatorInstanceE2ETest, CheatsApplyOnTheNextDrain) {
  cheats::Cheat gg{.contentHash = m_hash,
                   .name = "Infinite Lives",
                   .type = cheats::CheatType::GameGenie,
                   .rawCode = "SXIOPO",
                   .enabled = true};
  ASSERT_TRUE(m_cheatRepo->addCheat(gg));
  const auto stored = m_cheatRepo->getCheats(m_hash);
  ASSERT_EQ(stored.size(), 1u);

  auto *instance = loadGame();
  ASSERT_NE(instance, nullptr);
  ASSERT_TRUE(instance->initialize(nullptr));
  const auto clearedAtLoad = m_fakeCore->cheatsClearedCount();

  instance->setCheatEnabled(stored[0].id, false);
  EXPECT_EQ(m_fakeCore->cheatsClearedCount(), clearedAtLoad);

  instance->drainCommands();
  EXPECT_EQ(m_fakeCore->cheatsClearedCount(), clearedAtLoad + 1);

  m_emulationService->stopEmulation();
}

} // namespace firelight::emulation
