//
// Created by alexs on 1/7/2024.
//

#include "save_data.hpp"

namespace Firelight::Saves {

std::vector<std::byte> SaveData::getSaveRamData() const {
  return m_saveRamData;
}
} // namespace Firelight::Saves