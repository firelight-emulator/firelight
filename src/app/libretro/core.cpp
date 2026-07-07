#include "core.hpp"
#include "core_environment.hpp"
#include "libretro_dll.hpp"
#include "libretro/libretro_vulkan.h"

#include "SDL2/SDL.h"
#include "virtual_filesystem.hpp"
#include <cstdarg>
#include <filesystem>
#include <firelight/input/gamepad_input.hpp>
#include <stdexcept>
#include <vector>
#include <utility>

#include <spdlog/spdlog.h>

namespace libretro {
  // Only supports one core at a time for now, but, eh. The libretro C callbacks
  // reach the active core through this context (set on load, cleared on unload).
  // Declared in core_environment.hpp so the env-call handlers can read it too.
  CoreCallbackContext *g_ctx = nullptr;

  void log(enum retro_log_level level, const char *fmt, ...) {
    char msg[4096] = {};
    va_list va;
    va_start(va, fmt);
    vsnprintf(msg, sizeof(msg), fmt, va);
    va_end(va);

    msg[std::remove(msg, msg + strlen(msg), '\n') - msg] = 0;
    msg[std::remove(msg, msg + strlen(msg), '\r') - msg] = 0;

    // mGBA likes to spam the logs... though I could probably check the level.
    // if (strncmp(msg, "GBA DMA", 7) == 0) {
    //   return;
    // }

    if (level == RETRO_LOG_INFO) {
      spdlog::info("[Core] {}", msg);
    } else if (level == RETRO_LOG_DEBUG) {
      spdlog::debug("[Core] {}", msg);
    } else if (level == RETRO_LOG_WARN) {
      spdlog::warn("[Core] {}", msg);
    } else if (level == RETRO_LOG_ERROR) {
      spdlog::error("[Core] {}", msg);
    } else {
      spdlog::info("[Core] {}", msg); // Default to info for unknown levels.
    }
  }

  static int16_t inputStateCallback(unsigned port, unsigned device,
                                    unsigned index, unsigned id) {
    if (g_ctx == nullptr || g_ctx->input == nullptr) {
      // TODO: Report some error
      return 0;
    }

    return g_ctx->input->readInputState(port, device, index, id);
  }

  static void videoCallback(const void *data, unsigned width, unsigned height,
                            size_t pitch) {
    if (!g_ctx || !g_ctx->video) {
      return;
    }
    g_ctx->video->receive(data, width, height, pitch);
  }

  static bool envCallback(unsigned cmd, void *data) {
    if (!g_ctx || !g_ctx->core) {
      return false;
    }
    return g_ctx->core->handleEnvironmentCall(cmd, data);
  }


  template<typename T>
  static T loadRetroFunc(void *dll, const char *name) {
    // TODO error checking
    auto result = reinterpret_cast<T>(SDL_LoadFunction(dll, name));
    if (result == nullptr) {
      // std::cout << SDL_GetError() << std::endl;
    }
    return result;
  }

  Core::Core(firelight::libretro::CoreRunConfig config)
    : m_platformId(config.platformId),
      m_configurationProvider(std::move(config.configProvider)),
      systemDirectory(std::move(config.systemDirectory)),
      m_saveDirectory(std::move(config.saveDirectory)) {
    m_input.setPlatformId(config.platformId);
    m_dll = std::make_unique<LibretroDll>(config.corePath);

    retroSystemInfo = new retro_system_info;
    retroSystemAVInfo = new retro_system_av_info;

    m_callbackContext.core = this;
    m_callbackContext.input = &m_input;
    m_callbackContext.platformId = m_platformId;
    g_ctx = &m_callbackContext;

    m_dll->setEnvironment(envCallback);
    m_dll->setVideoRefresh(videoCallback);
    m_dll->setAudioSample([](int16_t left, int16_t right) {
    });

    auto processAudioLambda = [](const int16_t *data, size_t frames) -> size_t {
      if (g_ctx == nullptr || g_ctx->audio == nullptr) {
        return frames;
      }

      return g_ctx->audio->receive(data, frames);
    };

    m_dll->setAudioSampleBatch(processAudioLambda);
    m_dll->setInputPoll([] {
      if (g_ctx && g_ctx->input) {
        g_ctx->input->pollInput();
      }
    });
    m_dll->setInputState(inputStateCallback);
  }

