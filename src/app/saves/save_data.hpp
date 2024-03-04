#pragma once

#include <qimage.h>
#include <vector>

namespace firelight::Saves {

class SaveData {
public:
  explicit SaveData(const std::vector<char> &saveRamData);
  SaveData(const SaveData &other);
  [[nodiscard]] std::vector<char> getSaveRamData() const;
  void setImage(QImage image);
  [[nodiscard]] QImage getImage() const;

private:
  std::string m_contentMd5{};
  std::vector<char> m_saveRamData{};
  QImage m_screenshot{};
};

} // namespace firelight::Saves
