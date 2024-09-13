#include "library_scanner.hpp"

#include <fstream>
#include <qcryptographichash.h>
#include <qfile.h>
#include <openssl/evp.h>
#include <qfuture.h>
#include <qtconcurrentrun.h>
#include <spdlog/spdlog.h>
#include <utility>

#include "sqlite_library_database.hpp"

constexpr int MAX_FILESIZE_BYTES = 750000000;

LibraryScanner::LibraryScanner(
  firelight::db::ILibraryDatabase *lib_database,
  firelight::db::IContentDatabase *content_database)
  : library_database_(lib_database), content_database_(content_database) {
  scanner_thread_pool_ = std::make_unique<QThreadPool>();
  scanner_thread_pool_->setMaxThreadCount(thread_pool_size_);
  // directory_watcher_.addPath(
  //   QString::fromStdString(default_rom_path_.string()));

  m_scanDirectoryModel = new firelight::gui::LibraryPathModel(*lib_database);
  refreshDirectories();

  connect(m_scanDirectoryModel, &QAbstractListModel::dataChanged,
          [this](const QModelIndex &topLeft, const QModelIndex &bottomRight,
                 const QVector<int> &roles) {
            for (int i = topLeft.row(); i <= bottomRight.row(); ++i) {
              // printf("hi there: %s\n", m_scanDirectoryModel->data(
              //          m_scanDirectoryModel->index(i)).toString().toStdString().c_str());
              // m_scanDirectories[i] = m_scanDirectoryModel->data(
              //   m_scanDirectoryModel->index(i)).toString();
            }
          });

  // TODO: Probably don't need to scan everything every time
  connect(&directory_watcher_, &QFileSystemWatcher::directoryChanged,
          [&](const QString &) { startScan(); });
}

QAbstractListModel *LibraryScanner::scanDirectoryModel() const {
  return m_scanDirectoryModel;
}

