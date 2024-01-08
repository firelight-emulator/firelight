//
// Created by alexs on 1/7/2024.
//

#include "save_data.hpp"

namespace Firelight::Saves {

SaveData::SaveData(const std::vector<char> &saveRamData) {
  m_saveRamData = std::vector(saveRamData.begin(), saveRamData.end());

  printf("hey");
}

std::vector<char> SaveData::getSaveRamData() const { return m_saveRamData; }
} // namespace Firelight::Saves