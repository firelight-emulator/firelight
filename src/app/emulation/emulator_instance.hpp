#pragma once
#include <audio/audio_manager.hpp>
#include <event_dispatcher.hpp>
#include <future>
#include <libretro/core.hpp>
#include <manager_accessor.hpp>
#include <memory>
#include <saves/suspend_point.hpp>
#include <string>

namespace firelight::emulation {

class EmulatorInstance : public ManagerAccessor {
public:
  EmulatorInstance(std::unique_ptr<::libretro::Core>, std::string contentPath,
                   std::string contentHash, int platformId, int saveSlotNumber,
                   std::vector<uint8_t> gameData,
                   std::vector<uint8_t> saveData);
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

  void setRewindEnabled(bool enabled);
  bool isRewindEnabled() const;

  void setPictureMode(const std::string &pictureMode);
  std::string getPictureMode() const;

  void setAspectRatioMode(const std::string &aspectRatioMode);
  std::string getAspectRatioMode() const;

  std::vector<uint8_t> serializeState();
  void deserializeState(const std::vector<uint8_t> &state);

  ::libretro::Core *getCore();

private:
  bool m_initialized = false;

  std::unique_ptr<::libretro::Core> m_core;
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

  bool m_isRewindEnabled = true;
  std::string m_pictureMode = "aspect-ratio-fill";
  std::string m_aspectRatioMode = "emulator-corrected";

  // Settings
  settings::SettingsLevel m_currentSettingsLevel;
  void refreshAllSettings();

  ScopedConnection m_platformSettingChangedConnection;
  ScopedConnection m_gameSettingChangedConnection;
  ScopedConnection m_settingsLevelChangedConnection;
};

} // namespace firelight::emulation