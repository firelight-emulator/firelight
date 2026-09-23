#pragma once

#include <string>

namespace firelight::db {

// TODO
/**
 * The health of one SQLite file on disk
 */
struct DatabaseInfo {
  std::string path;
  bool exists = false;
  bool opens = false;
  bool integrityOk = false;
  // TODO
  // PRAGMA user_version; 0 means the schema is unversioned
  int userVersion = 0;
  int tableCount = 0;
  std::string error;
};

// TODO
/** Opens a database read-only and reports its health */
DatabaseInfo inspect(const std::string &path);

} // namespace firelight::db
