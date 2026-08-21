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
    m_savefiles[{metadata.contentHash, static_cast<int>(metadata.saveSlot)}] = metadata;
    ++createCount;
    return true;
  }

  std::optional<SavefileMetadata> getSavefileMetadata(std::string contentHash, int saveSlot) override {
    std::lock_guard lock(m_mutex);
    const auto it = m_savefiles.find({std::move(contentHash), saveSlot});
    if (it == m_savefiles.end()) {
      return std::nullopt;
    }
    return it->second;
  }

  bool updateSavefileMetadata(SavefileMetadata metadata) override {
    std::lock_guard lock(m_mutex);
    m_savefiles[{metadata.contentHash, static_cast<int>(metadata.saveSlot)}] = metadata;
    ++updateCount;
    return true;
  }

  std::vector<SavefileMetadata> getSavefileMetadataForContent(std::string) override { return {}; }

  // --- suspend points: minimal in-memory behavior ---
  bool createSuspendPointMetadata(SuspendPointMetadata &metadata) override {
    std::lock_guard lock(m_mutex);
    metadata.id = m_nextId++;
    m_suspends[metadata.id] = metadata;
    return true;
  }

  std::optional<SuspendPointMetadata> getSuspendPointMetadata(std::string contentHash, int saveSlot,
                                                              int pointIndex) override {
    std::lock_guard lock(m_mutex);
    for (const auto &[id, m] : m_suspends) {
      if (m.contentHash == contentHash && m.saveSlot == saveSlot && static_cast<int>(m.pointIndex) == pointIndex) {
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

  std::vector<SuspendPointMetadata> getSuspendPointMetadataForContent(std::string contentHash, int saveSlot) override {
    std::lock_guard lock(m_mutex);
    std::vector<SuspendPointMetadata> out;
    for (const auto &[id, m] : m_suspends) {
      if (m.contentHash == contentHash && m.saveSlot == saveSlot) {
        out.push_back(m);
      }
    }
    return out;
  }

  bool deleteSuspendPointMetadata(int id) override {
    std::lock_guard lock(m_mutex);
    return m_suspends.erase(id) > 0;
  }

  bool transferContent(const std::string &fromContentHash, const std::string &toContentHash) override {
    std::lock_guard lock(m_mutex);

    std::map<std::pair<std::string, int>, SavefileMetadata> movedSavefiles;
    for (auto &[key, metadata] : m_savefiles) {
      if (key.first != fromContentHash) {
        movedSavefiles[key] = metadata;
        continue;
      }

      // A slot already held at the destination wins, matching the real one
      const auto destination = std::pair{toContentHash, key.second};
      if (m_savefiles.count(destination) > 0) {
        movedSavefiles[key] = metadata;
        continue;
      }

      metadata.contentHash = toContentHash;
      movedSavefiles[destination] = metadata;
    }
    m_savefiles = movedSavefiles;

    for (auto &[id, metadata] : m_suspends) {
      if (metadata.contentHash == fromContentHash) {
        metadata.contentHash = toContentHash;
      }
    }

    return true;
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
