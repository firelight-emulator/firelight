#pragma once

#include "firelight/libretro/audio_data_receiver.hpp"
#include "firelight/libretro/configuration_provider.hpp"
#include "firelight/libretro/icore.hpp"
#include "firelight/libretro/retropad_provider.hpp"
#include "firelight/libretro/video_data_receiver.hpp"
#include "game.hpp"
#include "libretro/libretro.h"

#include <filesystem>
#include <firelight/libretro/pointer_input_provider.hpp>
#include <functional>
#include <map>
#include <qlibrary.h>
#include <vector>

using std::array;
using std::string;
using std::vector;

namespace libretro {
// MemoryType now lives in firelight/libretro/icore.hpp.

typedef void (*RetroSetEnvironment)(bool (*)(unsigned cmd, void *data));

typedef void (*RetroSetVideoRefresh)(retro_video_refresh_t);

typedef void (*RetroSetAudioSample)(retro_audio_sample_t);

typedef void (*RetroSetAudioSampleBatch)(retro_audio_sample_batch_t);

typedef void (*RetroInputState)(retro_input_state_t);

typedef void (*RetroInputPoll)(retro_input_poll_t);

typedef void (*RetroRunFunc)();

class Core : public ICore {
public:
  std::basic_string<char> dumpJson();

  Core(int platformId, const std::string &libPath,
       const std::shared_ptr<firelight::libretro::IConfigurationProvider>
           &configProvider,
       std::string systemDirectory);

  virtual ~Core();

  void
  setVideoReceiver(firelight::libretro::IVideoDataReceiver *receiver) override;

  void
  setRetropadProvider(firelight::libretro::IRetropadProvider *provider) override;

  void setPointerInputProvider(
      firelight::libretro::IPointerInputProvider *provider) override;

  [[nodiscard]] firelight::libretro::IPointerInputProvider *
  getPointerInputProvider() const;

  [[nodiscard]] firelight::libretro::IRetropadProvider *
  getRetropadProvider() const;

  void setAudioReceiver(std::shared_ptr<IAudioDataReceiver> receiver) override;

  bool handleEnvironmentCall(unsigned cmd, void *data);

  void init() override;

  void deinit();

  void reset() override;

  void run(double deltaTime) override;

  bool loadGame(Game *game) override;

  void unloadGame();

  std::vector<uint8_t> serializeState() const override;

  void deserializeState(const std::vector<uint8_t> &data) const override;

  size_t getSerializeSize() const;

  void setSystemDirectory(const string &) override;

  void setSaveDirectory(const string &) override;

  [[nodiscard]] std::vector<char>
  getMemoryData(MemoryType memType) const override;

  void writeMemoryData(MemoryType memType,
                       const std::vector<char> &data) override;

  firelight::libretro::IVideoDataReceiver *videoReceiver;

  void *getMemoryData(unsigned id) const override;

  size_t getMemorySize(unsigned id) const override;

  retro_memory_map *getMemoryMap() override;

  unsigned getDiskCount() const override;
  unsigned getCurrentDiskIndex() const override;
  bool setDiskIndex(unsigned index) override;

  std::vector<std::vector<ControllerDeviceOption>>
  getControllerDevices() const override;
  void setControllerPortDevice(unsigned port, unsigned device) override;
  void setPortInputDeviceClass(unsigned port, int deviceClass) override;
  void setAnalogPointerSpeed(double stepPerFrame) override;
  void setMouseControlsPointerDevices(bool enabled) override;
  // The user-selected input device class for `port` (GamepadInputClass value;
  // 1=Joypad when unset). Gates mouse/light-gun handling on the selected device.
  [[nodiscard]] int getPortInputClass(unsigned port) const;
  // Whether the physical mouse may drive light-gun / mouse devices (the toggle).
  [[nodiscard]] bool mouseControlsPointerDevices() const {
    return m_mouseControlsPointerDevices;
  }

  void setCheat(unsigned index, bool enabled, const std::string &code) override;
  void clearCheats() override;

  std::function<void()> destroyContextFunction = nullptr;

  retro_system_av_info *retroSystemAVInfo;
  int m_platformId = -1;

  // Snapshots per-frame mouse motion from the pointer provider (called from the
  // libretro input-poll callback), so a single relative-motion read serves both
  // the MOUSE_X and MOUSE_Y queries within a frame.
  void pollInput();
  std::pair<int16_t, int16_t> m_frameMouseDelta{0, 0};

private:
  // Resolved input device class per port (firelight::input::GamepadInputClass
  // value: 1=Joypad, 2=Mouse, 3=Light Gun) and the analog-stick glide speed.
  // pollInput() uses these to drive the pointer cursor from a gamepad stick on
  // Mouse/Light-Gun ports.
  std::map<unsigned, int> m_portInputClass;
  double m_analogPointerSpeed = 0.025;
  bool m_mouseControlsPointerDevices = true;

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
  // The disc-control interface a multi-disc core registers, stored by value
  // (the struct the core passes via the environment call may be transient).
  retro_disk_control_callback m_diskControl{};
  retro_disk_control_ext_callback m_diskControlExt{};
  bool m_hasDiskControl = false;
  bool m_hasDiskControlExt = false;
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
  // Per-port device options the core advertised via SET_CONTROLLER_INFO.
  std::vector<std::vector<ControllerDeviceOption>> m_controllerDevices;
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
