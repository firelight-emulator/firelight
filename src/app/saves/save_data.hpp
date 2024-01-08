//
// Created by alexs on 1/7/2024.
//

#ifndef SAVE_DATA_HPP
#define SAVE_DATA_HPP
#include <qimage.h>
#include <vector>

namespace Firelight::Saves {

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

} // namespace Firelight::Saves

#endif // SAVE_DATA_HPP
