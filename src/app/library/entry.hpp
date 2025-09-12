#pragma once

#include <QString>

namespace firelight::library {
struct Entry {
  int id = -1;
  QString displayName;
  QString contentHash;
  unsigned platformId = 0;
  unsigned activeSaveSlot = 1;
  bool hidden = false;
  QString icon1x1SourceUrl;
  QString boxartFrontSourceUrl;
  QString boxartBackSourceUrl;
  QString description;
  unsigned releaseYear = 0;
  QString developer;
  QString publisher;
  QString genres;
  QString regionIds;
  unsigned retroachievementsSetId = 0;

  std::vector<int> folderIds{};
  uint64_t createdAt = 0;
};
} // namespace firelight::library
