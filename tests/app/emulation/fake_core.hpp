#pragma once

#include <firelight/libretro/configuration_provider.hpp>
#include <firelight/libretro/icore.hpp>

#include <cstring>
#include <memory>
#include <vector>

namespace firelight::emulation {

// A minimal in-memory ICore for tests: no DLL is opened, so there is no
// process-exit on teardown and no real-hardware dependency. It models just
// enough to exercise EmulatorInstance's flow: a SAVE_RAM buffer and a frame
// counter that run() advances, both captured/restored by serialize/deserialize.
class FakeCore : public ::libretro::ICore {
public:
  static constexpr std::size_t SRAM_SIZE = 32;

  // --- wiring (record nothing; the fake ignores real receivers) ---
  void setVideoReceiver(firelight::libretro::IVideoDataReceiver *) override {}
  void setAudioReceiver(std::shared_ptr<IAudioDataReceiver>) override {}
  void setRetropadProvider(firelight::libretro::IRetropadProvider *) override {}
  void
  setPointerInputProvider(firelight::libretro::IPointerInputProvider *) override {
  }
  void setSystemDirectory(const std::string &) override {}
  void setSaveDirectory(const std::string &dir) override { m_saveDir = dir; }

  // When set, the fake declares a couple of core options on init(), exercising
  // the frontend's option-capture/persistence path.
  void setConfigProvider(
      std::shared_ptr<firelight::libretro::IConfigurationProvider> provider) {
    m_configProvider = std::move(provider);
  }

  // --- lifecycle ---
  void init() override {
    m_inited = true;
    if (m_configProvider) {
      firelight::libretro::IConfigurationProvider::Option a;
      a.key = "fake_opt_a";
      a.label = "Fake option A";
      a.description = "First fake option";
      a.defaultValueKey = "on";
      a.possibleValues = {{.key = "on", .label = "On"},
                          {.key = "off", .label = "Off"}};
      m_configProvider->registerOption(a);

      firelight::libretro::IConfigurationProvider::Option b;
      b.key = "fake_opt_b";
      b.label = "Fake option B";
      b.description = "Second fake option";
      b.defaultValueKey = "1";
      b.possibleValues = {{.key = "1", .label = "One"}};
      m_configProvider->registerOption(b);
    }
  }
  bool loadGame(::libretro::Game *) override {
    m_gameLoaded = true;
    if (m_sram.empty()) {
      m_sram.assign(SRAM_SIZE, 0); // simulate a game with battery-backed SRAM
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
  [[nodiscard]] std::size_t getSerializeSize() const override {
    return sizeof(int) + m_sram.size();
  }
  bool deserializeState(const std::vector<uint8_t> &data) const override {
    if (data.size() != getSerializeSize()) {
      return false;
    }
    std::memcpy(&m_frameCount, data.data(), sizeof(int));
    m_sram.assign(reinterpret_cast<const char *>(data.data()) + sizeof(int),
                  reinterpret_cast<const char *>(data.data()) + data.size());
    return true;
  }

  // --- memory access (SYSTEM_RAM is a test-configurable writable buffer so the
  //     cheat engine can be exercised; other regions are unmodeled) ---
  void setSystemRamSize(std::size_t bytes) { m_systemRam.assign(bytes, 0); }
  [[nodiscard]] const std::vector<uint8_t> &systemRam() const {
    return m_systemRam;
  }
  [[nodiscard]] void *getMemoryData(unsigned id) const override {
    if (id == ::libretro::SYSTEM_RAM && !m_systemRam.empty()) {
      return const_cast<uint8_t *>(m_systemRam.data());
    }
    return nullptr;
  }
  [[nodiscard]] std::size_t getMemorySize(unsigned id) const override {
    return id == ::libretro::SYSTEM_RAM ? m_systemRam.size() : 0;
  }
  retro_memory_map *getMemoryMap() override { return nullptr; }

  // --- disc control (test-configurable; 0 discs = single-disc content) ---
  void setDiscCount(const unsigned count) { m_discCount = count; }
  [[nodiscard]] unsigned getDiskCount() const override { return m_discCount; }
  [[nodiscard]] unsigned getCurrentDiskIndex() const override {
    return m_discIndex;
  }
  bool setDiskIndex(const unsigned index) override {
    if (index >= m_discCount) {
      return false;
    }
    m_discIndex = index;
    return true;
  }

  // --- controller port devices (test-configurable) ---
  void setControllerDevices(
      std::vector<std::vector<ControllerDeviceOption>> devices) {
    m_controllerDevices = std::move(devices);
  }
  [[nodiscard]] std::vector<std::vector<ControllerDeviceOption>>
  getControllerDevices() const override {
    return m_controllerDevices;
  }
  void setControllerPortDevice(unsigned port, unsigned device) override {
    m_portDeviceCalls.emplace_back(port, device);
  }

  // --- cheats (test-configurable) ---
  struct CheatCall {
    unsigned index;
    bool enabled;
    std::string code;
  };
  void setCheat(unsigned index, bool enabled,
                const std::string &code) override {
    m_cheatCalls.push_back({index, enabled, code});
  }
  void clearCheats() override { ++m_cheatsCleared; }
  [[nodiscard]] const std::vector<CheatCall> &cheatCalls() const {
    return m_cheatCalls;
  }
  [[nodiscard]] int cheatsClearedCount() const { return m_cheatsCleared; }

  // --- test probes ---
  [[nodiscard]] int frameCount() const { return m_frameCount; }
  [[nodiscard]] const std::vector<char> &sram() const { return m_sram; }
  [[nodiscard]] bool initialized() const { return m_inited; }
  [[nodiscard]] const std::string &savedSaveDirectory() const {
    return m_saveDir;
  }
  [[nodiscard]] const std::vector<std::pair<unsigned, unsigned>> &
  portDeviceCalls() const {
    return m_portDeviceCalls;
  }

private:
  // Mutable because ICore::deserializeState is const (matching the real Core's
  // signature) yet must restore emulator state.
  mutable int m_frameCount = 0;
  mutable std::vector<char> m_sram;
  bool m_inited = false;
  bool m_gameLoaded = false;
  unsigned m_discCount = 0;
  unsigned m_discIndex = 0;
  std::string m_saveDir;
  std::vector<std::vector<ControllerDeviceOption>> m_controllerDevices;
  std::vector<std::pair<unsigned, unsigned>> m_portDeviceCalls;
  std::vector<CheatCall> m_cheatCalls;
  int m_cheatsCleared = 0;
  std::vector<uint8_t> m_systemRam;
  std::shared_ptr<firelight::libretro::IConfigurationProvider> m_configProvider;
};

} // namespace firelight::emulation
