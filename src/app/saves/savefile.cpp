//
// Created by alexs on 1/7/2024.
//

#include "savefile.hpp"

#include <utility>

namespace firelight::saves {

Savefile::Savefile(const std::vector<char> &saveRamData) {
  m_saveRamData = std::vector(saveRamData.begin(), saveRamData.end());
}
Savefile::Savefile(const Savefile &other) {
  m_saveRamData =
      std::vector(other.m_saveRamData.begin(), other.m_saveRamData.end());
  m_screenshot = QImage(other.m_screenshot);
}

std::vector<char> Savefile::getSaveRamData() const { return m_saveRamData; }
void Savefile::setImage(QImage image) { m_screenshot = std::move(image); }
QImage Savefile::getImage() const { return m_screenshot; }
} // namespace firelight::saves