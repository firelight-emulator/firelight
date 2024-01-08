//
// Created by alexs on 1/7/2024.
//

#include "save_data.hpp"

namespace Firelight::Saves {

SaveData::SaveData(const std::vector<std::byte> &saveRamData)
    : m_saveRamData(saveRamData) {}

std::vector<std::byte> SaveData::getSaveRamData() const {
  return m_saveRamData;
}
} // namespace Firelight::Saves