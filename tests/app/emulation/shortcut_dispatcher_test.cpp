#include "emulation/shortcut_dispatcher.hpp"

#include <firelight/event_dispatcher.hpp>
#include <firelight/settings/settings_service.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>

#include <QCoreApplication>
#include <QSignalSpy>
#include <gtest/gtest.h>

namespace firelight::emulation {

namespace {
class RecordingController final : public IEmulatorController {
public:
  std::vector<int> written;

  [[nodiscard]] float playbackMultiplier() const override { return 1.0f; }
  void setPlaybackMultiplier(float) override {}
  [[nodiscard]] bool paused() const override { return false; }
  void setPaused(bool) override {}
  void advanceOneFrame() override {}
  void writeSuspendPoint(const int index) override { written.push_back(index); }
  void loadSuspendPoint(int) override {}
  void captureScreenshot() override {}
  void captureVideoClip() override {}
};
} // namespace

// Who pressed it. The dispatcher is the only thing that knows, so the co-op
// policy lives there rather than in the actions.
class ShortcutDispatcherTest : public testing::Test {
protected:
  settings::SqliteSettingsRepository m_repo{":memory:"};
  settings::SettingsService m_settingsService{m_repo};
  RecordingController m_controller;
  bool m_onlyPlayerOne = false;

  ShortcutActions m_actions{m_settingsService, [] { return false; },
                            ShortcutActions::Intents{}};

  std::unique_ptr<ShortcutDispatcher> m_dispatcher;

  void SetUp() override {
    settings::SettingsService::setInstance(&m_settingsService);
    m_actions.setController(&m_controller);
    m_dispatcher = std::make_unique<ShortcutDispatcher>(
        m_actions, [this](const int playerIndex) {
          return !m_onlyPlayerOne || playerIndex <= 0;
        });
  }

  void TearDown() override {
    m_dispatcher.reset();
    settings::SettingsService::setInstance(nullptr);
  }

  // The dispatcher hops to the GUI thread, so the queued call needs the event
  // loop to turn over before anything has happened.
  void fire(const int playerIndex, const std::string &id) {
    EventDispatcher::instance().publish(input::ShortcutEvent{
        .playerIndex = playerIndex, .id = id,
        .phase = input::ShortcutPhase::Started});
    QCoreApplication::processEvents();
  }
};

TEST_F(ShortcutDispatcherTest, EveryPlayerFiresShortcutsByDefault) {
  fire(0, "save_state");
  fire(1, "save_state");

  EXPECT_EQ(m_controller.written.size(), 2u);
}

TEST_F(ShortcutDispatcherTest, PlayerOneOnlyBlocksTheOtherPlayers) {
  m_onlyPlayerOne = true;

  fire(1, "save_state");
  EXPECT_TRUE(m_controller.written.empty());

  fire(0, "save_state");
  EXPECT_EQ(m_controller.written.size(), 1u);
}

// -1 means nothing device-specific fired it. That's the app itself, not a
// second player, so the policy must not swallow it.
TEST_F(ShortcutDispatcherTest, PlayerOneOnlyStillAllowsDevicelessEvents) {
  m_onlyPlayerOne = true;

  fire(-1, "save_state");
  EXPECT_EQ(m_controller.written.size(), 1u);
}

// The quick menu's route in: same object, same slot, same hardcore gate.
TEST_F(ShortcutDispatcherTest, TriggerRunsTheActionImmediately) {
  m_dispatcher->trigger("save_state");

  // No hop and no event loop: a click is already on the GUI thread.
  EXPECT_EQ(m_controller.written.size(), 1u);
}

// A menu button is whoever is holding the mouse, so the co-op policy — which is
// about which *pad* fired it — must not swallow it.
TEST_F(ShortcutDispatcherTest, TriggerIgnoresThePlayerPolicy) {
  m_onlyPlayerOne = true;

  m_dispatcher->trigger("save_state");

  EXPECT_EQ(m_controller.written.size(), 1u);
}

TEST_F(ShortcutDispatcherTest, HotkeysToggledBecomesAMessageNamingThePlayer) {
  QSignalSpy spy(m_dispatcher.get(), &ShortcutDispatcher::notified);

  EventDispatcher::instance().publish(
      input::HotkeysToggledEvent{.playerIndex = 1, .enabled = false});
  QCoreApplication::processEvents();

  ASSERT_EQ(spy.count(), 1);
  EXPECT_EQ(spy.at(0).at(0).toString(), QString("Hotkeys off — Player 2"));
}

TEST_F(ShortcutDispatcherTest, TurningHotkeysBackOnSaysSo) {
  QSignalSpy spy(m_dispatcher.get(), &ShortcutDispatcher::notified);

  EventDispatcher::instance().publish(
      input::HotkeysToggledEvent{.playerIndex = 0, .enabled = true});
  QCoreApplication::processEvents();

  ASSERT_EQ(spy.count(), 1);
  EXPECT_EQ(spy.at(0).at(0).toString(), QString("Hotkeys on — Player 1"));
}

} // namespace firelight::emulation
