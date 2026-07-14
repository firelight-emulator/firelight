#pragma once
#include <firelight/cheats/cheat.hpp>
#include <firelight/cheats/cheat_engine.hpp>
#include "emulation_context.hpp"
#include "libretro/core_registry.hpp"

#include <firelight/event_dispatcher.hpp>
#include <firelight/libretro/audio_input_provider.hpp>
#include <firelight/libretro/audio_output.hpp>
#include <firelight/libretro/icore.hpp>
#include <future>
#include <memory>
#include <firelight/saves/suspend_point.hpp>
#include <string>
#include <vector>

namespace firelight::emulation {

class CoreSettingsApplier;

// Threading: owned by EmulatorItemRenderer and confined to the render thread —
// initialize()/runFrame()/state calls all run there. getAudioBufferLevel() is
// the exception: it's read from the pacing thread (backed by an atomic).
class EmulatorInstance {
public:
  EmulatorInstance(std::unique_ptr<::libretro::ICore>, std::string contentPath,
                   std::string contentHash, int platformId, int saveSlotNumber,
                   std::vector<uint8_t> gameData, std::vector<uint8_t> saveData,
                   EmulationContext context);
  ~EmulatorInstance();

  // Must be called from the render thread (with active graphics context)
  bool initialize(libretro::IVideoDataReceiver *videoDataReceiver);
  bool isInitialized();

  std::string getContentHash() const;
  int getPlatformId() const;
  int getSaveSlotNumber() const;

  // Must be called from the render thread (with active graphics context)
  void runFrame();
  void reset();
  std::future<bool> save();

  // --- TAS harness support (roadmap Phase 1) ---
  // TAS mode makes runFrame() a pure emulation step for bit-exact replay: it
  // suppresses the periodic autosave, whose trigger is wall-clock time and whose
  // work is disk IO — both unrelated to emulation and a source of nondeterminism.
  // The TAS session sets this before it drives frames through a movie provider.
  void setTasMode(bool enabled) { m_tasMode = enabled; }
  [[nodiscard]] bool isTasMode() const { return m_tasMode; }
  // Autosave cadence in wall-clock seconds (default 10). Exposed so the cadence is
  // tunable and the TAS determinism guard above is directly testable.
  void setAutosaveIntervalSeconds(int seconds) { m_saveIntervalSeconds = seconds; }
  [[nodiscard]] int getAutosaveIntervalSeconds() const {
    return m_saveIntervalSeconds;
  }
  // Install the authoritative per-frame input source on the loaded core — the TAS
  // movie provider. Forwards to ICore::setRetropadProvider, replacing the live
  // input service for the duration of playback/record. Pass the live input service
  // (or nullptr) to restore normal control.
  void setRetropadProvider(firelight::libretro::IRetropadProvider *provider);

  void setMuted(bool muted);
  bool isMuted() const;

  // The mute state the AudioManager is born with in initialize(). Lets a CLI
  // `--muted` launch start muted from the first frame, before the render thread
  // creates the AudioManager (a QML muted binding fires too early to catch it).
  // Must be set before initialize().
  void setStartMuted(bool muted) { m_startMuted = muted; }

  // Suspends/resumes audio output when the game is paused/resumed, so buffered
  // audio doesn't keep playing after a pause.
  void setPaused(bool paused);

  void setRewindEnabled(bool enabled);
  bool isRewindEnabled() const;

  void setPictureMode(const std::string &pictureMode);
  std::string getPictureMode() const;

  void setAspectRatioMode(const std::string &aspectRatioMode);
  std::string getAspectRatioMode() const;

  void setIntegerScale(int integerScale);
  int getIntegerScale() const;

  void setSyncMethod(const std::string &syncMethod);
  std::string getSyncMethod() const;

  void setTargetFramerate(int targetFramerate);
  int getTargetFramerate() const;

  // Audio buffer fill ratio (0.0-1.0), or -1.0 when there is no audio device
  // (used by the "audio" sync method to pace frames). Safe to call from the
  // emulation pacing thread.
  float getAudioBufferLevel() const;

  // Biases audio playback rate (1.0 = native); set by sync-to-monitor to
  // resample audio to the display refresh rate.
  void setAudioPlaybackRateRatio(double ratio);

  // Enables/disables audio Dynamic Rate Control (the "dynamic-rate-control"
  // advanced setting). Stored so it can be applied when the AudioManager is
  // (re)created, and forwarded live when it already exists.
  void setDynamicRateControlEnabled(bool enabled);
  bool getDynamicRateControlEnabled() const;

  // Whether the instant-replay recorder should keep a rolling window while this
  // game runs (the "instant-replay-enabled" advanced setting). Stored here as the
  // resolved value; the renderer reads it to gate its ClipRecorder.
  void setInstantReplayEnabled(bool enabled);
  [[nodiscard]] bool getInstantReplayEnabled() const;

