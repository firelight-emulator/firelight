//
// Created by alexs on 12/13/2023.
//

#ifndef FIRELIGHT_LIBRARY_MANAGER_HPP
#define FIRELIGHT_LIBRARY_MANAGER_HPP

#include "entry.hpp"

namespace FL::Library {

class LibraryManager {
public:
  LibraryManager(const std::filesystem::path &libraryFile,
                 const std::filesystem::path &defaultRomPath);

  /**
   * Perform an on-command scan of the content directories to get up to date.
   */
  void scanNow();

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
  std::weak_ptr<Entry> getByRomId(std::string romId);

  /**
   * @param gameId
   * @return True if the Library contains an Entry with the given gameId.
   */
  bool contains(std::string gameId);

private:
  std::vector<std::filesystem::path> watchedRomDirs;
  std::vector<Entry> entries;
  std::unordered_map<std::string, std::string> romFileExtensions;
};

} // namespace FL::Library

#endif // FIRELIGHT_LIBRARY_MANAGER_HPP
