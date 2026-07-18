#include "shortcut_actions.hpp"

#include "audio/audio_settings.hpp"

#include <firelight/settings/settings_service.hpp>

#include <algorithm>

namespace firelight::emulation {

namespace {
constexpr float FAST_FORWARD_SPEED = 4.0f;
constexpr float SLOW_MOTION_SPEED = 0.5f;
constexpr float MIN_SPEED = 0.25f;
constexpr float MAX_SPEED = 16.0f;
constexpr int MAX_SAVE_SLOT = 8;
constexpr int VOLUME_STEP = 10;
} // namespace

ShortcutActions::ShortcutActions(settings::SettingsService &settingsService,
                                 std::function<bool()> hardcoreModeActive,
                                 Intents intents)
    : m_settingsService(settingsService),
      m_hardcoreModeActive(std::move(hardcoreModeActive)),
      m_intents(std::move(intents)) {}

void ShortcutActions::setController(IEmulatorController *controller) {
  m_controller = controller;
  if (!controller) {
    m_activeSpeedHold.clear();
    m_speedBeforeHold = 1.0f;
  }
}

bool ShortcutActions::blockedByHardcore(const input::ShortcutId &id) const {
  if (!m_hardcoreModeActive || !m_hardcoreModeActive()) {
    return false;
  }
  return id == "rewind" || id == "open_rewind_menu" || id == "slow_motion" ||
         id == "load_state" || id == "frame_advance";
}

void ShortcutActions::beginHold(const input::ShortcutId &id,
                                const float multiplier) {
  if (!m_controller || !m_activeSpeedHold.empty()) {
    return;
  }
  m_activeSpeedHold = id;
  m_speedBeforeHold = m_controller->playbackMultiplier();
  m_controller->setPlaybackMultiplier(multiplier);
  notify("Speed " + speedLabel(multiplier));
}

void ShortcutActions::endHold(const input::ShortcutId &id) {
  if (!m_controller || m_activeSpeedHold != id) {
    return;
  }
  m_activeSpeedHold.clear();
  m_controller->setPlaybackMultiplier(m_speedBeforeHold);
  notify("Speed " + speedLabel(m_speedBeforeHold));
}

void ShortcutActions::stepSpeed(const bool faster) {
  if (!m_controller) {
    return;
  }
  // Doubling/halving rather than stepping by one: 1x -> 2x -> 4x going up, and
  // 1x -> 0.5x -> 0.25x going down, so one action covers both ends
  const auto current = m_controller->playbackMultiplier();
  const auto next = faster ? current * 2.0f : current / 2.0f;
  const auto clamped = std::clamp(next, MIN_SPEED, MAX_SPEED);
  m_controller->setPlaybackMultiplier(clamped);
  notify("Speed " + speedLabel(clamped));
}

void ShortcutActions::setSaveSlot(const int slot) {
  // Wraps, so holding the key cycles rather than getting stuck at an end
  m_saveSlot = slot < 1 ? MAX_SAVE_SLOT : (slot > MAX_SAVE_SLOT ? 1 : slot);
  notify("Save slot " + std::to_string(m_saveSlot));
}

void ShortcutActions::stepVolume(const int delta) {
  auto current = 100;
  try {
    current = std::stoi(
        m_settingsService.getGlobalValue(audio::VOLUME_KEY).value_or("100"));
  } catch (const std::exception &) {
    current = 100;
  }
  const auto next = std::clamp(current + delta, 0, 100);
  m_settingsService.setGlobalValue(audio::VOLUME_KEY, std::to_string(next));

  // Turning it up is also how you undo a mute — leaving it silent while the
  // number climbs would look broken
  if (delta > 0 && m_settingsService.getGlobalValue(audio::MUTED_KEY)
                           .value_or("false") == "true") {
    m_settingsService.setGlobalValue(audio::MUTED_KEY, "false");
  }
  notify("Volume " + std::to_string(next) + "%");
}

void ShortcutActions::notify(const std::string &message) const {
  if (m_intents.notify) {
    m_intents.notify(message);
  }
}

std::string ShortcutActions::speedLabel(const float multiplier) {
  // 2 -> "2x", 0.5 -> "0.5x": trailing zeroes read like precision that isn't
  // there
  auto text = std::to_string(multiplier);
  text.erase(text.find_last_not_of('0') + 1);
  if (text.ends_with('.')) {
    text.pop_back();
  }
  return text + "x";
}

void ShortcutActions::handle(const input::ShortcutId &id,
                             const input::ShortcutPhase phase) {
  if (blockedByHardcore(id)) {
    return;
  }

  const bool started = phase == input::ShortcutPhase::Started;

  // Hold actions are the only ones that do anything on release
  if (id == "fast_forward") {
    started ? beginHold(id, FAST_FORWARD_SPEED) : endHold(id);
    return;
  }
  if (id == "slow_motion") {
    started ? beginHold(id, SLOW_MOTION_SPEED) : endHold(id);
    return;
  }
  if (!started) {
    return;
  }

  if (id == "toggle_fast_forward") {
    if (m_controller) {
      const auto atSpeed = m_controller->playbackMultiplier() != 1.0f;
      const auto next = atSpeed ? 1.0f : FAST_FORWARD_SPEED;
      m_controller->setPlaybackMultiplier(next);
      notify("Speed " + speedLabel(next));
    }
  } else if (id == "speed_up") {
    stepSpeed(true);
  } else if (id == "slow_down") {
    stepSpeed(false);
  } else if (id == "save_state") {
    if (m_controller) {
      m_controller->writeSuspendPoint(m_saveSlot);
      notify("Saved to slot " + std::to_string(m_saveSlot));
    }
  } else if (id == "load_state") {
    if (m_controller) {
      m_controller->loadSuspendPoint(m_saveSlot);
      notify("Loaded slot " + std::to_string(m_saveSlot));
    }
  } else if (id == "state_slot_next") {
    setSaveSlot(m_saveSlot + 1);
  } else if (id == "state_slot_prev") {
    setSaveSlot(m_saveSlot - 1);
  } else if (id == "pause") {
    if (m_controller) {
      const auto paused = !m_controller->paused();
      m_controller->setPaused(paused);
      notify(paused ? "Paused" : "Resumed");
    }
  } else if (id == "frame_advance") {
    if (m_controller) {
      m_controller->advanceOneFrame();
    }
  } else if (id == "reset") {
    if (m_intents.resetGame) {
      m_intents.resetGame();
    }
  } else if (id == "exit_game") {
    if (m_intents.exitGame) {
      m_intents.exitGame();
    }
  } else if (id == "screenshot") {
    if (m_controller) {
      m_controller->captureScreenshot();
      notify("Screenshot saved");
    }
  } else if (id == "capture_clip") {
    if (m_controller) {
      m_controller->captureVideoClip();
      notify("Clip saved");
    }
  } else if (id == "toggle_mute") {
    // Mute is a setting, not emulator state, so it works with no game running
    // and outlives the one you're playing
    const auto muted =
        m_settingsService.getGlobalValue(audio::MUTED_KEY).value_or("false") ==
        "true";
    m_settingsService.setGlobalValue(audio::MUTED_KEY,
                                     muted ? "false" : "true");
    notify(muted ? "Sound on" : "Muted");
  } else if (id == "volume_up") {
    stepVolume(VOLUME_STEP);
  } else if (id == "volume_down") {
    stepVolume(-VOLUME_STEP);
  } else if (id == "open_rewind_menu") {
    // The intent, not the command: opening this menu means asking for the
    // rewind points and showing them when they arrive, and the UI already owns
    // both halves along with the rewind-enabled check
    if (m_intents.openRewindMenu) {
      m_intents.openRewindMenu();
    }
  } else if (id == "open_quick_menu") {
    if (m_intents.openQuickMenu) {
      m_intents.openQuickMenu();
    }
  } else if (id == "toggle_fullscreen") {
    if (m_intents.toggleFullscreen) {
      m_intents.toggleFullscreen();
    }
  }
}

} // namespace firelight::emulation
