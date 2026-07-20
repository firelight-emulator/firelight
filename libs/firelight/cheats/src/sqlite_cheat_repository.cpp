#include <firelight/cheats/sqlite_cheat_repository.hpp>
#include <firelight/migrations/migration_runner.hpp>

#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Statement.h>
#include <SQLiteCpp/Transaction.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <utility>

namespace firelight::cheats {

namespace {
std::string pokesToJson(const std::vector<CheatPoke> &pokes) {
  nlohmann::json arr = nlohmann::json::array();
  for (const auto &p : pokes) {
    arr.push_back({{"a", p.address}, {"v", p.value}, {"s", p.size}, {"be", p.bigEndian}});
  }
  return arr.dump();
}

std::vector<CheatPoke> pokesFromJson(const std::string &s) {
  std::vector<CheatPoke> pokes;
  const auto j = nlohmann::json::parse(s, nullptr, /*allow_exceptions=*/false);
  if (j.is_array()) {
    for (const auto &e : j) {
      CheatPoke p;
      p.address = e.value("a", 0u);
      p.value = e.value("v", 0u);
      p.size = static_cast<uint8_t>(e.value("s", 1));
      p.bigEndian = e.value("be", false);
      pokes.push_back(p);
    }
  }
  return pokes;
}
} // namespace

SqliteCheatRepository::SqliteCheatRepository(std::string databaseFile) : m_databaseFile(std::move(databaseFile)) {
  m_database = std::make_unique<SQLite::Database>(m_databaseFile, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

  // Forward-only schema migrations (see migration_runner). A future change adds
  // the next-numbered migration
  const std::vector<migrations::Migration> schema = {
      {1,
       [this] {
         m_database->exec(R"(
           CREATE TABLE IF NOT EXISTS cheats (
               id INTEGER PRIMARY KEY AUTOINCREMENT,
               content_hash TEXT NOT NULL,
               name TEXT NOT NULL,
               type INTEGER NOT NULL DEFAULT 0,
               raw_code TEXT NOT NULL DEFAULT '',
               pokes_json TEXT NOT NULL DEFAULT '[]',
               enabled INTEGER NOT NULL DEFAULT 0,
               affects_hardcore INTEGER NOT NULL DEFAULT 1,
               ordinal INTEGER NOT NULL DEFAULT 0
           );
         )");
         m_database->exec("CREATE INDEX IF NOT EXISTS idx_cheats_hash ON cheats(content_hash)");
       }},
  };

  try {
    SQLite::Transaction transaction(*m_database);
    const int currentVersion = m_database->execAndGet("PRAGMA user_version").getInt();
    migrations::applyMigrations(currentVersion, schema, [this](const int v) {
      m_database->exec("PRAGMA user_version = " + std::to_string(v));
    });
    transaction.commit();
  } catch (const std::exception &e) {
    spdlog::error("Failed to initialize cheat store: {}", e.what());
  }
}

SqliteCheatRepository::~SqliteCheatRepository() = default;

std::vector<Cheat> SqliteCheatRepository::getCheats(const std::string &contentHash) {
  std::vector<Cheat> cheats;
  try {
    SQLite::Statement query(*m_database, "SELECT id, content_hash, name, type, raw_code, pokes_json, enabled, "
                                         "affects_hardcore, ordinal FROM cheats WHERE content_hash = :hash "
                                         "ORDER BY ordinal, id");
    query.bind(":hash", contentHash);
    while (query.executeStep()) {
      Cheat cheat;
      cheat.id = query.getColumn(0).getInt();
      cheat.contentHash = query.getColumn(1).getString();
      cheat.name = query.getColumn(2).getString();
      cheat.type = static_cast<CheatType>(query.getColumn(3).getInt());
      cheat.rawCode = query.getColumn(4).getString();
      cheat.pokes = pokesFromJson(query.getColumn(5).getString());
      cheat.enabled = query.getColumn(6).getInt() != 0;
      cheat.affectsHardcore = query.getColumn(7).getInt() != 0;
      cheat.ordinal = query.getColumn(8).getInt();
      cheats.push_back(std::move(cheat));
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to read cheats for {}: {}", contentHash, e.what());
  }
  return cheats;
}

bool SqliteCheatRepository::addCheat(Cheat &cheat) {
  try {
    int nextOrdinal = 0;
    {
      SQLite::Statement q(*m_database, "SELECT COALESCE(MAX(ordinal) + 1, 0) FROM cheats "
                                       "WHERE content_hash = :hash");
      q.bind(":hash", cheat.contentHash);
      if (q.executeStep()) {
        nextOrdinal = q.getColumn(0).getInt();
      }
    }

    SQLite::Statement insert(*m_database, "INSERT INTO cheats (content_hash, name, type, raw_code, pokes_json, "
                                          "enabled, affects_hardcore, ordinal) VALUES (:hash, :name, :type, "
                                          ":raw, :pokes, :enabled, :hardcore, :ordinal)");
    insert.bind(":hash", cheat.contentHash);
    insert.bind(":name", cheat.name);
    insert.bind(":type", static_cast<int>(cheat.type));
    insert.bind(":raw", cheat.rawCode);
    insert.bind(":pokes", pokesToJson(cheat.pokes));
    insert.bind(":enabled", cheat.enabled ? 1 : 0);
    insert.bind(":hardcore", cheat.affectsHardcore ? 1 : 0);
    insert.bind(":ordinal", nextOrdinal);
    insert.exec();

    cheat.id = static_cast<int>(m_database->getLastInsertRowid());
    cheat.ordinal = nextOrdinal;
    return true;
  } catch (const std::exception &e) {
    spdlog::error("Failed to add cheat '{}': {}", cheat.name, e.what());
    return false;
  }
}

bool SqliteCheatRepository::updateCheat(const Cheat &cheat) {
  try {
    SQLite::Statement update(*m_database, "UPDATE cheats SET name = :name, type = :type, raw_code = :raw, "
                                          "pokes_json = :pokes, enabled = :enabled, affects_hardcore = :hardcore, "
                                          "ordinal = :ordinal WHERE id = :id");
    update.bind(":name", cheat.name);
    update.bind(":type", static_cast<int>(cheat.type));
    update.bind(":raw", cheat.rawCode);
    update.bind(":pokes", pokesToJson(cheat.pokes));
    update.bind(":enabled", cheat.enabled ? 1 : 0);
    update.bind(":hardcore", cheat.affectsHardcore ? 1 : 0);
    update.bind(":ordinal", cheat.ordinal);
    update.bind(":id", cheat.id);
    return update.exec() > 0;
  } catch (const std::exception &e) {
    spdlog::error("Failed to update cheat {}: {}", cheat.id, e.what());
    return false;
  }
}

bool SqliteCheatRepository::setEnabled(int id, bool enabled) {
  try {
    SQLite::Statement update(*m_database, "UPDATE cheats SET enabled = :enabled WHERE id = :id");
    update.bind(":enabled", enabled ? 1 : 0);
    update.bind(":id", id);
    return update.exec() > 0;
  } catch (const std::exception &e) {
    spdlog::error("Failed to toggle cheat {}: {}", id, e.what());
    return false;
  }
}

bool SqliteCheatRepository::removeCheat(int id) {
  try {
    SQLite::Statement del(*m_database, "DELETE FROM cheats WHERE id = :id");
    del.bind(":id", id);
    return del.exec() > 0;
  } catch (const std::exception &e) {
    spdlog::error("Failed to remove cheat {}: {}", id, e.what());
    return false;
  }
}

} // namespace firelight::cheats