  Core::~Core() {
    spdlog::info("[Core] Unloading core");
    // Teardown order matters for HW-rendered cores (e.g. PPSSPP): the frontend
    // owns the VkDevice, but the core keeps using it through retro_unload_game /
    // retro_deinit — PPSSPP's Shutdown() calls vkDeviceWaitIdle, and its own
    // vkDestroyDevice is a no-op (the frontend must destroy the device). So run
    // the core's context_destroy, fully deinit the core, and only THEN tear down
    // the HW context/device — all while the DLL is still loaded, since
    // destroy_device is invoked from destroyHwContext() and needs the DLL.
    if (m_destroyContextFunction) {
      m_destroyContextFunction();
      m_destroyContextFunction = nullptr;
    }
    if (g_ctx != nullptr && m_dll != nullptr) {
      unloadGame();
      deinit();
      if (videoReceiver) {
        videoReceiver->destroyHwContext();
      }
      m_dll->unload();
      g_ctx = nullptr;
    } else if (videoReceiver) {
      // No loaded DLL to deinit, but still release any HW context we created.
      videoReceiver->destroyHwContext();
    }
  }

  bool Core::loadGame(Game *game) {
    this->game = game;
    retro_game_info info{};

    // info.path = game->getPath().c_str();
    info.path = strdup(
      std::filesystem::path(game->getPath()).string().c_str());
    // info.path = R"()";
    info.data = game->getData();
    info.size = game->getSize();
    info.meta = "";

    // spdlog::warn("Path before c_str: {}", game->getPath());
    // spdlog::warn("Path after c_str: {}", string(info.path));
    // TODO: meta?
    auto result = m_dll->loadGame(&info);

    m_dll->getSystemAVInfo(retroSystemAVInfo);
    if (videoReceiver) {
      videoReceiver->setSystemAVInfo(retroSystemAVInfo);
    }
    //  video->setGameGeometry(&retroSystemAVInfo->geometry);

    audioReceiver->initialize(retroSystemAVInfo->timing.sample_rate);
    // this->game = nullptr;
    return result;
  }

  void Core::unloadGame() { m_dll->unloadGame(); }

  std::vector<uint8_t> Core::serializeState() const {
    const auto size = m_dll->serializeSize();

    std::vector<uint8_t> data(size);
    if (!m_dll->serialize(data.data(), size)) {
      spdlog::error("[Core] Failed to serialize state ({} bytes)", size);
      return {};
    }

    return data;
  }

  bool Core::deserializeState(const std::vector<uint8_t> &data) const {
    const auto size = getSerializeSize();

    // A mismatched size means the state is for a different game/core version;
    // unserializing it could corrupt the running game, so refuse.
    if (data.size() != size) {
      spdlog::error(
        "[Core] Refusing to load state: size mismatch (data: {}, expected: {})",
        data.size(), size);
      return false;
    }

    if (!m_dll->unserialize(data.data(), size)) {
      spdlog::error("[Core] Core rejected state ({} bytes)", size);
      return false;
    }
    return true;
  }

  std::size_t Core::getSerializeSize() const { return m_dll->serializeSize(); }

  void Core::init() {
    m_dll->init();
    m_dll->getSystemInfo(retroSystemInfo);

    spdlog::info("[Core] Libretro core loaded. Core info:");
    spdlog::info("[Core]   Library name: {}", retroSystemInfo->library_name);
    spdlog::info("[Core]   Library version: {}", retroSystemInfo->library_version);
    spdlog::info("[Core]   Valid extensions: {}", retroSystemInfo->valid_extensions);
    spdlog::info("[Core]   Need full path: {}", retroSystemInfo->need_fullpath);
    spdlog::info("[Core]   Block extraction: {}", retroSystemInfo->block_extract);
  }

  void Core::deinit() { m_dll->deinit(); }

  void Core::reset() { m_dll->reset(); }

  void Core::run(double deltaTime) { m_dll->run(); }

  void Core::setSystemDirectory(const string &frontendSystemDirectory) {
    systemDirectory = frontendSystemDirectory;
  }

  void Core::setSaveDirectory(const string &frontendSaveDirectory) {
    m_saveDirectory = frontendSaveDirectory;
  }

  void Core::recordPotentialAPIViolation(const std::string &msg) {
    printf("Potential API violation: %s\n", msg.c_str());
  }

