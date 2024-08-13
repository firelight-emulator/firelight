#include "library_item_model.hpp"

#include "../app/db/sqlite_content_database.hpp"

#include <filesystem>
#include <spdlog/spdlog.h>

namespace firelight::gui {
  LibraryItemModel::LibraryItemModel(db::ILibraryDatabase *libraryDatabase,
                                     db::IContentDatabase *contentDatabase,
                                     db::IUserdataDatabase *userdataDatabase)
    : m_libraryDatabase(libraryDatabase), m_contentDatabase(contentDatabase),
      m_userdataDatabase(userdataDatabase) {
  }

  int LibraryItemModel::rowCount(const QModelIndex &parent) const {
    return m_items.size();
  }

  QVariant LibraryItemModel::data(const QModelIndex &index, int role) const {
    if (role < Qt::UserRole || index.row() >= m_items.size()) {
      return QVariant{};
    }

    auto item = m_items.at(index.row());

    switch (role) {
      case EntryId:
        return item.m_entryId;
      case DisplayName:
      case Qt::DisplayRole:
        return item.displayName;
      case CreatedAt:
        return item.createdAt;
      case PlatformName: {
        if (item.platformId == 7) {
          return "Nintendo 64";
        }
        if (item.platformId == 6) {
          return "SNES";
        }
        if (item.platformId == 5) {
          return "NES";
        }
        if (item.platformId == 2) {
          return "Game Boy Color";
        }
        if (item.platformId == 1) {
          return "Game Boy";
        }
        if (item.platformId == 3) {
          return "Game Boy Advance";
        }
        if (item.platformId == 10) {
          return "Nintendo DS";
        }
        return "Unknown";
      }
      case Playlists:
        return QVariant::fromValue(item.m_playlists);
      case ContentPath:
        return item.contentPath;
      case ParentGameName:
        return item.parentGameName;
      case LastPlayedAt:
        return item.lastPlayedAt;
      default:
        return QVariant{};
    }
  }

  QHash<int, QByteArray> LibraryItemModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[EntryId] = "id";
    roles[DisplayName] = "display_name";
    roles[PlatformName] = "platform_name";
    roles[ContentPath] = "content_path";
    roles[ParentGameName] = "parent_game_name";
    roles[LastPlayedAt] = "last_played_at";
    return roles;
  }

  void LibraryItemModel::updatePlaylistsForEntry(const int entryId) {
    const auto playlists = m_libraryDatabase->getPlaylistsForEntry(entryId);
    QVector<int> playlistIds;
    for (const auto &playlist: playlists) {
      playlistIds.push_back(playlist.id);
    }

    for (auto &item: m_items) {
      if (item.m_entryId == entryId) {
        item.m_playlists = playlistIds;
        break;
      }
    }
  }

  bool LibraryItemModel::isRomInLibrary(const int romId) const {
    if (romId == -1) {
      return false;
    }

    const auto rom = m_contentDatabase->getRom(romId);
    if (!rom.has_value()) {
      return false;
    }

    return !m_libraryDatabase->getMatchingLibraryEntries({.contentId = rom->md5})
        .empty();
  }

  bool LibraryItemModel::isModInLibrary(const int modId) const {
    if (modId == -1) {
      return false;
    }

    for (const auto &entry: m_libraryDatabase->getAllLibraryEntries()) {
      if (entry.modId == modId) {
        printf("Found mod in library: %d %s\n", modId, entry.displayName.c_str());
        return true;
      }
    }
    return false;
  }

  void LibraryItemModel::addModToLibrary(int modId) {
    const auto patches = m_contentDatabase->getMatchingPatches({.modId = modId});
    if (patches.empty()) {
      return;
    }

    // TODO
    if (patches.size() > 1) {
      spdlog::error("More than one patch found for mod {}", modId);
      return;
    }

    const auto &patch = patches.at(0);
    addPatchToLibrary(patch.id);
  }

  void LibraryItemModel::addPatchToLibrary(const int patchId) const {
    const auto patch = m_contentDatabase->getPatch(patchId);
    if (!patch.has_value()) {
      return;
    }

    std::filesystem::path path = patch->filename;
    copy(path, "roms/" + path.filename().string(),
         std::filesystem::copy_options::update_existing);
  }

  int LibraryItemModel::getCount() const { return m_items.size(); }

  void LibraryItemModel::refresh() {
    emit beginResetModel();
    m_items.clear();
    const auto entries = m_libraryDatabase->getAllLibraryEntries();

    for (const auto &entry: entries) {
      const auto playlists = m_libraryDatabase->getPlaylistsForEntry(entry.id);
      QVector<int> playlistIds;
      for (const auto &playlist: playlists) {
        playlistIds.push_back(playlist.id);
      }

      auto platformId = entry.platformId;
      std::string parentGameName;
      auto parent = m_libraryDatabase->getLibraryEntry(entry.parentEntryId);
      if (parent.has_value()) {
        parentGameName = parent->displayName;
        platformId = parent->platformId;
      }

      unsigned int lastSessionTime = 0;

      auto lastSession =
          m_userdataDatabase->getLatestPlaySession(entry.contentId);
      if (lastSession.has_value()) {
        lastSessionTime = lastSession->endTime;
      } else {
        lastSessionTime = entry.createdAt;
      }

      m_items.emplace_back(
        Item({
          entry.id, QString::fromStdString(entry.displayName), platformId,
          playlistIds, entry.createdAt,
          QString::fromStdString(entry.contentPath),
          QString::fromStdString(parentGameName), lastSessionTime
        }));
    }
    emit countChanged();
    emit endResetModel();
  }
} // namespace firelight::gui