void LibraryScanner::startScan() {
  QFuture<ScanResults> future =
      QtConcurrent::run(scanner_thread_pool_.get(), [this] {
        // TODO: prob should lock the db from write access lol
        emit scanStarted();
        emit scanningChanged();
        scanning_ = true;
        ScanResults scan_results;

        auto paths = library_database_->getAllLibraryContentDirectories();
        for (const auto &path: paths) {
          for (const auto &entry:
               std::filesystem::recursive_directory_iterator(path.path)) {
            if (entry.is_directory()) {
              continue;
            }

            auto filesize = entry.file_size();
            if (filesize > MAX_FILESIZE_BYTES) {
              spdlog::debug("File {} (size {}) too large; skipping",
                            entry.path().filename().string(),
                            entry.file_size());
              continue;
            }

            auto filename = canonical(entry.path()).string();
            scan_results.all_filenames.emplace_back(filename);

            auto existing = library_database_->getMatchingLibraryEntries(
              firelight::db::LibraryEntry{.contentPath = filename});
            if (!existing.empty()) {
              spdlog::debug("Found library entry with filename {}; skipping",
                            filename);
              continue;
            }
            // Check if we have an entry with this md5 - if so, update the
            // filename

            // Check if it's a rom or a patch

            // Check against content database

            if (auto ext = entry.path().extension();
              ext.string() == ".mod" || ext.string() == ".ips" ||
              ext.string() == ".ups" || ext.string() == ".bps" ||
              ext.string() == ".ups") {
              // std::vector<char> contents(filesize);
              // std::ifstream file(entry.path(), std::ios::binary);
              //
              // file.read(contents.data(), filesize);
              // file.close();
              //
              // auto contentId = calculateMD5(contents.data(), filesize);
              // handleScannedPatchFile(entry, scan_results);
              //
              // const auto md5 = calculateMD5(contents.data(), filesize);
              //
              // const auto patches =
              // content_database_->getMatchingPatches({.md5 = md5});
              // // if (patches.empty()) {
              // //   // TODO: Just add it to the library without any extra
              // info...
              // //   return;
              // // }
              //
              // auto displayName = entry.path().filename().string();
              // auto parent = -1;
              //
              // // TODO: Check for more than one?? Shouldn't happen
              // if (!patches.empty()) {
              //   const auto &patch = patches.at(0);
              //   auto rom = content_database_->getRom(patch.romId);
              //   if (rom.has_value()) {
              //     auto existing =
              //     library_database_->getMatchingLibraryEntries(
              //       {
              //         .contentId = rom.value().md5,
              //         .type = firelight::db::LibraryEntry::EntryType::ROM
              //       });
              //
              //     if (!existing.empty()) {
              //       parent = existing.at(0).id;
              //     }
              //     // romId = rom.value().id;
              //   }
              //
              //   auto mod = content_database_->getMod(patch.modId);
              //   if (mod.has_value()) {
              //     displayName = mod.value().name;
              //   }
              // }
              //
              // firelight::db::LibraryEntry e = {
              //   .displayName = displayName,
              //   .contentId = md5,
              //   .parentEntryId = parent,
              //   // .modId = patch.modId,
              //   .modId = -1,
              //   .type = firelight::db::LibraryEntry::EntryType::PATCH,
              //   .fileMd5 = md5,
              //   .fileCrc32 = md5,
              //   .sourceDirectory = entry.path().parent_path().string(),
              //   .contentPath = entry.path().relative_path().string()
              // };

              // scan_results.new_entries.emplace_back(e);
            } else if (ext.string() == ".smc" || ext.string() == ".n64" ||
                       ext.string() == ".v64" || ext.string() == ".z64" ||
                       ext.string() == ".gb" || ext.string() == ".gbc" ||
                       ext.string() == ".gba" || ext.string() == ".sfc" ||
                       ext.string() == ".nes" || ext.string() == ".iso") {
              auto platforms = content_database_->getMatchingPlatforms(
                {.supportedExtensions = {ext.string()}});

              if (platforms.empty()) {
                printf("File extension not recognized: %s\n",
                       ext.string().c_str());
                continue;
              }

              QFile file(entry.path().string().c_str());
              if (!file.open(QIODeviceBase::ReadOnly)) {
                printf(":(\n");
              }

              auto fileData = file.readAll();
              file.close();

              auto md5 = QCryptographicHash::hash(fileData, QCryptographicHash::Algorithm::Md5).toHex();
              printf("Md5: %s\n", md5.toStdString().c_str());
              scan_results.all_md5s.emplace_back(md5);

              auto contentId = md5;

              // TODO: Determine if ROM has a header
              if (ext == ".sfc" || ext == ".smc") {
                if (filesize % 1024 == 512) {
                  printf("FOUND HEADER!!! %s\n",
                         entry.path().filename().string().c_str());
                  fileData.remove(0, 512);
                  filesize -= 512;

                  contentId = QCryptographicHash::hash(fileData, QCryptographicHash::Algorithm::Md5).toHex();
                  // Copy thing into a new vector without the header
                  // std::vector<char> new_thing(size - 512);
                  // std::copy(thing.begin() + 512, thing.end(),
                  // new_thing.begin());
                }
              } else if (ext == ".nes") {
                if (fileData.startsWith("NES\x1A")) {
                  printf("FOUND HEADER!!! %s\n",
                         entry.path().filename().string().c_str());
                  fileData.remove(0, 16);
                  filesize -= 16;

                  contentId = QCryptographicHash::hash(fileData, QCryptographicHash::Algorithm::Md5).toHex();
                }
              }


              auto display_name = entry.path().filename().string();
              printf("display name: %s\n", display_name.c_str());

              firelight::db::LibraryEntry e = {
                .displayName = display_name,
                .contentId = contentId.toStdString(),
                .platformId = platforms.at(0).id,
                .type = firelight::db::LibraryEntry::EntryType::ROM,
                .fileMd5 = md5.toStdString(),
                .fileCrc32 = md5.toStdString(), // TODO: Calculate CRC32
                .sourceDirectory =
                canonical(entry.path().parent_path()).string(),
                .contentPath = canonical(entry.path()).string()
              };

              auto roms =
                  content_database_->getMatchingRoms({.md5 = contentId.toStdString()});
              if (!roms.empty()) {
                auto rom = roms.at(0);

                auto game = content_database_->getGame(rom.gameId);
                if (game.has_value()) {
                  e.gameId = game->id;
                  e.displayName = game->name;
                }
              }

              scan_results.new_entries.emplace_back(e);
            }
          }
        }

        auto allPaths = library_database_->getAllContentPaths();

        for (const auto &path: allPaths) {
          if (std::ranges::find(scan_results.all_filenames, path) ==
              scan_results.all_filenames.end()) {
            auto matchingEntry = library_database_->getMatchingLibraryEntries(
              {.contentPath = path});

            library_database_->deleteLibraryEntry(matchingEntry.at(0).id);
          }
        }

        for (auto &new_entry: scan_results.new_entries) {
          printf("Creating library entry: %s\n", new_entry.displayName.c_str());
          library_database_->createLibraryEntry(new_entry);
        }

        // for (const auto &existing_entry : scan_results.existing_entries) {
        //   library_database_->updateEntryContentPath(
        //       existing_entry.id, existing_entry.source_directory,
        //       existing_entry.content_path);
        // }

        emit scanningChanged();
        emit scanFinished();
        scanning_ = false;
        return scan_results;
      });
}

bool LibraryScanner::scanning() const { return scanning_; }

void LibraryScanner::refreshDirectories() {
  auto dirs = library_database_->getAllLibraryContentDirectories();

  for (const auto &dir: dirs) {
    directory_watcher_.addPath(QString::fromStdString(dir.path));
  }
}

void LibraryScanner::handleScannedPatchFile(
  const std::filesystem::directory_entry &entry,
  ScanResults &scan_results) const {
}