  // Forward the two core-side input settings (glide speed for stick-driven
  // pointer devices; whether the physical mouse drives mouse/light-gun devices).
  void setAnalogPointerSpeed(double stepPerFrame);
  void setMouseControlsPointerDevices(bool enabled);

  std::vector<uint8_t> serializeState();
  bool deserializeState(const std::vector<uint8_t> &state);

  // Multi-disc control. getDiscCount() is 0 for single-disc/cartridge content.
  // swapDisc publishes a DiscChangedEvent on success.
  [[nodiscard]] unsigned getDiscCount() const;
  [[nodiscard]] unsigned getCurrentDiscIndex() const;
  bool swapDisc(unsigned index);

  // Raw per-port device options the running core advertises via
  // SET_CONTROLLER_INFO (empty when there's no choice).
  [[nodiscard]] std::vector<std::vector<::libretro::ICore::ControllerDeviceOption>>
  getControllerDevices() const;
  // Curated, console-native controller variants selectable for `port` on the
  // loaded core (default first), cross-referenced with the core's advertisement.
  [[nodiscard]] std::vector<CoreDeviceVariant>
  getAvailableControllerVariants(unsigned port) const;
  // Number of controller ports the running core exposes (SET_CONTROLLER_INFO).
  [[nodiscard]] unsigned getControllerPortCount() const;
  // The coreDeviceId currently selected for `port` (the per-game override if
  // still valid, else the default) — lets the UI highlight the active choice.
  [[nodiscard]] unsigned getSelectedControllerVariant(unsigned port) const;
  // Selects a variant (by its coreDeviceId) for `port`: applies it to the core
  // live, applies any companion core options, and persists it for this game.
  void setPortControllerVariant(unsigned port, unsigned coreDeviceId);

  // --- cheats (Game Genie / Action Replay) ---
  // This game's saved cheats (from the cheat repository).
  [[nodiscard]] std::vector<cheats::Cheat> getCheats() const;
  // Toggles a cheat, persists it, and re-applies the active set.
  void setCheatEnabled(int cheatId, bool enabled);
  // Adds a fully-formed cheat for this game (the UI / a decoder builds it,
  // resolving pokes for RAM cheats). Persisted; re-applied.
  void addCheat(cheats::Cheat cheat);
  // Persists an edit and re-applies.
  void updateCheat(const cheats::Cheat &cheat);
  // Removes a cheat and re-applies.
  void removeCheat(int cheatId);

private:
  // Resolves the controller variant selected for a port: the per-game override
  // (by coreDeviceId) if still valid for the loaded core, else the default.
  [[nodiscard]] CoreDeviceVariant resolveSelectedVariant(unsigned port) const;
  // Persists a variant's companion core options as game values so the core
  // picks them up (e.g. FCEUmm Zapper -> fceumm_zapper_mode=clightgun).
  void applyCompanionOptions(const CoreDeviceVariant &variant);

  // Rebuilds the active cheat set from the repository: RAM cheats go to the
  // per-frame engine, Game Genie codes to the core, and any cheat that affects
  // gameplay is skipped while RA hardcore mode is active. No-op without a repo.
  void applyCheats();

  cheats::CheatEngine m_cheatEngine;

  bool m_initialized = false;

  EmulationContext m_context;
  std::unique_ptr<::libretro::ICore> m_core;
  std::shared_ptr<IAudioOutput> m_audioOutput;
  std::unique_ptr<libretro::IAudioInputProvider> m_audioInput;
  std::vector<uint8_t> m_gameData;
  std::vector<uint8_t> m_saveData;

  std::vector<SuspendPoint> m_rewindSuspendPoints;
  std::chrono::time_point<std::chrono::steady_clock> m_lastSaveTime;
  int m_saveIntervalSeconds = 10;
  // When true, runFrame() skips the wall-clock autosave (see setTasMode).
  bool m_tasMode = false;

  std::string m_contentPath;
  std::string m_contentHash;
  int m_platformId;
  int m_saveSlotNumber;

  // Applied by m_settingsApplier (in the constructor); the declared defaults live
  // in the settings catalog, not here. These initial values are just placeholders.
  bool m_isRewindEnabled = false;
  // Initial mute state applied when the AudioManager is created in initialize().
  bool m_startMuted = false;
  std::string m_pictureMode;
  std::string m_aspectRatioMode;
  int m_integerScale = 0;
  std::string m_syncMethod;
  int m_targetFramerate = 0;
  bool m_dynamicRateControl = true;
  // Read from the render thread (renderer's clip feed); written from the GUI
  // thread when settings change. A bool toggle, so a benign 1-frame-stale read.
  bool m_instantReplayEnabled = false;

  // Observes setting changes and applies this game's resolved common settings.
  std::unique_ptr<CoreSettingsApplier> m_settingsApplier;
};

} // namespace firelight::emulation