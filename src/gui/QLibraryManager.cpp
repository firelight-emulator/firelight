//
// Created by alexs on 12/22/2023.
//

#include "QLibraryManager.hpp"
#include <qfuture.h>
#include <qtconcurrentrun.h>

#include <openssl/evp.h>
#include <utility>

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

void QLibraryManager::refresh() const {
  // m_shortModel->setEntries(m_library_manager->getEntries());
}

void QLibraryManager::startScan() {
  QFuture<QString> future =
      QtConcurrent::run(scanner_thread_pool_.get(), [this] {
        emit scanStarted();
        for (const auto &file : std::filesystem::recursive_directory_iterator(
                 default_rom_path_)) {
          if (file.is_directory()) {
            continue;
          }

          // TODO: Recognize patch types too
          // Get Platform from extension
          auto ext = file.path().extension();
          auto platform =
              content_database_->getPlatformByExtension(ext.string());

          if (!platform.has_value()) {
            printf("not a game bruh\n");
          } else {
            printf("found game: %s\n", file.path().string().c_str());
          }
        }

        emit scanFinished();
        // std::vector<Entry> results;
        //
        // for (const auto &path : watchedRomDirs) {
        //   for (const auto &entry :
        //        std::filesystem::recursive_directory_iterator(path)) {
        //     if (entry.is_directory()) {
        //       continue;
        //     }
        //
        //     auto ext = entry.path().extension();
        //     std::string platform_display_name =
        //         get_display_name_by_extension(ext.string());
        //     if (platform_display_name.empty()) {
        //       continue;
        //     }
        //
        //     auto size = entry.file_size();
        //     if (size > MAX_FILESIZE_BYTES) {
        //       continue;
        //     }
        //
        //     std::vector<char> thing(size);
        //     std::ifstream file(entry.path(), std::ios::binary);
        //
        //     file.read(thing.data(), size);
        //     file.close();
        //
        //     auto md5 = calculateMD5(thing.data(), size);
        //
        //     auto result = contentDatabase->getRomByMd5(md5);
        //     if (result.has_value()) {
        //       auto game = contentDatabase->getGameByRomId(result->id);
        //
        //       auto displayName = result->filename;
        //       if (game.has_value()) {
        //         displayName = game->name;
        //       }
        //       // TODO: Will need to do a game lookup here as well.
        //       Entry e = {.id = -1,
        //                  .display_name = displayName,
        //                  .verified = true,
        //                  .platform = result->platform,
        //                  .md5 = md5,
        //                  .game = result->game,
        //                  .rom = result->id,
        //                  .romhack = -1,
        //                  .content_path = entry.path().string()};
        //       insertEntry(e);
        //     } else {
        //       Entry e = {.id = -1,
        //                  .display_name = entry.path().stem().string(),
        //                  .verified = false,
        //                  .platform =
        //                      platform_display_name, // TODO: Based on file
        //                      extension
        //                  .md5 = md5,
        //                  .game = -1,
        //                  .rom = -1,
        //                  .romhack = -1,
        //                  .content_path = entry.path().string()};
        //       insertEntry(e);
        //       // TODO: Start trying other stuff
        //     }
        //
        //     // Check for MD5 match. If a match, it's verified!
        //     // Otherwise, it's unverified.
        //     // If unverified, match the filename against the known filenames
        //     or
        //     // game name.
        //     // If THAT fails, add it to a list that the user needs to
        //     manually
        //     // assign.
        //   }
        // }
        return QString();
      });
}
