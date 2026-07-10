#pragma once

#include <firelight/userdata_database.hpp>

#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace firelight {

// In-memory IUserdataDatabase for tests. SaveManager persists metadata from a
// std::async thread; the real SqliteUserdataDatabase uses a single
// thread-affine QSqlDatabase connection, so using it here would be cross-thread
// QSqlDatabase misuse (which corrupts Qt's global SQL state). This fake is
// thread-safe and keeps the metadata reachable (and lets tests assert
// create/update counts).
class FakeUserdataDatabase : public db::IUserdataDatabase {
public:
  bool createSavefileMetadata(db::SavefileMetadata &metadata) override {
    std::lock_guard lock(m_mutex);
    metadata.id = m_nextId++;
    m_savefiles[{metadata.contentId, static_cast<int>(metadata.slotNumber)}] =
        metadata;
    ++createCount;
    return true;
  }

  std::optional<db::SavefileMetadata>
  getSavefileMetadata(std::string contentId, int slotNumber) override {
    std::lock_guard lock(m_mutex);
    const auto it = m_savefiles.find({std::move(contentId), slotNumber});
    if (it == m_savefiles.end()) {
      return std::nullopt;
    }
    return it->second;
  }

  bool updateSavefileMetadata(db::SavefileMetadata metadata) override {
    std::lock_guard lock(m_mutex);
    m_savefiles[{metadata.contentId, static_cast<int>(metadata.slotNumber)}] =
        metadata;
    ++updateCount;
    return true;
  }

  std::vector<db::SavefileMetadata>
  getSavefileMetadataForContent(std::string) override {
    return {};
  }

  // --- unused by tests ---
  bool tableExists(std::string) override { return true; }
  bool createSuspendPointMetadata(db::SuspendPointMetadata &) override {
    return true;
  }
  std::optional<db::SuspendPointMetadata>
  getSuspendPointMetadata(std::string, int, int) override {
    return std::nullopt;
  }
  bool updateSuspendPointMetadata(const db::SuspendPointMetadata &) override {
    return true;
  }
  std::vector<db::SuspendPointMetadata>
  getSuspendPointMetadataForContent(std::string, int) override {
    return {};
  }
  bool deleteSuspendPointMetadata(int) override { return true; }
  std::optional<std::string> getPlatformSettingValue(int,
                                                     std::string) override {
    return std::nullopt;
  }
  std::map<std::string, std::string> getAllPlatformSettings(int) override {
    return {};
  }
  void setPlatformSettingValue(int, std::string, std::string) override {}

  bool createModInstallation(db::ModInstallation &installation) override {
    std::lock_guard lock(m_mutex);
    installation.id = m_nextModId++;
    m_modInstallations[installation.id] = installation;
    return true;
  }

  bool updateModInstallation(const db::ModInstallation &installation) override {
    std::lock_guard lock(m_mutex);
    const auto it = m_modInstallations.find(installation.id);
    if (it == m_modInstallations.end()) {
      return false;
    }
    it->second = installation;
    return true;
  }

  std::optional<db::ModInstallation> getModInstallation(int id) override {
    std::lock_guard lock(m_mutex);
    const auto it = m_modInstallations.find(id);
    if (it == m_modInstallations.end()) {
      return std::nullopt;
    }
    return it->second;
  }

  std::optional<db::ModInstallation>
  getModInstallationForEntry(int entryId) override {
    std::lock_guard lock(m_mutex);
    for (const auto &[id, installation] : m_modInstallations) {
      if (installation.entryId == entryId) {
        return installation;
      }
    }
    return std::nullopt;
  }

  std::vector<db::ModInstallation>
  getModInstallationsForBaseContentHash(std::string baseContentHash) override {
    std::lock_guard lock(m_mutex);
    std::vector<db::ModInstallation> result;
    for (const auto &[id, installation] : m_modInstallations) {
      if (installation.baseContentHash == baseContentHash) {
        result.push_back(installation);
      }
    }
    return result;
  }

  std::vector<db::ModInstallation> getAllModInstallations() override {
    std::lock_guard lock(m_mutex);
    std::vector<db::ModInstallation> result;
    result.reserve(m_modInstallations.size());
    for (const auto &[id, installation] : m_modInstallations) {
      result.push_back(installation);
    }
    return result;
  }

  bool deleteModInstallation(int id) override {
    std::lock_guard lock(m_mutex);
    return m_modInstallations.erase(id) > 0;
  }

  int createCount = 0;
  int updateCount = 0;

private:
  std::mutex m_mutex;
  int m_nextId = 1;
  int m_nextModId = 1;
  std::map<std::pair<std::string, int>, db::SavefileMetadata> m_savefiles;
  std::map<int, db::ModInstallation> m_modInstallations;
};

} // namespace firelight
