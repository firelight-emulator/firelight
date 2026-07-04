#include "cheats/cheat_engine.hpp"

#include <firelight/libretro/icore.hpp>

#include <cstdint>
#include <utility>

namespace firelight::cheats {

void CheatEngine::setActivePokes(std::vector<CheatPoke> pokes) {
  m_pokes = std::move(pokes);
}

void CheatEngine::clear() { m_pokes.clear(); }

void CheatEngine::apply(::libretro::ICore &core) const {
  if (m_pokes.empty()) {
    return;
  }
  // The unsigned-id overload returns the live writable RAM pointer (the same one
  // rcheevos reads); the MemoryType overload would copy save data instead.
  const auto ramId = static_cast<unsigned>(::libretro::SYSTEM_RAM);
  auto *ram = static_cast<uint8_t *>(core.getMemoryData(ramId));
  const std::size_t size = core.getMemorySize(ramId);
  if (ram == nullptr || size == 0) {
    return;
  }

  for (const auto &poke : m_pokes) {
    const std::size_t end =
        static_cast<std::size_t>(poke.address) + poke.size;
    if (end > size) {
      continue; // out of range for this core's RAM
    }
    for (uint8_t i = 0; i < poke.size; ++i) {
      const uint8_t byteIndex = poke.bigEndian ? (poke.size - 1 - i) : i;
      ram[poke.address + i] =
          static_cast<uint8_t>((poke.value >> (byteIndex * 8)) & 0xFFu);
    }
  }
}

} // namespace firelight::cheats
