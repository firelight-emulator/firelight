#pragma once

#include "firelight/libretro/audio_data_receiver.hpp"
#include "firelight/libretro/configuration_provider.hpp"
#include "firelight/libretro/retropad_provider.hpp"
#include "firelight/libretro/video_data_receiver.hpp"
#include "game.hpp"
#include "libretro/libretro.h"

#include <filesystem>
#include <firelight/libretro/pointer_input_provider.hpp>
#include <functional>
#include <qlibrary.h>
#include <vector>

using std::array;
using std::string;
using std::vector;

namespace libretro {
enum MemoryType {
  SAVE_RAM = RETRO_MEMORY_SAVE_RAM,
  RTC = RETRO_MEMORY_RTC,
  SYSTEM_RAM = RETRO_MEMORY_SYSTEM_RAM,
  VIDEO_RAM = RETRO_MEMORY_VIDEO_RAM
};

typedef void (*RetroSetEnvironment)(bool (*)(unsigned cmd, void *data));

typedef void (*RetroSetVideoRefresh)(retro_video_refresh_t);

typedef void (*RetroSetAudioSample)(retro_audio_sample_t);

typedef void (*RetroSetAudioSampleBatch)(retro_audio_sample_batch_t);

typedef void (*RetroInputState)(retro_input_state_t);

typedef void (*RetroInputPoll)(retro_input_poll_t);

typedef void (*RetroRunFunc)();

class Core {
public:
  std::basic_string<char> dumpJson();

  Core(int platformId, const std::string &libPath,
       const std::shared_ptr<firelight::libretro::IConfigurationProvider>
           &configProvider,
       std::string systemDirectory);

  virtual ~Core();

  void setVideoReceiver(firelight::libretro::IVideoDataReceiver *receiver);

  void setRetropadProvider(firelight::libretro::IRetropadProvider *provider);

  void
  setPointerInputProvider(firelight::libretro::IPointerInputProvider *provider);

  [[nodiscard]] firelight::libretro::IPointerInputProvider *
  getPointerInputProvider() const;

  [[nodiscard]] firelight::libretro::IRetropadProvider *
  getRetropadProvider() const;

  void setAudioReceiver(std::shared_ptr<IAudioDataReceiver> receiver);

  bool handleEnvironmentCall(unsigned cmd, void *data);

  void init();

  void deinit();

  void reset();

  void run(double deltaTime);

  bool loadGame(Game *game);

  void unloadGame();

  std::vector<uint8_t> serializeState() const;

  void deserializeState(const std::vector<uint8_t> &data) const;

  size_t getSerializeSize() const;

  void setSystemDirectory(const string &);

  void setSaveDirectory(const string &);

  [[nodiscard]] std::vector<char> getMemoryData(MemoryType memType) const;

  void writeMemoryData(MemoryType memType, const std::vector<char> &data);

  firelight::libretro::IVideoDataReceiver *videoReceiver;

  void *getMemoryData(unsigned id) const;

  size_t getMemorySize(unsigned id) const;

  retro_memory_map *getMemoryMap();

  std::function<void()> destroyContextFunction = nullptr;

  retro_system_av_info *retroSystemAVInfo;
  int m_platformId = -1;

private:
  std::unique_ptr<QLibrary> coreLib;
  std::function<void()> m_destroyContextFunction = nullptr;

  Game *game;

  firelight::libretro::IRetropadProvider *m_retropadProvider;
  std::shared_ptr<IAudioDataReceiver> audioReceiver;
  std::shared_ptr<firelight::libretro::IConfigurationProvider>
      m_configurationProvider;
  firelight::libretro::IPointerInputProvider *m_pointerInputProvider;

  retro_vfs_interface m_vfsInterface;

  vector<string> environmentCalls;

  retro_system_info *retroSystemInfo;

  // Informational to frontend.
  bool canRunWithNoGame = false;
  unsigned performanceLevel = 0;
  bool supportsAchievements = false;
  bool shutdown = false;
  vector<retro_input_descriptor> inputDescriptors;

  // Informational to core.
  string systemDirectory;
  string coreAssetsDirectory;
  string saveDirectory;
  string libretroPath;
  string username;
  unsigned frontendLanguage;
  bool isJITCapable;

  retro_disk_control_callback *diskControlCallback;
  unsigned diskControlInterfaceVersion;
  retro_disk_control_ext_callback *diskControlExtCallback;
  retro_rumble_interface *rumbleInterface;
  uint64_t serializationQuirksBitmap;
  retro_vfs_interface_info *virtualFileSystemInterfaceInfo;
  retro_led_interface *ledInterface;
  unsigned messageInterfaceVersion;
  retro_message_ext *messageExt; // todo
  retro_fastforwarding_override *fastforwardingOverride;
  retro_system_content_info_override *contentInfoOverride;
  retro_game_info_ext *gameInfoExt;
  retro_throttle_state *throttleState;
  int saveStateContext;
  retro_microphone_interface *microphoneInterface;
  retro_netpacket_callback *netpacketCallback;
  retro_device_power *devicePower;
  bool fastforwarding;

  retro_sensor_interface *sensorInterface;
  retro_camera_callback *cameraCallback;
  retro_log_callback *logCallback;
  retro_perf_callback *performanceCallback;
  retro_location_callback *locationCallback;
  retro_get_proc_address_interface *procAddressCallback;
  vector<retro_subsystem_info> subsystemInfo;
  vector<retro_memory_descriptor> memoryDescriptors;

  retro_memory_map memoryMap{};

  int audioVideoEnableBitmap;

  retro_audio_callback *audioCallback;
  unsigned minimumAudioLatency;
  retro_midi_interface *midiInterface;
  retro_audio_buffer_status_callback *audioBufferStatusCallback;

  unsigned numActiveInputDevices;
  bool supportsInputBitmasks;
  vector<retro_controller_description> controllerInfo;
  uint64_t inputDeviceCapabilitiesBitmask;
  retro_keyboard_callback *keyboardCallback;

  void recordPotentialAPIViolation(const string &msg);

  void *dll;

  void (*symRetroInit)();

  void (*symRetroDeinit)();

  unsigned (*symRetroApiVersion)();

  void (*symRetroGetSystemInfo)(retro_system_info *);

  void (*symRetroGetSystemAVInfo)(retro_system_av_info *);

  void (*symRetroSetControllerPortDevice)(unsigned, unsigned);

  void (*symRetroReset)();

  RetroRunFunc symRetroRun;

  size_t (*symRetroSerializeSize)();

  bool (*symRetroSerialize)(void *, size_t);

  bool (*symRetroUnserialize)(const void *, size_t);

  void (*symRetroCheatReset)();

  void (*symRetroCheatSet)(unsigned, bool, const char *);

  bool (*symRetroLoadGame)(const retro_game_info *);

  bool (*symRetroLoadGameSpecial)(unsigned, const retro_game_info *, size_t);

  void (*symRetroUnloadGame)();

  unsigned int (*symRetroGetRegion)();

  void *(*symRetroGetMemoryData)(unsigned);

  size_t (*symRetroGetMemoryDataSize)(unsigned);
};
} // namespace libretro
