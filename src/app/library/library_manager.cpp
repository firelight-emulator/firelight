//
// Created by alexs on 12/13/2023.
//

#include "library_manager.hpp"
#include <fstream>
#include <openssl/evp.h>

namespace FL::Library {
const int MAX_FILESIZE_BYTES = 50000000;

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

LibraryManager::LibraryManager(const std::filesystem::path &libraryFile,
                               const std::filesystem::path &defaultRomPath) {
  // read from library file, validate entries, add to list

  romFileExtensions[".gb"] = "gameboy";
  romFileExtensions[".gbc"] = "gameboy color";
  romFileExtensions[".gba"] = "gameboy";
  romFileExtensions[".z64"] = "gameboy";
  romFileExtensions[".smc"] = "gameboy";
  addWatchedRomDirectory(defaultRomPath);
}

void LibraryManager::scanNow() {
  std::vector<FL::Library::Entry> results;

  for (const auto &path : watchedRomDirs) {
    for (const auto &entry :
         std::filesystem::recursive_directory_iterator(path)) {
      if (entry.is_directory()) {
        continue;
      }

      auto ext = entry.path().extension();
      if (romFileExtensions.find(ext.string()) == romFileExtensions.end()) {
        continue;
      }

      auto size = entry.file_size();
      if (size >= MAX_FILESIZE_BYTES) {
        continue;
      }

      std::vector<char> thing(size);
      std::ifstream file(entry.path(), std::ios::binary);

      file.read(thing.data(), size);
      file.close();

      auto md5 = calculateMD5(thing.data(), size);
      printf("Got file: %ls, md5: %s\n", entry.path().c_str(), md5.c_str());

      // Check for MD5 match. If a match, it's verified!
      // Otherwise, it's unverified.
      // If unverified, match the filename against the known filenames or
      // game name.
      // If THAT fails, add it to a list that the user needs to manually assign.
    }
  }
}

void LibraryManager::addWatchedRomDirectory(
    const std::filesystem::path &romPath) {
  // TODO: Validations :-)
  watchedRomDirs.push_back(romPath);
}
std::weak_ptr<Entry> LibraryManager::getByRomId(std::string romId) {
  return std::weak_ptr<Entry>();
}

bool LibraryManager::contains(std::string gameId) {
  return std::any_of(entries.begin(), entries.end(),
                     [&gameId](const Entry &e) { return e.gameId == gameId; });
}
} // namespace FL::Library
