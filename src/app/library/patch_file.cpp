#include "patch_file.hpp"

#include "../patching/rom_patch.hpp"

#include <fstream>

namespace firelight::library {
bool PatchFile::load() {
  if (m_filePath.empty()) {
    return false;
  }

  std::ifstream file(m_filePath, std::ios::binary);
  if (!file) {
    return false;
  }

  m_patchData = std::vector<uint8_t>(std::istreambuf_iterator(file), {});

  file.close();

  auto suffix = m_filePath.substr(m_filePath.find_last_of('.') + 1);
  if (suffix == "ips") {
    m_patchType = IPS;
    m_patch = std::make_shared<patching::IPSPatch>(m_patchData);
  } else if (suffix == "bps") {
    m_patchType = BPS;
    m_patch = std::make_shared<patching::BPSPatch>(m_patchData);
  } else if (suffix == "ups") {
    m_patchType = UPS;
    m_patch = std::make_shared<patching::UPSPatch>(m_patchData);
  } else {
    m_patchType = UNKNOWN;
  }

  return true;
}

PatchFile::PatchType PatchFile::getType() const { return m_patchType; }
bool PatchFile::isValid() {}

std::vector<uint8_t> PatchFile::patch(const std::vector<uint8_t> &data) const {
  if (!m_patch) {
    return {};
  }

  return m_patch->patchRom(data);
}
} // namespace firelight::library