#include "database_inspector.hpp"

#include <SQLiteCpp/SQLiteCpp.h>
#include <filesystem>

namespace firelight::db {

DatabaseInfo inspect(const std::string &path) {
  DatabaseInfo info;
  info.path = path;

  std::error_code ec;
  info.exists = std::filesystem::exists(path, ec);
  if (!info.exists) {
    return info;
  }

  try {
    SQLite::Database db(path, SQLite::OPEN_READONLY);
    info.opens = true;

    SQLite::Statement integrity(db, "PRAGMA integrity_check");
    if (integrity.executeStep()) {
      info.integrityOk = integrity.getColumn(0).getString() == "ok";
    }

    SQLite::Statement version(db, "PRAGMA user_version");
    if (version.executeStep()) {
      info.userVersion = version.getColumn(0).getInt();
    }

    SQLite::Statement tables(db, "SELECT COUNT(*) FROM sqlite_master WHERE type = 'table'");
    if (tables.executeStep()) {
      info.tableCount = tables.getColumn(0).getInt();
    }
  } catch (const std::exception &e) {
    info.error = e.what();
  }

  return info;
}

} // namespace firelight::db
