#include <firelight/saves/savefile.hpp>

#include <utility>

namespace firelight::saves {

Savefile::Savefile(const std::vector<char> &saveRamData) {
  m_saveRamData = std::vector(saveRamData.begin(), saveRamData.end());
}

Savefile::Savefile(const Savefile &other) {
  m_saveRamData = std::vector(other.m_saveRamData.begin(), other.m_saveRamData.end());
}

std::vector<char> Savefile::getSaveRamData() const { return m_saveRamData; }

} // namespace firelight::saves
