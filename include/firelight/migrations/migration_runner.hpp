#pragma once

#include <functional>
#include <vector>

namespace firelight::migrations {

/**
* A DB migration step. When applying migrations, each Migration whose version exceeds the current database version
* will have its apply() function called, and then the database version will be updated to that Migration's version
*/
struct Migration {
  int version;
  std::function<void()> apply;
};

/**
* Applies every migration whose version exceeds currentVersion, in order, running apply() then setVersion(version)
* for each one. Returns the version the database ends on
*/
inline int applyMigrations(int currentVersion, const std::vector<Migration> &migrations,
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
