#include "emulator_instance.hpp"

#include <spdlog/spdlog.h>

#include <utility>

namespace firelight::emulation {
EmulatorInstance::EmulatorInstance(
    std::unique_ptr<::libretro::Core> core, std::string contentPath,
    std::string contentHash, const int platformId, const int saveSlotNumber,
    std::vector<uint8_t> gameData, std::vector<uint8_t> saveData)
    : m_core(std::move(core)), m_gameData(std::move(gameData)),
      m_saveData(std::move(saveData)), m_contentPath(std::move(contentPath)),
      m_contentHash(std::move(contentHash)), m_platformId(platformId),
      m_saveSlotNumber(saveSlotNumber) {}

EmulatorInstance::~EmulatorInstance() {
  spdlog::info("[EmulatorInstance] Shutting down");
}

bool EmulatorInstance::initialize(
    libretro::IVideoDataReceiver *videoDataReceiver) {
  m_core->setVideoReceiver(videoDataReceiver);
  m_core->init();

  ::libretro::Game game(m_contentPath, m_gameData);
  m_core->loadGame(&game);

  if (m_saveData.size() > 0) {
    m_core->writeMemoryData(
        ::libretro::SAVE_RAM,
        std::vector<char>(m_saveData.begin(), m_saveData.end()));
  }

  m_initialized = true;
  return true;
}
bool EmulatorInstance::isInitialized() { return m_initialized; }
std::string EmulatorInstance::getContentHash() const { return m_contentHash; }
int EmulatorInstance::getPlatformId() const { return m_platformId; }
void EmulatorInstance::runFrame() { m_core->run(0); }
void EmulatorInstance::reset() { m_core->reset(); }

std::future<bool> EmulatorInstance::save() {
  return std::async(std::launch::async, [this]() -> bool {
    saves::Savefile saveData(m_core->getMemoryData(::libretro::SAVE_RAM));
    // if (!m_currentImage.isNull() && m_currentImage.width() > 0 &&
    //     m_currentImage.height() > 0) {
    //   saveData.setImage(m_currentImage.copy());
    // }

    QFuture<bool> result = getSaveManager()->writeSaveData(
        m_contentHash.data(), m_saveSlotNumber, saveData);

    return true;
  });
}
std::vector<uint8_t> EmulatorInstance::serializeState() {
  return m_core->serializeState();
}
void EmulatorInstance::deserializeState(const std::vector<uint8_t> &state) {
  m_core->deserializeState(state);
}
::libretro::Core *EmulatorInstance::getCore() { return m_core.get(); }
} // namespace firelight::emulation