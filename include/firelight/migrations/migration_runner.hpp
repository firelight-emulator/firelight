#pragma once

#include <functional>
#include <vector>

namespace firelight::migrations {

// One forward-only schema step. `apply` runs this version's DDL; it runs only
// when the database's stored version is below `version`
struct Migration {
  int version; // > 0, strictly increasing across the list
  std::function<void()> apply;
};

// Applies every migration whose version exceeds currentVersion, in list order,
// running apply() then setVersion(version) for each; returns the version the
// database ends at. The caller reads currentVersion (from PRAGMA user_version),
// writes it back in setVersion, and should wrap the call in a transaction so a
// throwing migration rolls back. Migrations must be listed in ascending order
// Engine-agnostic: nothing here depends on SQLiteCpp or QSqlDatabase
inline int applyMigrations(int currentVersion,
                           const std::vector<Migration> &migrations,
                           const std::function<void(int)> &setVersion) {
  int version = currentVersion;
  for (const auto &migration : migrations) {
    if (migration.version > version) {
      migration.apply();
      setVersion(migration.version);
      version = migration.version;
    }
  }
  return version;
}

} // namespace firelight::migrations
