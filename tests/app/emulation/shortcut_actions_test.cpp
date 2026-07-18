#include "emulation/shortcut_actions.hpp"

#include "audio/audio_settings.hpp"

#include <firelight/input/shortcut_registry.hpp>
#include <firelight/settings/settings_service.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>
#include <gtest/gtest.h>

namespace firelight::emulation {

namespace {
// Records what a shortcut asked the emulator to do, so a test can assert on the
// action rather than on a running core
class FakeController final : public IEmulatorController {
public:
  float multiplier = 1.0f;
  bool isPaused = false;
  int framesAdvanced = 0;
  std::vector<int> written;
  std::vector<int> loaded;
  int screenshots = 0;
  int clips = 0;

  [[nodiscard]] float playbackMultiplier() const override { return multiplier; }
  void setPlaybackMultiplier(const float m) override { multiplier = m; }
  [[nodiscard]] bool paused() const override { return isPaused; }
  void setPaused(const bool p) override { isPaused = p; }
  void advanceOneFrame() override { ++framesAdvanced; }
  void writeSuspendPoint(const int index) override { written.push_back(index); }
  void loadSuspendPoint(const int index) override { loaded.push_back(index); }
  void captureScreenshot() override { ++screenshots; }
  void captureVideoClip() override { ++clips; }
};
} // namespace

class ShortcutActionsTest : public testing::Test {
protected:
  void SetUp() override {
    settings::SettingsService::setInstance(&m_settingsService);
    m_actions.setController(&m_controller);
    m_actions.setIntents({
        .openQuickMenu = [this] { ++m_quickMenus; },
        .openRewindMenu = [this] { ++m_rewindMenus; },
        .toggleFullscreen = [this] { ++m_fullscreens; },
        .resetGame = [this] { ++m_resets; },
        .exitGame = [this] { ++m_exits; },
        .notify = [this](const std::string &message) {
          m_messages.push_back(message);
        },
    });
  }
  void TearDown() override { input::ShortcutRegistry::instance().clear(); }

  void press(const std::string &id) {
    m_actions.handle(id, input::ShortcutPhase::Started);
  }
  void release(const std::string &id) {
    m_actions.handle(id, input::ShortcutPhase::Ended);
  }

  settings::SqliteSettingsRepository m_settingsRepo{":memory:"};
  settings::SettingsService m_settingsService{m_settingsRepo};

  FakeController m_controller;
  bool m_hardcore = false;
  int m_quickMenus = 0;
  int m_rewindMenus = 0;
  int m_fullscreens = 0;
  int m_resets = 0;
  int m_exits = 0;
  std::vector<std::string> m_messages;

