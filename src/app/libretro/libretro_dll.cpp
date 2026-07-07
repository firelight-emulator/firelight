#include "libretro_dll.hpp"

#include <qlibrary.h>
#include <stdexcept>
#include <vector>

namespace libretro {

LibretroDll::LibretroDll(const std::string &path) {
  m_lib = std::make_unique<QLibrary>(QString::fromStdString(path));
  if (!m_lib->load()) {
    throw std::runtime_error("Failed to load libretro core '" + path +
                             "': " + m_lib->errorString().toStdString());
  }

  // Resolve every libretro entry point up front. A compliant core exports all of
  // them (features it doesn't support are stubbed), so a missing symbol means a
  // broken/incompatible core; collect any misses and fail with a clear message
  // instead of crashing on a null call later.
  std::vector<std::string> missing;
  const auto req = [&](const char *name) -> QFunctionPointer {
    const auto ptr = m_lib->resolve(name);
    if (!ptr) {
      missing.emplace_back(name);
    }
    return ptr;
  };

  m_init = reinterpret_cast<void (*)()>(req("retro_init"));
  m_deinit = reinterpret_cast<void (*)()>(req("retro_deinit"));
  m_apiVersion = reinterpret_cast<unsigned (*)()>(req("retro_api_version"));
  m_getSystemInfo = reinterpret_cast<void (*)(retro_system_info *)>(
    req("retro_get_system_info"));
  m_getSystemAVInfo = reinterpret_cast<void (*)(retro_system_av_info *)>(
    req("retro_get_system_av_info"));
  m_setControllerPortDevice = reinterpret_cast<void (*)(unsigned, unsigned)>(
    req("retro_set_controller_port_device"));
  m_reset = reinterpret_cast<void (*)()>(req("retro_reset"));
  m_run = reinterpret_cast<void (*)()>(req("retro_run"));
  m_serializeSize =
      reinterpret_cast<size_t (*)()>(req("retro_serialize_size"));
  m_serialize =
      reinterpret_cast<bool (*)(void *, size_t)>(req("retro_serialize"));
  m_unserialize = reinterpret_cast<bool (*)(const void *, size_t)>(
    req("retro_unserialize"));
  m_cheatReset = reinterpret_cast<void (*)()>(req("retro_cheat_reset"));
  m_cheatSet = reinterpret_cast<void (*)(unsigned, bool, const char *)>(
    req("retro_cheat_set"));
  m_loadGame = reinterpret_cast<bool (*)(const retro_game_info *)>(
    req("retro_load_game"));
  m_loadGameSpecial =
      reinterpret_cast<bool (*)(unsigned, const retro_game_info *, size_t)>(
        req("retro_load_game_special"));
  m_unloadGame = reinterpret_cast<void (*)()>(req("retro_unload_game"));
  m_getRegion = reinterpret_cast<unsigned (*)()>(req("retro_get_region"));
  m_getMemoryData =
      reinterpret_cast<void *(*)(unsigned)>(req("retro_get_memory_data"));
  m_getMemorySize =
      reinterpret_cast<size_t (*)(unsigned)>(req("retro_get_memory_size"));

  m_setEnvironment = reinterpret_cast<void (*)(retro_environment_t)>(
    req("retro_set_environment"));
  m_setVideoRefresh = reinterpret_cast<void (*)(retro_video_refresh_t)>(
    req("retro_set_video_refresh"));
  m_setAudioSample = reinterpret_cast<void (*)(retro_audio_sample_t)>(
    req("retro_set_audio_sample"));
  m_setAudioSampleBatch = reinterpret_cast<void (*)(retro_audio_sample_batch_t)>(
    req("retro_set_audio_sample_batch"));
  m_setInputPoll = reinterpret_cast<void (*)(retro_input_poll_t)>(
    req("retro_set_input_poll"));
  m_setInputState = reinterpret_cast<void (*)(retro_input_state_t)>(
    req("retro_set_input_state"));

  if (!missing.empty()) {
    std::string joined;
    for (size_t i = 0; i < missing.size(); ++i) {
      joined += (i == 0 ? "" : ", ") + missing[i];
    }
    m_lib->unload();
    throw std::runtime_error("libretro core '" + path +
                             "' is missing required symbol(s): " + joined);
  }
}

LibretroDll::~LibretroDll() = default;

void LibretroDll::init() { m_init(); }
void LibretroDll::deinit() { m_deinit(); }
void LibretroDll::reset() { m_reset(); }
void LibretroDll::run() { m_run(); }
bool LibretroDll::loadGame(const retro_game_info *info) { return m_loadGame(info); }
void LibretroDll::unloadGame() { m_unloadGame(); }
void LibretroDll::getSystemInfo(retro_system_info *info) { m_getSystemInfo(info); }

void LibretroDll::getSystemAVInfo(retro_system_av_info *info) {
  m_getSystemAVInfo(info);
}

void LibretroDll::setControllerPortDevice(unsigned port, unsigned device) {
  if (m_setControllerPortDevice) {
    m_setControllerPortDevice(port, device);
  }
}

size_t LibretroDll::serializeSize() { return m_serializeSize(); }
bool LibretroDll::serialize(void *data, size_t size) { return m_serialize(data, size); }

bool LibretroDll::unserialize(const void *data, size_t size) {
  return m_unserialize(data, size);
}

void LibretroDll::cheatReset() {
  if (m_cheatReset) {
    m_cheatReset();
  }
}

void LibretroDll::cheatSet(unsigned index, bool enabled, const char *code) {
  if (m_cheatSet) {
    m_cheatSet(index, enabled, code);
  }
}

void *LibretroDll::getMemoryData(unsigned id) { return m_getMemoryData(id); }
size_t LibretroDll::getMemorySize(unsigned id) { return m_getMemorySize(id); }

void LibretroDll::setEnvironment(retro_environment_t cb) { m_setEnvironment(cb); }
void LibretroDll::setVideoRefresh(retro_video_refresh_t cb) {
  m_setVideoRefresh(cb);
}
void LibretroDll::setAudioSample(retro_audio_sample_t cb) { m_setAudioSample(cb); }
void LibretroDll::setAudioSampleBatch(retro_audio_sample_batch_t cb) {
  m_setAudioSampleBatch(cb);
}
void LibretroDll::setInputPoll(retro_input_poll_t cb) { m_setInputPoll(cb); }
void LibretroDll::setInputState(retro_input_state_t cb) { m_setInputState(cb); }

void LibretroDll::unload() {
  if (m_lib) {
    m_lib->unload();
  }
}

bool LibretroDll::isLoaded() const { return m_lib && m_lib->isLoaded(); }

} // namespace libretro
