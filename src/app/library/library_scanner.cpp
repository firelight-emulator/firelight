#include "library_scanner.hpp"

#include <fstream>
#include <openssl/evp.h>
#include <qfuture.h>
#include <qtconcurrentrun.h>
#include <spdlog/spdlog.h>
#include <utility>

constexpr int MAX_FILESIZE_BYTES = 750000000;

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

LibraryScanner::LibraryScanner(
    firelight::db::ILibraryDatabase *lib_database,
    std::filesystem::path default_rom_path,
    firelight::db::IContentDatabase *content_database)
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

          auto filesize = entry.file_size();
          if (filesize > MAX_FILESIZE_BYTES) {
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

          std::vector<char> contents(filesize);
          std::ifstream file(entry.path(), std::ios::binary);

          file.read(contents.data(), filesize);
          file.close();

          auto contentMd5 = calculateMD5(contents.data(), filesize);
          // Check if we have an entry with this md5 - if so, update the
          // filename

          // Check if it's a rom or a patch

          // Check against content database

          if (auto ext = entry.path().extension();
              ext.string() == ".mod" || ext.string() == ".ips" ||
              ext.string() == ".ups" || ext.string() == ".bps" ||
              ext.string() == ".ups") {
            handleScannedPatchFile(entry, scan_results);
          } else if (ext.string() == ".smc" || ext.string() == ".n64" ||
                     ext.string() == ".v64" || ext.string() == ".z64" ||
                     ext.string() == ".gb" || ext.string() == ".gbc" ||
                     ext.string() == ".gba" || ext.string() == ".nds" ||
                     ext.string() == ".md" || ext.string() == ".sfc") {
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
}
bool LibraryScanner::scanning() const { return scanning_; }

void LibraryScanner::handleScannedRomFile(
    const std::filesystem::directory_entry &entry,
    ScanResults &scan_results) const {
  auto ext = entry.path().extension();
  auto size = entry.file_size();

  auto platforms = content_database_->getMatchingPlatforms(
      {.supportedExtensions = {ext.string()}});

  if (platforms.empty()) {
    printf("File extension not recognized: %s\n", ext.string().c_str());
    return;
  }

  std::vector<char> thing(size);
  std::ifstream file(entry.path(), std::ios::binary);

  file.read(thing.data(), size);
  file.close();

  auto md5 = calculateMD5(thing.data(), size);
  scan_results.all_md5s.emplace_back(md5);

  auto display_name = entry.path().filename().string();

  firelight::db::LibraryEntry e = {
      .displayName = display_name,
      .contentMd5 = md5,
      .platformId = platforms.at(0).id,
      .type = firelight::db::LibraryEntry::EntryType::ROM,
      .sourceDirectory = entry.path().parent_path().string(),
      .contentPath = entry.path().relative_path().string()};

  auto roms = content_database_->getMatchingRoms({.md5 = md5});
  if (!roms.empty()) {
    auto rom = roms.at(0);

    auto game = content_database_->getGame(rom.gameId);
    if (game.has_value()) {
      e.displayName = game->name;
    }
  }

  scan_results.new_entries.emplace_back(e);
}

void LibraryScanner::handleScannedPatchFile(
    const std::filesystem::directory_entry &entry,
    ScanResults &scan_results) const {
  auto ext = entry.path().extension();
  auto size = entry.file_size();

  std::vector<char> thing(size);
  std::ifstream file(entry.path(), std::ios::binary);

  file.read(thing.data(), size);
  file.close();
  const auto md5 = calculateMD5(thing.data(), size);

  const auto patches = content_database_->getMatchingPatches({.md5 = md5});
  if (patches.empty()) {
    // TODO: Just add it to the library without any extra info...
    return;
  }

  auto displayName = entry.path().filename().string();
  auto parent = -1;

  // TODO: Check for more than one?? Shouldn't happen
  const auto &patch = patches.at(0);
  auto rom = content_database_->getRom(patch.romId);
  if (rom.has_value()) {
    auto existing = library_database_->getMatchingLibraryEntries(
        {.contentMd5 = rom.value().md5,
         .type = firelight::db::LibraryEntry::EntryType::ROM});

    if (!existing.empty()) {
      parent = existing.at(0).id;
    }
    // romId = rom.value().id;
  }

  auto mod = content_database_->getMod(patch.modId);
  if (mod.has_value()) {
    displayName = mod.value().name;
  }

  firelight::db::LibraryEntry e = {
      .displayName = displayName,
      .contentMd5 = md5,
      .parentEntryId = parent,
      .type = firelight::db::LibraryEntry::EntryType::PATCH,
      .sourceDirectory = entry.path().parent_path().string(),
      .contentPath = entry.path().relative_path().string()};

  scan_results.new_entries.emplace_back(e);
}