  std::vector<char> Core::getMemoryData(const MemoryType memType) const {
    const auto size = m_dll->getMemorySize(static_cast<unsigned>(memType));
    const auto ptr = static_cast<char *>(
      m_dll->getMemoryData(static_cast<unsigned>(memType)));

    return std::vector(ptr, ptr + size);
  }

  void Core::writeMemoryData(const MemoryType memType,
                             const std::vector<char> &data) {
    const auto size = m_dll->getMemorySize(static_cast<unsigned>(memType));
    const auto ptr = m_dll->getMemoryData(static_cast<unsigned>(memType));

    if (data.size() > size) {
      spdlog::error("Data size is larger than memory size");
      return;
    }

    memcpy(ptr, data.data(), data.size());
  }

  void *Core::getMemoryData(const unsigned id) const {
    return m_dll->getMemoryData(id);
  }

  size_t Core::getMemorySize(const unsigned id) const {
    return m_dll->getMemorySize(id);
  }

  retro_memory_map *Core::getMemoryMap() { return &memoryMap; }

  unsigned Core::getDiskCount() const {
    if (m_hasDiskControlExt && m_diskControlExt.get_num_images) {
      return m_diskControlExt.get_num_images();
    }
    if (m_hasDiskControl && m_diskControl.get_num_images) {
      return m_diskControl.get_num_images();
    }
    return 0;
  }

  unsigned Core::getCurrentDiskIndex() const {
    if (m_hasDiskControlExt && m_diskControlExt.get_image_index) {
      return m_diskControlExt.get_image_index();
    }
    if (m_hasDiskControl && m_diskControl.get_image_index) {
      return m_diskControl.get_image_index();
    }
    return 0;
  }

  bool Core::setDiskIndex(const unsigned index) {
    // Prefer the extended interface's functions when present; both expose the
    // same base eject/set-index/count callbacks.
    const auto setEject =
        m_hasDiskControlExt
          ? m_diskControlExt.set_eject_state
          : (m_hasDiskControl
               ? m_diskControl.set_eject_state
               : nullptr);
    const auto setIndex =
        m_hasDiskControlExt
          ? m_diskControlExt.set_image_index
          : (m_hasDiskControl
               ? m_diskControl.set_image_index
               : nullptr);
    if (!setEject || !setIndex || index >= getDiskCount()) {
      return false;
    }
    // A disc swap is eject -> select -> insert.
    setEject(true);
    const bool ok = setIndex(index);
    setEject(false);
    return ok;
  }

  std::vector<std::vector<Core::ControllerDeviceOption> >
  Core::getControllerDevices() const {
    return m_controllerDevices;
  }

  void Core::setControllerPortDevice(const unsigned port,
                                     const unsigned device) {
    m_dll->setControllerPortDevice(port, device);
  }

  void Core::setPortInputDeviceClass(const unsigned port, const int deviceClass) {
    m_input.setPortInputDeviceClass(port, deviceClass);
  }

  void Core::setAnalogPointerSpeed(const double stepPerFrame) {
    m_input.setAnalogPointerSpeed(stepPerFrame);
  }

  void Core::setMouseControlsPointerDevices(const bool enabled) {
    m_input.setMouseControlsPointerDevices(enabled);
  }

  int Core::getPortInputClass(const unsigned port) const {
    return m_input.getPortInputClass(port);
  }

  void Core::setCheat(const unsigned index, const bool enabled,
                      const std::string &code) {
    m_dll->cheatSet(index, enabled, code.c_str());
  }

  void Core::clearCheats() {
    m_dll->cheatReset();
  }

  void Core::setVideoReceiver(firelight::libretro::IVideoDataReceiver *receiver) {
    videoReceiver = receiver;
    m_callbackContext.video = receiver;
  }

  void Core::setRetropadProvider(
    firelight::libretro::IRetropadProvider *provider) {
    m_input.setRetropadProvider(provider);
  }

  void Core::setPointerInputProvider(
    firelight::libretro::IPointerInputProvider *provider) {
    m_input.setPointerInputProvider(provider);
  }

  void Core::setAudioInputProvider(
    firelight::libretro::IAudioInputProvider *provider) {
    m_audioInputProvider = provider;
    m_callbackContext.audioInput = provider;
  }

  void Core::setAudioReceiver(std::shared_ptr<IAudioOutput> receiver) {
    audioReceiver = std::move(receiver);
    m_callbackContext.audio = audioReceiver.get();
  }
} // namespace libretro
