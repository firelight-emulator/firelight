// TODO: NEEDS REVIEW
#pragma once
#include "emulation_context.hpp"
#include "emulator_command.hpp"
#include "frame_slot.hpp"
#include "libretro/core_registry.hpp"

#include <firelight/cheats/cheat.hpp>
#include <firelight/cheats/cheat_engine.hpp>
#include <firelight/event_dispatcher.hpp>
#include <firelight/image.hpp>
#include <firelight/libretro/audio_input_provider.hpp>
#include <firelight/libretro/audio_output.hpp>
#include <firelight/libretro/icore.hpp>
#include <firelight/monitoring/monitor.hpp>
#include <firelight/saves/suspend_point.hpp>

#include <atomic>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace firelight::emulation {

class CoreSettingsApplier;

class EmulatorInstance {
public:
  /**
   * How many rewind snapshots are kept in memory
   */
  static constexpr size_t MAX_REWIND_POINTS = 10;

  EmulatorInstance(std::unique_ptr<::libretro::ICore>, std::string contentPath, std::string contentHash, int platformId,
                   int saveSlotNumber, std::vector<uint8_t> gameData, std::vector<uint8_t> saveData,
                   EmulationContext context);
  ~EmulatorInstance();

  bool initialize(libretro::IVideoDataReceiver *videoDataReceiver);
  bool isInitialized();

  std::string getContentHash() const;
  int getPlatformId() const;
  int getSaveSlotNumber() const;

  void runFrame();
  void reset();
  std::future<bool> save();

  void setMuted(bool muted);

  /**
   * Records how fast the game is being run, which decides whether it is heard
   */
  void setSpeedMultiplier(float multiplier);
  bool isMuted() const;

  // The mute state the AudioManager is born with in initialize(). Lets a CLI
  // `--muted` launch start muted from the first frame, before the render thread
  // creates the AudioManager (a QML muted binding fires too early to catch it)
  // Must be set before initialize()
  void setStartMuted(bool muted) { m_startMuted = muted; }

  // Suspends/resumes audio output when the game is paused/resumed, so buffered
  // audio doesn't keep playing after a pause
  void setPaused(bool paused);

  void setRewindEnabled(bool enabled);
  bool isRewindEnabled() const;

  // Hands every input straight to the game, for a system that wants the whole
  // keyboard. The per-device toggle_hotkeys shortcut sits on top of this
  void setHotkeysDisabled(bool disabled);

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

  float getAudioBufferLevel() const;

  void setAudioPlaybackRateRatio(double ratio);

  void setDynamicRateControlEnabled(bool enabled);
  bool getDynamicRateControlEnabled() const;

  void setInstantReplayEnabled(bool enabled);
  [[nodiscard]] bool getInstantReplayEnabled() const;

  // Sink for the host game stream (null when netplay isn't wired); the
  // renderer pushes each frame into it
  [[nodiscard]] media::IClipSink *getNetplayStreamSink() const { return m_context.netplayStreamSink; }

  /**
   * The latest frame this emulator produced, and the one place anything reads it from — the
   * renderer, screenshots, suspend-point thumbnails, the clip recorder, the netplay sink
   */
  [[nodiscard]] FrameSlot &getFrameSlot() { return m_frameSlot; }

  // --- Queued work ----------------------------------------------------------
  /**
   * Queues something to be done between frames. Safe to call from any thread
   */
  void submitCommand(const EmulatorCommand &command);

  /**
   * Does everything queued since the last call. Must run on the thread that runs frames, which is
   * what makes serializing a state safe without locking one out of the other
   */
  void drainCommands();

  /**
   * Where commands this instance doesn't own are sent — the ones that need pixels off a GPU, or a
   * QML image provider. Unset in a headless run, where those commands are simply dropped
   */
  void setCommandSink(std::function<void(const EmulatorCommand &)> sink) {
    std::lock_guard lock(m_hooksMutex);
    m_commandSink = std::move(sink);
  }

  /**
   * Where the picture attached to a suspend or rewind point comes from. Unset means points are
   * stored without one, which is what a run with nothing on screen wants
   */
  void setThumbnailProvider(std::function<Image()> provider) {
    std::lock_guard lock(m_hooksMutex);
    m_thumbnailProvider = std::move(provider);
  }

