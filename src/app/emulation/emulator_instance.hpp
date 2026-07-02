#pragma once
#include "emulation_context.hpp"

#include <audio/audio_manager.hpp>
#include <firelight/event_dispatcher.hpp>
#include <firelight/libretro/icore.hpp>
#include <future>
#include <memory>
#include <firelight/saves/suspend_point.hpp>
#include <string>

namespace firelight::emulation {

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

  void setMuted(bool muted);
  bool isMuted() const;

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

  std::vector<uint8_t> serializeState();
  void deserializeState(const std::vector<uint8_t> &state);

private:
  bool m_initialized = false;

  EmulationContext m_context;
  std::unique_ptr<::libretro::ICore> m_core;
  std::shared_ptr<AudioManager> m_audioManager;
  std::vector<uint8_t> m_gameData;
  std::vector<uint8_t> m_saveData;

  std::vector<SuspendPoint> m_rewindSuspendPoints;
  std::chrono::time_point<std::chrono::steady_clock> m_lastSaveTime;
  int m_saveIntervalSeconds = 10;

  std::string m_contentPath;
  std::string m_contentHash;
  int m_platformId;
  int m_saveSlotNumber;

  // Populated by refreshAllSettings() (called in the constructor); the declared
  // defaults live in the settings catalog, not here. These initial values are
  // just placeholders.
  bool m_isRewindEnabled = false;
  std::string m_pictureMode;
  std::string m_aspectRatioMode;
  int m_integerScale = 0;
  std::string m_syncMethod;
  int m_targetFramerate = 0;

  // Settings — resolved by inheritance (game -> platform -> global -> default),
  // so any change at any tier that affects this game triggers a refresh.
  void refreshAllSettings();

  ScopedConnection m_platformSettingChangedConnection;
  ScopedConnection m_gameSettingChangedConnection;
  ScopedConnection m_globalSettingChangedConnection;
};

} // namespace firelight::emulation