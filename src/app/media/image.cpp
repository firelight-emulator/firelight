#include "image.hpp"

#include <spdlog/spdlog.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <string>

namespace firelight::media {
Image::Image(const uint8_t *data, const int width, const int height,
             const int numChannels)
    : m_width(width), m_height(height), m_numChannels(numChannels) {
  m_size = m_height * m_width * m_numChannels;

  m_data.assign(data, data + m_size);
}

void Image::flipVertical() {
  const size_t bytesPerRow = static_cast<size_t>(m_width) * m_numChannels;

  std::vector<uint8_t> tempRow(bytesPerRow);

  for (int i = 0; i < m_height / 2; ++i) {
    uint8_t *topRow = &m_data[i * bytesPerRow];
    uint8_t *bottomRow = &m_data[(m_height - 1 - i) * bytesPerRow];

    // Top to temp, bottom to top, temp to bottom
    memcpy(tempRow.data(), topRow, bytesPerRow);
    memcpy(topRow, bottomRow, bytesPerRow);
    memcpy(bottomRow, tempRow.data(), bytesPerRow);
  }
}

bool Image::saveToFile(std::string filename) {
  spdlog::info("Writing image to file: {}", filename);
  stbi_write_png(filename.c_str(), m_width, m_height, m_numChannels,
                 m_data.data(), m_width * m_numChannels);
  return false;
}
bool Image::isEmpty() const { return false; }
} // namespace firelight::media