#pragma once

#include "core_input_router.hpp"
#include "firelight/libretro/audio_output.hpp"
#include "firelight/libretro/configuration_provider.hpp"
#include "firelight/libretro/core_run_config.hpp"
#include "firelight/libretro/icore.hpp"
#include "firelight/libretro/retropad_provider.hpp"
#include "firelight/libretro/video_data_receiver.hpp"
#include "game.hpp"
#include "libretro/libretro.h"

#include <array>
#include <filesystem>
#include <firelight/input/input_frame.hpp>
#include <firelight/libretro/pointer_input_provider.hpp>
#include <functional>
#include <map>
#include <qlibrary.h>
#include <unordered_map>
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

    class Core;
    class LibretroDll;

    // What the libretro C callbacks (video/audio/input/env/mic/rumble) reach the
    // active core through, without exposing Core's members. Populated by Core as
    // receivers/providers are set; read by the callbacks via a file-scope pointer.
    struct CoreCallbackContext {
        Core *core = nullptr;
        firelight::libretro::CoreInputRouter *input = nullptr;
        firelight::libretro::IVideoDataReceiver *video = nullptr;
        IAudioOutput *audio = nullptr;
        firelight::libretro::IAudioInputProvider *audioInput = nullptr;
        int platformId = -1;
    };

    class Core : public ICore {
    public:
        std::basic_string<char> dumpJson();

        explicit Core(firelight::libretro::CoreRunConfig config);

        virtual ~Core();

        void
        setVideoReceiver(firelight::libretro::IVideoDataReceiver *receiver) override;

        void
        setRetropadProvider(firelight::libretro::IRetropadProvider *provider) override;

        void setPointerInputProvider(
            firelight::libretro::IPointerInputProvider *provider) override;

        void setAudioInputProvider(
            firelight::libretro::IAudioInputProvider *provider) override;

        void setAudioReceiver(std::shared_ptr<IAudioOutput> receiver) override;

        bool handleEnvironmentCall(unsigned cmd, void *data);

        void init() override;

        void deinit();

        void reset() override;

        void run(double deltaTime) override;

        bool loadGame(Game *game) override;

        void unloadGame();

        std::vector<uint8_t> serializeState() const override;

        bool deserializeState(const std::vector<uint8_t> &data) const override;

        [[nodiscard]] std::size_t getSerializeSize() const override;

        void setSystemDirectory(const string &) override;

        void setSaveDirectory(const string &) override;

        [[nodiscard]] std::vector<char>
        getMemoryData(MemoryType memType) const override;

        void writeMemoryData(MemoryType memType,
                             const std::vector<char> &data) override;

        void *getMemoryData(unsigned id) const override;

        size_t getMemorySize(unsigned id) const override;

        retro_memory_map *getMemoryMap() override;

        unsigned getDiskCount() const override;

        unsigned getCurrentDiskIndex() const override;

        bool setDiskIndex(unsigned index) override;

        std::vector<std::vector<ControllerDeviceOption> >
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
            return m_input.mouseControlsPointerDevices();
        }

        void setCheat(unsigned index, bool enabled, const std::string &code) override;

        void clearCheats() override;

        std::function<void()> destroyContextFunction = nullptr;

        retro_system_av_info *retroSystemAVInfo;
        int m_platformId = -1;

    private:
        std::function<void()> m_destroyContextFunction = nullptr;

        Game *game;

        firelight::libretro::IVideoDataReceiver *videoReceiver = nullptr;
        firelight::libretro::CoreInputRouter m_input;
        // Backs the file-scope pointer the C callbacks read; kept current by the
        // receiver/provider setters.
        CoreCallbackContext m_callbackContext;

        std::shared_ptr<IAudioOutput> audioReceiver;
        std::shared_ptr<firelight::libretro::IConfigurationProvider>
        m_configurationProvider;
        firelight::libretro::IAudioInputProvider *m_audioInputProvider = nullptr;

        retro_vfs_interface m_vfsInterface;

        vector<string> environmentCalls;

        // One environment-call handler: its debug name + the body that services it.
        struct EnvHandler {
            const char *name;
            std::function<bool(void *data)> handler;
        };
        // cmd -> handler, built once (lazily) and dispatched by handleEnvironmentCall.
        std::unordered_map<unsigned, EnvHandler> m_envHandlers;
        void buildEnvironmentHandlers();

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
        string m_saveDirectory;
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
        std::vector<std::vector<ControllerDeviceOption> > m_controllerDevices;
        uint64_t inputDeviceCapabilitiesBitmask;
        retro_keyboard_callback *keyboardCallback;

        void recordPotentialAPIViolation(const string &msg);

        // The loaded core DLL + its resolved retro_* entry points.
        std::unique_ptr<LibretroDll> m_dll;
    };
} // namespace libretro
