#pragma once

#include <string>
#include <vector>

namespace firelight::saves {

class Savefile {
public:
  explicit Savefile(const std::vector<char> &saveRamData);
  Savefile(const Savefile &other);
  [[nodiscard]] std::vector<char> getSaveRamData() const;

private:
  std::string m_contentId{};
  std::vector<char> m_saveRamData{};
};

} // namespace firelight::saves
