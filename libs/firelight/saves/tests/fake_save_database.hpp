#pragma once

#include <firelight/saves/save_database.hpp>

#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace firelight::saves {

// In-memory ISaveDatabase for tests. SaveManager persists metadata from a
// background thread, so this fake is thread-safe and lets tests assert
// create/update counts
class FakeSaveDatabase : public ISaveDatabase {
public:
  bool createSavefileMetadata(SavefileMetadata &metadata) override {
    std::lock_guard lock(m_mutex);
    metadata.id = m_nextId++;
    m_savefiles[{metadata.contentId, static_cast<int>(metadata.slotNumber)}] =
        metadata;
    ++createCount;
    return true;
  }

  std::optional<SavefileMetadata>
  getSavefileMetadata(std::string contentId, int slotNumber) override {
    std::lock_guard lock(m_mutex);
    const auto it = m_savefiles.find({std::move(contentId), slotNumber});
    if (it == m_savefiles.end()) {
      return std::nullopt;
    }
    return it->second;
  }

  bool updateSavefileMetadata(SavefileMetadata metadata) override {
    std::lock_guard lock(m_mutex);
    m_savefiles[{metadata.contentId, static_cast<int>(metadata.slotNumber)}] =
        metadata;
    ++updateCount;
    return true;
  }

  std::vector<SavefileMetadata>
  getSavefileMetadataForContent(std::string) override {
    return {};
  }

  // --- suspend points: minimal in-memory behavior ---
  bool createSuspendPointMetadata(SuspendPointMetadata &metadata) override {
    std::lock_guard lock(m_mutex);
    metadata.id = m_nextId++;
    m_suspends[metadata.id] = metadata;
    return true;
  }
  std::optional<SuspendPointMetadata>
  getSuspendPointMetadata(std::string contentId, int saveSlotNumber,
                          int slotNumber) override {
    std::lock_guard lock(m_mutex);
    for (const auto &[id, m] : m_suspends) {
      if (m.contentId == contentId && m.saveSlotNumber == saveSlotNumber &&
          static_cast<int>(m.slotNumber) == slotNumber) {
        return m;
      }
    }
    return std::nullopt;
  }
  bool updateSuspendPointMetadata(const SuspendPointMetadata &metadata) override {
    std::lock_guard lock(m_mutex);
    m_suspends[metadata.id] = metadata;
    return true;
  }
  std::vector<SuspendPointMetadata>
  getSuspendPointMetadataForContent(std::string contentId,
                                    int saveSlotNumber) override {
    std::lock_guard lock(m_mutex);
    std::vector<SuspendPointMetadata> out;
    for (const auto &[id, m] : m_suspends) {
      if (m.contentId == contentId && m.saveSlotNumber == saveSlotNumber) {
        out.push_back(m);
      }
    }
    return out;
  }
  bool deleteSuspendPointMetadata(int id) override {
    std::lock_guard lock(m_mutex);
    return m_suspends.erase(id) > 0;
  }

  int createCount = 0;
  int updateCount = 0;

private:
  std::mutex m_mutex;
  int m_nextId = 1;
  std::map<std::pair<std::string, int>, SavefileMetadata> m_savefiles;
  std::map<int, SuspendPointMetadata> m_suspends;
};

} // namespace firelight::saves