  /**
   * Where a point's own picture goes when the emulator is put back to it. Without this the screen
   * keeps the frame from the state that was just left, which for a game that is stopped is the last
   * thing it shows — the point was chosen by its picture, so that picture is what has to appear
   */
  void setFrameRestorer(std::function<void(const Image &)> restorer) {
    std::lock_guard lock(m_hooksMutex);
    m_frameRestorer = std::move(restorer);
  }

  /** When a rewind point was taken and what was on screen then */
  struct RewindPointPicture {
    int64_t timestamp = 0;
    Image image;
  };

  /**
   * The rewind points held in memory, newest first. Any thread
   */
  [[nodiscard]] std::vector<RewindPointPicture> getRewindPointPictures() const;

  /**
   * @return Whether undoing the last suspend-point load would do anything. Any thread
   */
  [[nodiscard]] bool canUndoLoadSuspendPoint() const;

  // Forward the two core-side input settings (glide speed for stick-driven
  // pointer devices; whether the physical mouse drives mouse/light-gun devices)
  void setAnalogPointerSpeed(double stepPerFrame);
  void setMouseControlsPointerDevices(bool enabled);

  std::vector<uint8_t> serializeState();
  bool deserializeState(const std::vector<uint8_t> &state);

  // Multi-disc control. getDiscCount() is 0 for single-disc/cartridge content
  // swapDisc publishes a DiscChangedEvent on success
  [[nodiscard]] unsigned getDiscCount() const;
  [[nodiscard]] unsigned getCurrentDiscIndex() const;
  bool swapDisc(unsigned index);

  // Raw per-port device options the running core advertises via
  // SET_CONTROLLER_INFO (empty when there's no choice)
  [[nodiscard]] std::vector<std::vector<::libretro::ICore::ControllerDeviceOption>> getControllerDevices() const;
  // Curated, console-native controller variants selectable for `port` on the
  // loaded core (default first), cross-referenced with the core's advertisement
  [[nodiscard]] std::vector<CoreDeviceVariant> getAvailableControllerVariants(unsigned port) const;
  // Number of controller ports the running core exposes (SET_CONTROLLER_INFO)
  [[nodiscard]] unsigned getControllerPortCount() const;
  // The coreDeviceId currently selected for `port` (the per-game override if
  // still valid, else the default) — lets the UI highlight the active choice
  [[nodiscard]] unsigned getSelectedControllerVariant(unsigned port) const;
  // Selects a variant (by its coreDeviceId) for `port`: applies it to the core
  // live, applies any companion core options, and persists it for this game
  void setPortControllerVariant(unsigned port, unsigned coreDeviceId);

  // --- cheats (Game Genie / Action Replay) ---
  // This game's saved cheats (from the cheat repository)
  [[nodiscard]] std::vector<cheats::Cheat> getCheats() const;
  // Toggles a cheat, persists it, and re-applies the active set
  void setCheatEnabled(int cheatId, bool enabled);
  // Adds a fully-formed cheat for this game (the UI / a decoder builds it,
  // resolving pokes for RAM cheats). Persisted; re-applied
  void addCheat(cheats::Cheat cheat);
  // Persists an edit and re-applies
  void updateCheat(const cheats::Cheat &cheat);
  // Removes a cheat and re-applies
  void removeCheat(int cheatId);

private:
  /**
   * Pushes the resampler correction the setting and the pacing mode agree on
   */
  void applyAudioRateControl() const;

  /**
   * Handles one queued command, or hands it on when it belongs to whoever owns the screen
   */
  void handleCommand(const EmulatorCommand &command);

  /**
   * Hands a point's picture to whatever is showing the game, if anything is and there is one
   */
  void restoreFrame(const Image &image);

  /**
   * Builds a point from the state as it stands, with a picture when something can supply one
   */
  [[nodiscard]] SuspendPoint capturePoint();

  std::mutex m_commandQueueMutex;
  std::deque<EmulatorCommand> m_commandQueue;
  std::function<void(const EmulatorCommand &)> m_commandSink;
  std::function<Image()> m_thumbnailProvider;
  std::function<void(const Image &)> m_frameRestorer;
  mutable std::mutex m_hooksMutex;

  // Rolling rewind snapshots, newest first, and the state replaced by the last suspend-point load
  std::deque<SuspendPoint> m_rewindPoints;
  SuspendPoint m_beforeLastLoadSuspendPoint;
  mutable std::mutex m_rewindPointsMutex;

