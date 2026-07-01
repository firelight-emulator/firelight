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

  int createCount = 0;
  int updateCount = 0;

private:
  std::mutex m_mutex;
  int m_nextId = 1;
  std::map<std::pair<std::string, int>, db::SavefileMetadata> m_savefiles;
};

} // namespace firelight
