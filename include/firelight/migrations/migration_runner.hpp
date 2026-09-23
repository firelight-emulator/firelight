#pragma once

#include <functional>
#include <vector>

namespace firelight::migrations {

// One forward-only schema step
struct Migration {
  int version; // > 0, strictly increasing
  std::function<void()> apply;
};

// Applies every migration whose version exceeds currentVersion, in list order
inline int applyMigrations(const int currentVersion, const std::vector<Migration> &migrations,
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
