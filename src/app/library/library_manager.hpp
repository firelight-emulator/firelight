//
// Created by alexs on 12/13/2023.
//

#ifndef FIRELIGHT_LIBRARY_MANAGER_HPP
#define FIRELIGHT_LIBRARY_MANAGER_HPP

#include "../db/content_database.hpp"
#include "entry.hpp"
#include <sqlite3.h>
#include <unordered_map>
#include <vector>

namespace FL::Library {

class LibraryManager {
public:
  LibraryManager(std::filesystem::path libraryFile,
                 const std::filesystem::path &defaultRomPath,
                 ContentDatabase *contentDb);

  /**
   * Perform an on-command scan of the content directories to get up to date.
   */
  void scanNow();

  std::optional<Entry> getEntryById(int id);

  /**
   * Add a path to be scanned and continually watched for ROM files.
   * @param romPath The path to a directory which contains ROM files.
   */
  void addWatchedRomDirectory(const std::filesystem::path &romPath);

  /**
   *
   * @param romId
   * @return A pointer to the found Entry if it exists.
   */
  static std::weak_ptr<Entry> getByRomId(std::string romId);

  /**
   * @param gameId
   * @return True if the Library contains an Entry with the given gameId.
   */
  static bool contains(int gameId);

  std::vector<Entry> getEntries();

private:
  ContentDatabase *contentDatabase = nullptr;
  void insertEntry(Entry &entry);
  sqlite3 *getOrCreateDbConnection() const;

  Entry entryFromDbRow(sqlite3_stmt *stmt);
  std::filesystem::path libraryDbFile;
  std::vector<std::filesystem::path> watchedRomDirs;
  std::vector<Entry> entries;
  std::unordered_map<std::string, std::string> romFileExtensions;

  sqlite3 *database;
};

} // namespace FL::Library

#endif // FIRELIGHT_LIBRARY_MANAGER_HPP
