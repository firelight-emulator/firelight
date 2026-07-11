#pragma once

#include <firelight/saves/save_database.hpp>

#include <memory>
#include <mutex>
#include <string>

namespace SQLite {
class Database;
}

namespace firelight::saves {
// SQLiteCpp-backed save/suspend metadata index. Qt-free. A single connection is
// guarded by a mutex so it can be shared across threads (matching the pattern in
// SqliteGameCaptureRepository). The Phase 2 save worker will hold its own
// instance on its own thread.
class SqliteSaveDatabase final : public ISaveDatabase {
public:
  explicit SqliteSaveDatabase(const std::string &dbFile);
  ~SqliteSaveDatabase() override;

  bool createSavefileMetadata(SavefileMetadata &metadata) override;

  std::optional<SavefileMetadata> getSavefileMetadata(std::string contentId,
                                                      int slotNumber) override;

  bool updateSavefileMetadata(SavefileMetadata metadata) override;

  std::vector<SavefileMetadata>
  getSavefileMetadataForContent(std::string contentId) override;

  bool createSuspendPointMetadata(SuspendPointMetadata &metadata) override;

  std::optional<SuspendPointMetadata>
  getSuspendPointMetadata(std::string contentId, int saveSlotNumber,
                          int slotNumber) override;

  bool updateSuspendPointMetadata(const SuspendPointMetadata &metadata) override;

  std::vector<SuspendPointMetadata>
  getSuspendPointMetadataForContent(std::string contentId,
                                    int saveSlotNumber) override;

  bool deleteSuspendPointMetadata(int id) override;

private:
  std::string m_databaseFile;
  std::unique_ptr<SQLite::Database> m_db;
  std::mutex m_mutex;
};
} // namespace firelight::saves
