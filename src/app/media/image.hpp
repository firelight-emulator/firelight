#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace firelight::media {
class Image {
public:
  Image() = default;
  Image(const uint8_t *data, int width, int height, int numChannels);

  void flipVertical();
  bool saveToFile(std::string filename);
  bool isEmpty() const;

private:
  int m_width = 0;
  int m_height = 0;
  int m_numChannels = 0;
  size_t m_size = 0;
  std::vector<uint8_t> m_data;
};
} // namespace firelight::media