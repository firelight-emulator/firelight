#include "library_scanner.hpp"

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

LibraryScanner::LibraryScanner(firelight::db::ILibraryDatabase *lib_database,
                               std::filesystem::path default_rom_path,
                               IContentDatabase *content_database)
    : default_rom_path_(std::move(default_rom_path)),
      library_database_(lib_database), content_database_(content_database) {
  scanner_thread_pool_ = std::make_unique<QThreadPool>();
  scanner_thread_pool_->setMaxThreadCount(thread_pool_size_);
  directory_watcher_.addPath(
      QString::fromStdString(default_rom_path_.string()));

  // TODO: Probably don't need to scan everything every time
  connect(&directory_watcher_, &QFileSystemWatcher::directoryChanged,
          [&](const QString &) { startScan(); });
}

void LibraryScanner::startScan() {
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

          if (entry.file_size() > MAX_FILESIZE_BYTES) {
            spdlog::debug("File {} (size {}) too large; skipping",
                          entry.path().filename().string(), entry.file_size());
            continue;
          }

          auto filename = entry.path().relative_path().string();
          auto existing = library_database_->getMatchingLibraryEntries(
              firelight::db::LibraryEntry{.contentPath = filename});
          if (!existing.empty()) {
            spdlog::debug("Found library entry with filename {}; skipping",
                          filename);
            continue;
          }

          if (auto ext = entry.path().extension();
              ext.string() == ".mod" || ext.string() == ".ips") {
            handleScannedPatchFile(entry, scan_results);
          } else if (ext.string() == ".smc" || ext.string() == ".n64" ||
                     ext.string() == ".v64" || ext.string() == ".z64" ||
                     ext.string() == ".gb" || ext.string() == ".gbc" ||
                     ext.string() == ".gba") {
            handleScannedRomFile(entry, scan_results);
          }
        }

        for (auto &new_entry : scan_results.new_entries) {
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

  // TODO: With scan results...
  // TODO: For each new entry, add it to the database.
  // TODO: For each existing entry, the main difference, if any, should be the
  // TODO:  filename, but there could be updated links too
  // TODO: Get a list of all md5s from the db. For each one, check if it's in
  // TODO:  the list of md5s we found when we scanned. If not, remove the entry
}
bool LibraryScanner::scanning() const { return scanning_; }

void LibraryScanner::handleScannedPatchFile(
    const std::filesystem::directory_entry &entry,
    ScanResults &scan_results) const {
  auto size = entry.file_size();
  if (size > MAX_FILESIZE_BYTES) {
    // spdlog::info("File {} too large; skipping",
    //              entry.path().filename().string());
    return;
  }

  std::vector<char> thing(size);
  std::ifstream file(entry.path(), std::ios::binary);

  file.read(thing.data(), size);
  file.close();

  auto md5 = calculateMD5(thing.data(), size);
  scan_results.all_md5s.emplace_back(md5);

  // auto romhack = content_database_->getRomhackByMd5(md5);
  // if (romhack.has_value()) {
  // }

  // TODO: Check content database for romhack with this md5
  // TODO: If not found, then user needs to select a rom to patch

  // LibEntry e = {.id = -1,
  //               .display_name = entry.path().filename().string(),
  //               .type = EntryType::PATCH,
  //               .verified = false,
  //               .md5 = md5,
  //               .platform = 1,
  //               .game = -1,
  //               .rom = -1,
  //               .parent_entry = -1,
  //               .romhack = -1,
  //               .romhack_release = -1,
  //               .source_directory = entry.path().parent_path().string(),
  //               .content_path = entry.path().relative_path().string()};
  //
  // scan_results.new_entries.emplace_back(e);
}

void LibraryScanner::handleScannedRomFile(
    const std::filesystem::directory_entry &entry,
    ScanResults &scan_results) const {
  auto ext = entry.path().extension();
  auto size = entry.file_size();

  auto platform = content_database_->getPlatformByExtension(ext.string());

  if (!platform.has_value()) {
    printf("File extension not recognized: %s\n", ext.string().c_str());
    return;
  }

  std::vector<char> thing(size);
  std::ifstream file(entry.path(), std::ios::binary);

  file.read(thing.data(), size);
  file.close();

  auto md5 = calculateMD5(thing.data(), size);
  scan_results.all_md5s.emplace_back(md5);

  auto matchingRoms =
      library_database_->getMatchingLibraryEntries(firelight::db::LibraryEntry{
          .contentMd5 = md5,
      });

  // if (!matchingRoms.empty()) {
  //   auto matching = matchingRoms.at(0);
  //   matching.source_directory = entry.path().parent_path().string();
  //   matching.content_path = entry.path().relative_path().string();
  //   scan_results.existing_entries.emplace_back(matching);
  //   return;
  // }

  auto display_name = entry.path().filename().string();
  auto verified = false;
  auto game_id = -1;
  auto rom_id = -1;

  firelight::db::LibraryEntry e = {
      .id = -1,
      .displayName = display_name,
      .contentMd5 = md5,
      .platformId = platform->id,
      .type = firelight::db::LibraryEntry::EntryType::ROM,
      .sourceDirectory = entry.path().parent_path().string(),
      .contentPath = entry.path().relative_path().string()};

  // if (auto rom = content_database_->getRomByMd5(md5); rom.has_value()) {
  //   auto game = content_database_->getGameByRomId(rom->id);
  //   if (game.has_value()) {
  //     display_name = game->name;
  //   }
  //
  //   verified = true;
  //   game_id = game->id;
  //   rom_id = rom->id;
  // }

  // LibEntry e = {.id = -1,
  //               .display_name = display_name,
  //               .type = EntryType::ROM,
  //               .verified = verified,
  //               .md5 = md5,
  //               .platform = platform->id,
  //               .game = game_id,
  //               .rom = rom_id,
  //               .parent_entry = -1,
  //               .romhack = -1,
  //               .romhack_release = -1,
  //               .source_directory = entry.path().parent_path().string(),
  //               .content_path = entry.path().relative_path().string()};

  scan_results.new_entries.emplace_back(e);
}