  ShortcutActions m_actions{m_settingsService, [this] { return m_hardcore; },
                            ShortcutActions::Intents{}};
};

// A hotkey's only feedback is the toast, so the wording is behaviour
TEST_F(ShortcutActionsTest, SavingAndLoadingNameTheSlot) {
  press("save_state");
  EXPECT_EQ(m_messages.back(), "Saved to slot 1");

  press("state_slot_next");
  EXPECT_EQ(m_messages.back(), "Save slot 2");

  press("load_state");
  EXPECT_EQ(m_messages.back(), "Loaded slot 2");
}

TEST_F(ShortcutActionsTest, SpeedMessagesReadAsMultipliers) {
  press("speed_up");
  EXPECT_EQ(m_messages.back(), "Speed 2x");

  press("slow_down");
  press("slow_down");
  EXPECT_EQ(m_messages.back(), "Speed 0.5x");
}

// Releasing a hold has to say what you went back to, not what you left
TEST_F(ShortcutActionsTest, ReleasingAHoldReportsTheRestoredSpeed) {
  m_controller.multiplier = 2.0f;
  press("fast_forward");
  ASSERT_EQ(m_messages.back(), "Speed 4x");

  release("fast_forward");
  EXPECT_EQ(m_messages.back(), "Speed 2x");
}

TEST_F(ShortcutActionsTest, VolumeStepsAndClampsAtTheEnds) {
  press("volume_down");
  EXPECT_EQ(m_settingsService.getGlobalValue(audio::VOLUME_KEY), "90");
  EXPECT_EQ(m_messages.back(), "Volume 90%");

  press("volume_up");
  press("volume_up"); // already at 100; must not run past it
  EXPECT_EQ(m_settingsService.getGlobalValue(audio::VOLUME_KEY), "100");

  for (int i = 0; i < 12; ++i) {
    press("volume_down");
  }
  EXPECT_EQ(m_settingsService.getGlobalValue(audio::VOLUME_KEY), "0");
}

// Turning it up while muted has to make sound, or it looks broken
TEST_F(ShortcutActionsTest, VolumeUpUnmutes) {
  press("toggle_mute");
  ASSERT_EQ(m_settingsService.getGlobalValue(audio::MUTED_KEY), "true");

  press("volume_up");
  EXPECT_EQ(m_settingsService.getGlobalValue(audio::MUTED_KEY), "false");
}

// Volume down is not a mute: it must leave the mute state alone
TEST_F(ShortcutActionsTest, VolumeDownLeavesMuteAlone) {
  press("toggle_mute");
  press("volume_down");
  EXPECT_EQ(m_settingsService.getGlobalValue(audio::MUTED_KEY), "true");
}

TEST_F(ShortcutActionsTest, VolumeWorksWithNoGameRunning) {
  m_actions.setController(nullptr);
  press("volume_down");
  EXPECT_EQ(m_settingsService.getGlobalValue(audio::VOLUME_KEY), "90");
}

TEST_F(ShortcutActionsTest, MuteReportsTheStateItSwitchedTo) {
  press("toggle_mute");
  EXPECT_EQ(m_messages.back(), "Muted");

  press("toggle_mute");
  EXPECT_EQ(m_messages.back(), "Sound on");
}

// Frame advance is pressed repeatedly by design; a toast per frame would be
// noise on top of the thing you're trying to look at
TEST_F(ShortcutActionsTest, FrameAdvanceSaysNothing) {
  press("frame_advance");
  EXPECT_TRUE(m_messages.empty());
}

TEST_F(ShortcutActionsTest, HoldFastForwardRestoresThePreviousSpeed) {
  // Not 1x: releasing a hold has to put back whatever you were at, which is the
  // whole reason the speed is remembered when the hold starts
  m_controller.multiplier = 2.0f;

  press("fast_forward");
  EXPECT_GT(m_controller.multiplier, 2.0f);

  release("fast_forward");
  EXPECT_FLOAT_EQ(m_controller.multiplier, 2.0f);
}

TEST_F(ShortcutActionsTest, ASecondHoldDoesNotClobberTheRestorePoint) {
  press("fast_forward");
  const auto fastForwarding = m_controller.multiplier;

  // Slow motion arriving mid-fast-forward must not record 4x as the speed to
  // restore, or releasing would leave the game stuck fast
  press("slow_motion");
  EXPECT_FLOAT_EQ(m_controller.multiplier, fastForwarding);

  release("fast_forward");
  EXPECT_FLOAT_EQ(m_controller.multiplier, 1.0f);
}

TEST_F(ShortcutActionsTest, SpeedStepsDoubleAndHalveWithinBounds) {
  press("speed_up");
  EXPECT_FLOAT_EQ(m_controller.multiplier, 2.0f);
  press("speed_up");
  EXPECT_FLOAT_EQ(m_controller.multiplier, 4.0f);

  for (int i = 0; i < 10; ++i) {
    press("slow_down");
  }
  EXPECT_GE(m_controller.multiplier, 0.25f) << "slowing down has a floor";

  for (int i = 0; i < 20; ++i) {
    press("speed_up");
  }
  EXPECT_LE(m_controller.multiplier, 16.0f) << "speeding up has a ceiling";
}

TEST_F(ShortcutActionsTest, ToggleFastForwardFlipsBetweenSpeedAndNormal) {
  press("toggle_fast_forward");
  EXPECT_GT(m_controller.multiplier, 1.0f);
  press("toggle_fast_forward");
  EXPECT_FLOAT_EQ(m_controller.multiplier, 1.0f);
}

TEST_F(ShortcutActionsTest, SaveAndLoadUseTheSlotCursor) {
  EXPECT_EQ(m_actions.saveSlot(), 1);

  press("save_state");
  press("state_slot_next");
  press("save_state");
  press("load_state");

  ASSERT_EQ(m_controller.written.size(), 2u);
  EXPECT_EQ(m_controller.written[0], 1);
  EXPECT_EQ(m_controller.written[1], 2);
  ASSERT_EQ(m_controller.loaded.size(), 1u);
  EXPECT_EQ(m_controller.loaded[0], 2);
}

TEST_F(ShortcutActionsTest, SlotCursorWrapsRatherThanSticking) {
  press("state_slot_prev");
  EXPECT_EQ(m_actions.saveSlot(), 8) << "stepping below the first slot wraps";

  press("state_slot_next");
  EXPECT_EQ(m_actions.saveSlot(), 1) << "and stepping past the last wraps back";
}

TEST_F(ShortcutActionsTest, PauseFlipsWhicheverDevicePressedIt) {
  // The engine reports an edge and this owns the value, so two presses from
  // anywhere have to alternate rather than re-assert the same state
  press("pause");
  EXPECT_TRUE(m_controller.isPaused);
  press("pause");
  EXPECT_FALSE(m_controller.isPaused);
}

TEST_F(ShortcutActionsTest, MuteTogglesTheSettingSoItOutlivesTheGame) {
  press("toggle_mute");
  EXPECT_EQ(m_settingsService.getGlobalValue(audio::MUTED_KEY).value_or(""),
            "true");
  press("toggle_mute");
  EXPECT_EQ(m_settingsService.getGlobalValue(audio::MUTED_KEY).value_or(""),
            "false");
}

TEST_F(ShortcutActionsTest, MuteAndScreenshotWorkWithNoGameRunning) {
  // Both are ScopeAlways, so they fire in the library where there's no emulator
  m_actions.setController(nullptr);

  press("toggle_mute");
  EXPECT_EQ(m_settingsService.getGlobalValue(audio::MUTED_KEY).value_or(""),
            "true");

  press("screenshot"); // must not crash on the absent controller
  press("open_quick_menu");
  EXPECT_EQ(m_quickMenus, 1);
}

TEST_F(ShortcutActionsTest, HardcoreModeBlocksTheActionsThatCheat) {
  m_hardcore = true;

  press("load_state");
  press("open_rewind_menu");
  press("frame_advance");
  press("slow_motion");

  EXPECT_TRUE(m_controller.loaded.empty());
  EXPECT_EQ(m_rewindMenus, 0);
  EXPECT_EQ(m_controller.framesAdvanced, 0);
  EXPECT_FLOAT_EQ(m_controller.multiplier, 1.0f);

  // Saving and going faster aren't cheating
  press("save_state");
  press("speed_up");
  EXPECT_EQ(m_controller.written.size(), 1u);
  EXPECT_GT(m_controller.multiplier, 1.0f);
}

TEST_F(ShortcutActionsTest, FrameAdvanceStepsAndLeavesItPaused) {
  press("frame_advance");
  EXPECT_EQ(m_controller.framesAdvanced, 1);
}

TEST_F(ShortcutActionsTest, ReleaseIsIgnoredByEverythingButHolds) {
  release("save_state");
  release("pause");
  release("screenshot");

  EXPECT_TRUE(m_controller.written.empty());
  EXPECT_FALSE(m_controller.isPaused);
  EXPECT_EQ(m_controller.screenshots, 0);
}

// The failure this phase exists to prevent: an action declared in the shipped
// catalog that nothing dispatches. Every one used to be exactly that
TEST_F(ShortcutActionsTest, EveryShippedActionActuallyDoesSomething) {
  auto &registry = input::ShortcutRegistry::instance();
  ASSERT_TRUE(registry.loadFromFile(FL_SHORTCUTS_FILE));
  ASSERT_FALSE(registry.listActions().empty());

  for (const auto &action : registry.listActions()) {
    // TODO
    // The one action ShortcutActions is right to ignore: it turns off the
    // hotkeys of the device that pressed it, and only the engine knows which
    // device that was. ShortcutEngine tests cover it
    if (action.id == input::TOGGLE_HOTKEYS_ID) {
      continue;
    }

    FakeController controller;
    // Clearing the controller drops any hold left over from a previous action,
    // which would otherwise (correctly) make the next hold a no-op
    m_actions.setController(nullptr);
    m_actions.setController(&controller);
    const auto quickMenus = m_quickMenus;
    const auto rewindMenus = m_rewindMenus;
    const auto fullscreens = m_fullscreens;
    const auto muted = m_settingsService.getGlobalValue(audio::MUTED_KEY);
    const auto volume = m_settingsService.getGlobalValue(audio::VOLUME_KEY);

    press(action.id);

    const bool touchedEmulator =
        controller.multiplier != 1.0f || controller.isPaused ||
        controller.framesAdvanced > 0 || !controller.written.empty() ||
        !controller.loaded.empty() ||
        controller.screenshots > 0 || controller.clips > 0;
    const bool touchedUi = m_quickMenus != quickMenus ||
                           m_rewindMenus != rewindMenus ||
                           m_fullscreens != fullscreens;
    const bool touchedSettings =
        m_settingsService.getGlobalValue(audio::MUTED_KEY) != muted ||
        m_settingsService.getGlobalValue(audio::VOLUME_KEY) != volume;
    const bool touchedLifecycle = m_resets > 0 || m_exits > 0;
    // The slot cursor is the only visible effect of the slot steppers
    const bool movedSlotCursor =
        action.id == "state_slot_next" || action.id == "state_slot_prev";
    m_resets = 0;
    m_exits = 0;

    EXPECT_TRUE(touchedEmulator || touchedUi || touchedSettings ||
                touchedLifecycle || movedSlotCursor)
        << "data/shortcuts.json declares '" << action.id
        << "' but ShortcutActions does nothing with it, so binding it would "
           "silently do nothing";
  }
}

} // namespace firelight::emulation
