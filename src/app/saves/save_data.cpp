//
// Created by alexs on 1/7/2024.
//

#include "save_data.hpp"

#include <utility>

namespace firelight::Saves {

SaveData::SaveData(const std::vector<char> &saveRamData) {
  m_saveRamData = std::vector(saveRamData.begin(), saveRamData.end());
}
SaveData::SaveData(const SaveData &other) {
  m_saveRamData =
      std::vector(other.m_saveRamData.begin(), other.m_saveRamData.end());
  m_screenshot = QImage(other.m_screenshot);
}

std::vector<char> SaveData::getSaveRamData() const { return m_saveRamData; }
void SaveData::setImage(QImage image) { m_screenshot = std::move(image); }
QImage SaveData::getImage() const { return m_screenshot; }
} // namespace Firelight::Saves