  FrameSlot m_frameSlot;

  // Resolves the controller variant selected for a port: the per-game override
  // (by coreDeviceId) if still valid for the loaded core, else the default
  [[nodiscard]] CoreDeviceVariant resolveSelectedVariant(unsigned port) const;
  // Persists a variant's companion core options as game values so the core
  // picks them up (e.g. FCEUmm Zapper -> fceumm_zapper_mode=clightgun)
  void applyCompanionOptions(const CoreDeviceVariant &variant);

  // Rebuilds the active cheat set from the repository: RAM cheats go to the
  // per-frame engine, Game Genie codes to the core, and any cheat that affects
  // gameplay is skipped while RA hardcore mode is active. No-op without a repo
  void applyCheats();

  cheats::CheatEngine m_cheatEngine;

  // Written on the render thread when the core comes up, read by the thread that runs frames
  std::atomic<bool> m_initialized = false;

  EmulationContext m_context;
  std::unique_ptr<::libretro::ICore> m_core;

  // Keys on their way to a core that asked for the keyboard. They arrive on the
  // GUI thread and the core only runs on the render thread, so they queue here
  // and are handed over at the top of a frame rather than mid-run
  struct PendingKey {
    bool down;
    unsigned key;
    uint16_t modifiers;
  };

  std::vector<PendingKey> m_pendingKeys;
  std::mutex m_pendingKeysMutex;
  ScopedConnection m_keyboardKeyConnection;
  void drainKeyboardEvents();

  // Pushes the resolved hotkey state to the input service. Re-run once the core
  // is loaded, since only then is wantsKeyboard() meaningful
  void applyHotkeyState();
  bool m_hotkeysDisabled = false;
  std::shared_ptr<IAudioOutput> m_audioOutput;
  std::unique_ptr<libretro::IAudioInputProvider> m_audioInput;
  std::vector<uint8_t> m_gameData;
  std::vector<uint8_t> m_saveData;

  std::vector<SuspendPoint> m_rewindSuspendPoints;
  std::chrono::time_point<std::chrono::steady_clock> m_lastSaveTime;
  int m_saveIntervalSeconds = 10;
  // Holds the in-flight autosave. Discarding the future std::async returns would
  // block the render thread in its destructor until the write finished (a hitch
  // every save interval); keeping it here lets the write run in the background
  std::future<bool> m_pendingSave;

  std::string m_contentPath;
  std::string m_contentHash;
  int m_platformId;
  int m_saveSlotNumber;

  std::atomic<bool> m_isRewindEnabled{false};
  bool m_startMuted = false;
  std::string m_pictureMode;
  std::string m_aspectRatioMode;
  std::atomic<int> m_integerScale{0};
  std::string m_syncMethod;
  std::atomic<int> m_targetFramerate{0};
  std::atomic<bool> m_dynamicRateControl{true};

  mutable std::mutex m_settingsMutex;

  std::atomic<bool> m_instantReplayEnabled{false};

  // Observes setting changes and applies this game's resolved common settings
  /**
   * Silences the output when either the caller or the speed asks for it
   */
  void applyMuted() const;

  /**
   * Whether a game running at this speed should be heard
   */
  [[nodiscard]] bool shouldMuteAtSpeed(float multiplier) const;

  /**
   * Applies the pacing and the speed together, neither being the whole rate on its own
   */
  void applyAudioRatio() const;

  bool m_mutedByRequest = false;
  bool m_mutedBySpeed = false;

  double m_pacingAudioRatio = 1.0;
  double m_speedAudioRatio = 1.0;
  mutable std::mutex m_audioStateMutex;

  std::unique_ptr<CoreSettingsApplier> m_settingsApplier;

  monitoring::Span m_runFrameSpan = monitoring::Monitor::instance().span("run_frame", "The core advancing one frame");
  monitoring::Span m_saveKickSpan =
      monitoring::Monitor::instance().span("save_kick", "Starting the periodic save on its own thread");
  monitoring::Span m_retroRunSpan = monitoring::Monitor::instance().span("retro_run", "The core's own work");
  monitoring::Span m_cheatsSpan = monitoring::Monitor::instance().span("cheats", "Re-applying RAM cheats");
  monitoring::Span m_achievementsSpan =
      monitoring::Monitor::instance().span("achievements", "Evaluating achievements against the core's memory");
};

} // namespace firelight::emulation
