//
// Created by alexs on 12/22/2023.
//

#include "QLibraryManager.hpp"

#include <fstream>
#include <openssl/evp.h>
#include <qfuture.h>
#include <qtconcurrentrun.h>
#include <spdlog/spdlog.h>
#include <utility>

constexpr int MAX_FILESIZE_BYTES = 75000000;

static std::string calculateMD5(const char *input, int size) {
  unsigned char md5Hash[EVP_MAX_MD_SIZE];
  unsigned int md5HashLength;

  std::string output;

  // Create a message digest context
  auto mdctx = EVP_MD_CTX_new();

  // Initialize the message digest context
  if (mdctx == nullptr || !EVP_DigestInit_ex2(mdctx, EVP_md5(), nullptr)) {
    EVP_MD_CTX_free(mdctx);
    throw std::runtime_error("Failed to initialize hash context");
  }

  // Add data to the message digest context
  if (!EVP_DigestUpdate(mdctx, input, size)) {
    EVP_MD_CTX_free(mdctx);
    throw std::runtime_error("Failed to update hash context");
  }

  // Finalize the hash computation
  if (!EVP_DigestFinal_ex(mdctx, md5Hash, &md5HashLength)) {
    EVP_MD_CTX_free(mdctx);
    throw std::runtime_error("Failed to finalize hash context");
  }

  // Clean up the message digest context
  EVP_MD_CTX_free(mdctx);

  output.resize(md5HashLength * 2);
  for (unsigned int i = 0; i < md5HashLength; ++i)
    std::sprintf(&output[i * 2], "%02x", md5Hash[i]);

  return output;
}

QLibraryManager::QLibraryManager(LibraryDatabase *lib_database,
                                 std::filesystem::path default_rom_path,
                                 ContentDatabase *content_database,
                                 QLibraryViewModel *model)
    : default_rom_path_(std::move(default_rom_path)),
      library_database_(lib_database), content_database_(content_database),
      model_(model) {
  scanner_thread_pool_ = std::make_unique<QThreadPool>();
  scanner_thread_pool_->setMaxThreadCount(thread_pool_size_);
  directory_watcher_.addPath(
      QString::fromStdString(default_rom_path_.string()));

  // TODO: Probably don't need to scan everything every time
  connect(&directory_watcher_, &QFileSystemWatcher::directoryChanged,
          [&](const QString &) { startScan(); });
  connect(
      this, &QLibraryManager::scanFinished, model,
      [this, model] { model->set_items(get_model_items_()); },
      Qt::QueuedConnection);
}
std::optional<LibEntry> QLibraryManager::get_by_id(int id) const {
  auto result = library_database_->get_matching(LibraryDatabase::Filter({
      .id = id,
  }));

  if (result.empty()) {
    return {};
  }

  if (result.size() > 1) {
    spdlog::debug("uhhh??");
  }

  return {result.at(0)};
}

void QLibraryManager::startScan() {
  QFuture<ScanResults> future =
      QtConcurrent::run(scanner_thread_pool_.get(), [this] {
        // TODO: prob should lock the db from write access lol
        emit scanStarted();
        emit scanningChanged();
        scanning_ = true;
        ScanResults scan_results;

        for (const auto &entry :
             std::filesystem::recursive_directory_iterator(default_rom_path_)) {
          if (entry.is_directory()) {
            continue;
          }

          // TODO: Recognize patch types too
          auto ext = entry.path().extension();
          auto platform =
              content_database_->getPlatformByExtension(ext.string());

          auto size = entry.file_size();
          if (size > MAX_FILESIZE_BYTES) {
            // spdlog::info("File {} too large; skipping",
            //              entry.path().filename().string());
            continue;
          }

          if (!platform.has_value()) {
            printf("File extension not recognized: %s\n", ext.string().c_str());
            continue;
          }

          auto filename = entry.path().relative_path().string();
          auto libEntry =
              library_database_->get_matching(LibraryDatabase::Filter({
                  .content_path = filename,
              }));
          if (!libEntry.empty()) {
            spdlog::debug("Found library entry with filename {}; skipping",
                          filename);
            continue; // TODO: For now let's assume no change.
          }

          // check against filename and size and source

          std::vector<char> thing(size);
          std::ifstream file(entry.path(), std::ios::binary);

          file.read(thing.data(), size);
          file.close();

          auto md5 = calculateMD5(thing.data(), size);
          scan_results.all_md5s.emplace_back(md5);

          if (!library_database_
                   ->get_matching(LibraryDatabase::Filter({
                       .md5 = md5,
                   }))
                   .empty()) {
            // TODO: implement
            scan_results.existing_entries.emplace_back();
            continue;
          }

          auto display_name = entry.path().filename().string();
          auto verified = false;
          auto game_id = -1;
          auto rom_id = -1;

          auto rom = content_database_->getRomByMd5(md5);
          if (rom.has_value()) {
            auto game = content_database_->getGameByRomId(rom->id);
            if (game.has_value()) {
              display_name = game->name;
            }

            verified = true;
            game_id = game->id;
            rom_id = rom->id;
          }

          LibEntry e = {.id = -1,
                        .display_name = display_name,
                        .verified = verified,
                        .md5 = md5,
                        .platform = platform->id,
                        .game = game_id,
                        .rom = rom_id,
                        .romhack = -1,
                        .source_directory = entry.path().parent_path().string(),
                        .content_path = entry.path().relative_path().string()};

          scan_results.new_entries.emplace_back(e);
        }

        for (const auto &new_entry : scan_results.new_entries) {
          library_database_->add_or_update_entry(new_entry);
        }

        for (const auto &existing_entry : scan_results.existing_entries) {
          library_database_->add_or_update_entry(existing_entry);
        }

        emit scanningChanged();
        emit scanFinished();
        scanning_ = false;
        return scan_results;
      });

  // TODO: With scan results...
  // TODO: For each new entry, add it to the database.
  // TODO: For each existing entry, the main difference, if any, should be the
  // TODO:  filename, but there could be updated links too
  // TODO: Get a list of all md5s from the db. For each one, check if it's in
  // TODO:  the list of md5s we found when we scanned. If not, remove the entry
}
bool QLibraryManager::scanning() { return scanning_; }

std::vector<QLibraryViewModel::Item> QLibraryManager::get_model_items_() const {
  const auto all = library_database_->get_all_entries();

  std::vector<QLibraryViewModel::Item> items;
  for (const auto &e : all) {
    QLibraryViewModel::Item item;

    item.id = e.id;
    item.display_name = e.display_name;

    items.emplace_back(item);
  }

  return items;
}
