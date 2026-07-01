#pragma once

#include <firelight/libretro/icore.hpp>

#include <cstring>
#include <vector>

namespace firelight::emulation {

// A minimal in-memory ICore for tests: no DLL is opened, so there is no
// process-exit on teardown and no real-hardware dependency. It models just
// enough to exercise EmulatorInstance's flow: a SAVE_RAM buffer and a frame
// counter that run() advances, both captured/restored by serialize/deserialize.
class FakeCore : public ::libretro::ICore {
public:
  static constexpr std::size_t kSramSize = 32;

  // --- wiring (record nothing; the fake ignores real receivers) ---
  void setVideoReceiver(firelight::libretro::IVideoDataReceiver *) override {}
  void setAudioReceiver(std::shared_ptr<IAudioDataReceiver>) override {}
  void setRetropadProvider(firelight::libretro::IRetropadProvider *) override {}
  void
  setPointerInputProvider(firelight::libretro::IPointerInputProvider *) override {
  }
  void setSystemDirectory(const std::string &) override {}

  // --- lifecycle ---
  void init() override { m_inited = true; }
  bool loadGame(::libretro::Game *) override {
    m_gameLoaded = true;
    if (m_sram.empty()) {
      m_sram.assign(kSramSize, 0); // simulate a game with battery-backed SRAM
    }
    return true;
  }
  void run(double) override {
    ++m_frameCount;
    if (!m_sram.empty()) {
      m_sram[0] = static_cast<char>(m_frameCount & 0xFF); // observable mutation
    }
  }
  void reset() override {
    m_frameCount = 0;
    if (!m_sram.empty()) {
      m_sram[0] = 0;
    }
  }

  // --- save data / state ---
  void writeMemoryData(::libretro::MemoryType type,
                       const std::vector<char> &data) override {
    if (type == ::libretro::SAVE_RAM) {
      m_sram = data;
    }
  }
  [[nodiscard]] std::vector<char>
  getMemoryData(::libretro::MemoryType type) const override {
    return type == ::libretro::SAVE_RAM ? m_sram : std::vector<char>{};
  }
  [[nodiscard]] std::vector<uint8_t> serializeState() const override {
    std::vector<uint8_t> out(sizeof(int));
    std::memcpy(out.data(), &m_frameCount, sizeof(int));
    out.insert(out.end(), m_sram.begin(), m_sram.end());
    return out;
  }
  void deserializeState(const std::vector<uint8_t> &data) const override {
    if (data.size() < sizeof(int)) {
      return;
    }
    std::memcpy(&m_frameCount, data.data(), sizeof(int));
    m_sram.assign(reinterpret_cast<const char *>(data.data()) + sizeof(int),
                  reinterpret_cast<const char *>(data.data()) + data.size());
  }

  // --- RetroAchievements memory access (unused while achievements is null) ---
  [[nodiscard]] void *getMemoryData(unsigned) const override { return nullptr; }
  [[nodiscard]] std::size_t getMemorySize(unsigned) const override { return 0; }
  retro_memory_map *getMemoryMap() override { return nullptr; }

  // --- test probes ---
  [[nodiscard]] int frameCount() const { return m_frameCount; }
  [[nodiscard]] const std::vector<char> &sram() const { return m_sram; }
  [[nodiscard]] bool initialized() const { return m_inited; }

private:
  // Mutable because ICore::deserializeState is const (matching the real Core's
  // signature) yet must restore emulator state.
  mutable int m_frameCount = 0;
  mutable std::vector<char> m_sram;
  bool m_inited = false;
  bool m_gameLoaded = false;
};

} // namespace firelight::emulation
