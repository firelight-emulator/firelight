#include <firelight/library/patch_file.hpp>

#include <firelight/library/file_bytes.hpp>
#include <patching/rom_patch.hpp>
#include <patching/bps_patch.hpp>
#include <patching/ips_patch.hpp>
#include <patching/ups_patch.hpp>

namespace firelight::library {

PatchFile::~PatchFile() = default;

bool PatchFile::load() {
  if (m_filePath.empty()) {
    return false;
  }

  m_patchData = readAllBytes(m_filePath);
  if (m_patchData.empty()) {
    return false;
  }

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

std::vector<uint8_t> PatchFile::patch(const std::vector<uint8_t> &data) const {
  if (!m_patch) {
    return {};
  }

  return m_patch->patchRom(data);
}
} // namespace firelight::library
