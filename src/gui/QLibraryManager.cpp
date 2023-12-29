//
// Created by alexs on 12/22/2023.
//

#include "QLibraryManager.hpp"

#include <fstream>
#include <qfuture.h>
#include <qtconcurrentrun.h>

#include <openssl/evp.h>
#include <spdlog/spdlog.h>
#include <utility>

constexpr int MAX_FILESIZE_BYTES = 50000000;

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
                                 QLibEntryModelShort *shortModel)
    : default_rom_path_(std::move(default_rom_path)),
      library_database_(lib_database), content_database_(content_database),
      m_shortModel(shortModel) {
  scanner_thread_pool_ = std::make_unique<QThreadPool>();
  scanner_thread_pool_->setMaxThreadCount(thread_pool_size_);
}

void QLibraryManager::startScan() {
  QFuture<ScanResults> future =
      QtConcurrent::run(scanner_thread_pool_.get(), [this] {
        emit scanStarted();
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
            spdlog::info("File %s too large; skipping\n",
                         entry.path().filename().string().c_str());
            continue;
          }

          if (!platform.has_value()) {
            printf("File extension not recognized: %s\n", ext.string().c_str());
            continue;
          }

          std::vector<char> thing(size);
          std::ifstream file(entry.path(), std::ios::binary);

          file.read(thing.data(), size);
          file.close();

          auto md5 = calculateMD5(thing.data(), size);
          if (library_database_->get_entry_by_md5(md5).has_value()) {
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
                        .platform_id = platform->id,
                        .md5 = md5,
                        .game = game_id,
                        .rom = rom_id,
                        .romhack = -1,
                        .content_path = absolute(entry.path()).string()};

          scan_results.new_entries.emplace_back(e);
        }

        printf("Summary of scan results:\n");
        printf("\tNew entries:\n");
        for (const auto &e : scan_results.new_entries) {
          printf("\t\t%s\n", e.display_name.c_str());
        }
        printf("\tExisting entries:\n");
        for (const auto &e : scan_results.existing_entries) {
          printf("\t\t%s\n", e.display_name.c_str());
        }

        emit scanFinished();
        return scan_results;
      });
}
