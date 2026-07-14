#pragma once

#include "emulator_instance.hpp"

#include <firelight/tas/tas_emulator.hpp>

#include <cstdint>
#include <vector>

namespace firelight::emulation {

// Adapts the app's EmulatorInstance to the Qt-free tas::ITasEmulator surface a
// TasSession drives, so the resident harness runs on the real emulation pipeline.
// A thin forwarder — the session owns all TAS policy (cursor, greenzone, provider);
// this only exposes the four primitives it needs. The caller is responsible for
// putting the instance in TAS mode (setTasMode(true)) and running on the render
// thread, matching EmulatorInstance's confinement.
class EmulatorInstanceTasEmulator final : public tas::ITasEmulator {
public:
  explicit EmulatorInstanceTasEmulator(EmulatorInstance &instance)
      : m_instance(instance) {}

  void setRetropadProvider(libretro::IRetropadProvider *provider) override {
    m_instance.setRetropadProvider(provider);
  }
  void runFrame() override { m_instance.runFrame(); }
  [[nodiscard]] std::vector<uint8_t> serializeState() override {
    return m_instance.serializeState();
  }
  bool deserializeState(const std::vector<uint8_t> &state) override {
    return m_instance.deserializeState(state);
  }

private:
  EmulatorInstance &m_instance;
};

} // namespace firelight::emulation
