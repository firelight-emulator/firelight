#include "firelight/settings/sqlite_core_option_repository.hpp"

#include <SQLiteCpp/Transaction.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <utility>

namespace firelight::settings {

SqliteCoreOptionRepository::SqliteCoreOptionRepository(std::string databaseFile)
    : m_databaseFile(std::move(databaseFile)) {
  m_database = std::make_unique<SQLite::Database>(
      m_databaseFile, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

  m_database->exec(R"(
    CREATE TABLE IF NOT EXISTS core_options (
        core_name TEXT NOT NULL,
        key TEXT NOT NULL,
        label TEXT NOT NULL,
        description TEXT NOT NULL,
        default_value TEXT NOT NULL,
        values_json TEXT NOT NULL,
        category_key TEXT NOT NULL DEFAULT '',
        category_label TEXT NOT NULL DEFAULT '',
        position INTEGER NOT NULL,
        PRIMARY KEY (core_name, key)
    );
  )");

  // TODO
  // Migrate DBs created before category columns existed. CREATE TABLE IF NOT
  // EXISTS won't add columns to an existing table, so add them here; otherwise
  // reads/writes referencing them fail on older databases
  const auto hasColumn = [this](const char *name) {
    SQLite::Statement query(*m_database, "PRAGMA table_info(core_options)");
    while (query.executeStep()) {
      if (query.getColumn(1).getString() == name) {
        return true;
      }
    }
    return false;
  };
  if (!hasColumn("category_key")) {
    m_database->exec("ALTER TABLE core_options ADD COLUMN category_key TEXT NOT "
                     "NULL DEFAULT ''");
  }
  if (!hasColumn("category_label")) {
    m_database->exec("ALTER TABLE core_options ADD COLUMN category_label TEXT "
                     "NOT NULL DEFAULT ''");
  }
}

SqliteCoreOptionRepository::~SqliteCoreOptionRepository() = default;

void SqliteCoreOptionRepository::upsertCoreOptions(
    const std::string &coreName, const std::vector<CoreOption> &options) {
  try {
    // Replace-all: a core's declared option set is authoritative, so options it
    // no longer declares should disappear from the cache
    SQLite::Transaction transaction(*m_database);
    {
      SQLite::Statement del(*m_database,
                            "DELETE FROM core_options WHERE core_name = :core");
      del.bind(":core", coreName);
      del.exec();
    }

    SQLite::Statement insert(
        *m_database,
        "INSERT INTO core_options (core_name, key, label, description, "
        "default_value, values_json, category_key, category_label, position) "
        "VALUES (:core, :key, :label, :desc, :def, :values, :catKey, "
        ":catLabel, :pos)");
    int position = 0;
    for (const auto &option : options) {
      nlohmann::json values = nlohmann::json::array();
      for (const auto &v : option.values) {
        values.push_back({{"value", v.value}, {"label", v.label}});
      }
      insert.bind(":core", coreName);
      insert.bind(":key", option.key);
      insert.bind(":label", option.label);
      insert.bind(":desc", option.description);
      insert.bind(":def", option.defaultValue);
      insert.bind(":values", values.dump());
      insert.bind(":catKey", option.category);
      insert.bind(":catLabel", option.categoryLabel);
      insert.bind(":pos", position++);
      insert.exec();
      insert.reset();
    }

    transaction.commit();
  } catch (const std::exception &e) {
    spdlog::error("Failed to upsert core options for {}: {}", coreName,
                  e.what());
  }
}

std::vector<CoreOption>
SqliteCoreOptionRepository::getCoreOptions(const std::string &coreName) {
  std::vector<CoreOption> options;
  try {
    SQLite::Statement query(
        *m_database,
        "SELECT key, label, description, default_value, values_json, "
        "category_key, category_label FROM core_options WHERE core_name = :core "
        "ORDER BY position");
    query.bind(":core", coreName);
    while (query.executeStep()) {
      CoreOption option;
      option.key = query.getColumn(0).getString();
      option.label = query.getColumn(1).getString();
      option.description = query.getColumn(2).getString();
      option.defaultValue = query.getColumn(3).getString();
      option.category = query.getColumn(5).getString();
      option.categoryLabel = query.getColumn(6).getString();

      const auto valuesJson = nlohmann::json::parse(
          query.getColumn(4).getString(), nullptr, /*allow_exceptions=*/false);
      if (valuesJson.is_array()) {
        for (const auto &v : valuesJson) {
          option.values.push_back({v.value("value", std::string{}),
                                   v.value("label", std::string{})});
        }
      }
      options.push_back(std::move(option));
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to read core options for {}: {}", coreName, e.what());
  }
  return options;
}

} // namespace firelight::settings
