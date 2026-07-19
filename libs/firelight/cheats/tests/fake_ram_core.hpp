#pragma once

#include <firelight/libretro/icore.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace firelight::cheats {

// A minimal ICore for exercising CheatEngine in isolation: a test-configurable,
// writable SYSTEM_RAM buffer; every other ICore method is an inert stub. Keeps
// the cheats module test independent of the app's emulation FakeCore
class FakeRamCore final : public ::libretro::ICore {
public:
  void setSystemRamSize(std::size_t bytes) { m_ram.assign(bytes, 0); }

  [[nodiscard]] const std::vector<uint8_t> &systemRam() const { return m_ram; }

  [[nodiscard]] void *getMemoryData(unsigned id) const override {
    return (id == ::libretro::SYSTEM_RAM && !m_ram.empty()) ? const_cast<uint8_t *>(m_ram.data()) : nullptr;
  }

  [[nodiscard]] std::size_t getMemorySize(unsigned id) const override {
    return id == ::libretro::SYSTEM_RAM ? m_ram.size() : 0;
  }

  // --- inert stubs for the rest of ICore ---
  void setVideoReceiver(firelight::libretro::IVideoDataReceiver *) override {}

  void setAudioReceiver(std::shared_ptr<IAudioOutput>) override {}

  void setRetropadProvider(firelight::libretro::IRetropadProvider *) override {}

  void setPointerInputProvider(firelight::libretro::IPointerInputProvider *) override {}

  void setAudioInputProvider(firelight::libretro::IAudioInputProvider *) override {}

  void setSystemDirectory(const std::string &) override {}

  void setSaveDirectory(const std::string &) override {}

  void init() override {}

  void reset() override {}

  void run(double) override {}

  bool loadGame(::libretro::Game *) override { return true; }

  [[nodiscard]] std::vector<uint8_t> serializeState() const override { return {}; }

  [[nodiscard]] std::size_t getSerializeSize() const override { return 0; }

  bool deserializeState(const std::vector<uint8_t> &) const override { return true; }

  void writeMemoryData(::libretro::MemoryType, const std::vector<char> &) override {}

  [[nodiscard]] std::vector<char> getMemoryData(::libretro::MemoryType) const override { return {}; }

  retro_memory_map *getMemoryMap() override { return nullptr; }

  [[nodiscard]] unsigned getDiskCount() const override { return 0; }

  [[nodiscard]] unsigned getCurrentDiskIndex() const override { return 0; }

  bool setDiskIndex(unsigned) override { return false; }

  [[nodiscard]] std::vector<std::vector<ControllerDeviceOption>> getControllerDevices() const override { return {}; }

  void setControllerPortDevice(unsigned, unsigned) override {}

  void setCheat(unsigned, bool, const std::string &) override {}

  void clearCheats() override {}

  void sendKeyboardEvent(bool, unsigned, uint32_t, uint16_t) override {}

  [[nodiscard]] bool wantsKeyboard() const override { return false; }

private:
  std::vector<uint8_t> m_ram;
};

} // namespace firelight::cheats
