#pragma once

#include <qimage.h>
#include <vector>

namespace firelight::saves {

class Savefile {
public:
  explicit Savefile(const std::vector<char> &saveRamData);
  Savefile(const Savefile &other);
  [[nodiscard]] std::vector<char> getSaveRamData() const;
  void setImage(QImage image);
  [[nodiscard]] QImage getImage() const;

private:
  std::string m_contentMd5{};
  std::vector<char> m_saveRamData{};
  QImage m_screenshot{};
};

} // namespace firelight::Saves
