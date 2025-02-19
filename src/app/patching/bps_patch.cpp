#include "bps_patch.hpp"
#include "util.hpp"
#include <filesystem>
#include <fstream>
#include <spdlog/spdlog.h>
#include <zlib.h>

namespace firelight::patching {

constexpr int SOURCE_READ_ACTION = 0;
constexpr int TARGET_READ_ACTION = 1;
constexpr int SOURCE_COPY_ACTION = 2;
constexpr int TARGET_COPY_ACTION = 3;

BPSPatch::BPSPatch(const std::string &path) {
  const auto size = std::filesystem::file_size(path);

  std::ifstream file(path, std::ios::binary);

  std::vector<uint8_t> data(size);
  file.read(reinterpret_cast<char *>(data.data()), size);

  file.close();

  *this = BPSPatch(data);
}

BPSPatch::BPSPatch(const std::vector<uint8_t> &data) {
  size_t index = 4; // Skip BPS1 magic string

  m_inputFileSize = readVLV(data, index);
  m_outputFileSize = readVLV(data, index);

  const auto metadataSize = readVLV(data, index);

  m_metadata = std::string(data.begin() + index,
                           data.begin() + index +
                               metadataSize); // TODO: Check if this is correct

  index += metadataSize;

  while (index < data.size() - 12) {
    const auto value = readVLV(data, index);

    BPSPatchAction action{.type = value & 3, .length = (value >> 2) + 1};

    switch (action.type) {
    case SOURCE_READ_ACTION:
      // No additional data to read; intentionally empty.
      break;
    case TARGET_READ_ACTION:
      for (size_t i = 0; i < action.length; ++i) {
        action.bytes.push_back(data[index++]);
      }
      break;
    case SOURCE_COPY_ACTION:
    case TARGET_COPY_ACTION: {
      const auto offset = readVLV(data, index);
      action.relativeOffset = (offset & 1 ? -1 : 1) * (offset >> 1);
      break;
    }
    default:
      // TODO: Put actual error here + test
      spdlog::warn("Invalid action type: {}", action.type);
      m_isValid = false;
      return;
    }

    m_actions.emplace_back(action);
  }

  m_inputFileCRC32Checksum = data[index + 3] << 24 | data[index + 2] << 16 |
                             data[index + 1] << 8 | data[index];
  index += 4;
  m_outputFileCRC32Checksum = data[index + 3] << 24 | data[index + 2] << 16 |
                              data[index + 1] << 8 | data[index];
  index += 4;
  m_patchFileCRC32Checksum = data[index + 3] << 24 | data[index + 2] << 16 |
                             data[index + 1] << 8 | data[index];

  auto crc = crc32(0L, nullptr, 0);
  crc = crc32(crc, data.data(), data.size() - 4);
  if (crc != m_patchFileCRC32Checksum) {
    spdlog::warn("Patch data does not match expected checksum");
    m_isValid = false;
  }
}
std::vector<uint8_t>
BPSPatch::patchRom(const std::vector<uint8_t> &data) const {
  uint32_t crc = crc32(0L, nullptr, 0);
  crc = crc32(crc, data.data(), data.size());
  if (crc != m_inputFileCRC32Checksum) {
    throw std::runtime_error("Input data does not match expected checksum");
  }

  std::vector<uint8_t> patchedRom(m_outputFileSize);

  int64_t sourceRelativeOffset = 0;
  int64_t targetRelativeOffset = 0;

  auto outputOffset = 0;

  for (const auto &action : m_actions) {
    switch (action.type) {
    case SOURCE_READ_ACTION:
      for (size_t i = 0; i < action.length; ++i) {
        patchedRom[outputOffset] = data[outputOffset];
        outputOffset++;
      }
      break;
    case TARGET_READ_ACTION:
      for (size_t i = 0; i < action.length; ++i) {
        patchedRom[outputOffset++] = action.bytes[i];
      }
      break;
    case SOURCE_COPY_ACTION:
      sourceRelativeOffset += action.relativeOffset;
      for (size_t i = 0; i < action.length; ++i) {
        patchedRom[outputOffset++] = data[sourceRelativeOffset++];
      }
      break;
    case TARGET_COPY_ACTION:
      targetRelativeOffset += action.relativeOffset;
      for (size_t i = 0; i < action.length; ++i) {
        patchedRom[outputOffset++] = patchedRom[targetRelativeOffset++];
      }
      break;
    default:
      throw std::runtime_error("Invalid action type: " +
                               std::to_string(action.type));
    }
  }

  return patchedRom;
}

bool BPSPatch::isValid() const { return m_isValid; }

} // namespace firelight::patching