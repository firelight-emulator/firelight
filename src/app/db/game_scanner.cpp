//
// Created by alexs on 11/16/2023.
//

#include "game_scanner.hpp"
#include <filesystem>
#include <fstream>
#include <openssl/evp.h>

namespace FL::DB {

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

GameScanner::GameScanner(GameRepository *repository)
    : gameRepository(repository) {}

std::vector<FL::Library::Entry>
GameScanner::scanDirectory(const std::string &path, bool recursive) {
  std::vector<FL::Library::Entry> results;

  for (const auto &entry :
       std::filesystem::recursive_directory_iterator(path)) {
    if (!entry.is_directory()) {

      auto ext = entry.path().extension();
      if (ext == ".gb" || ext == ".gbc" || ext == ".z64" || ext == ".smc" ||
          ext == ".mod") {
        auto size = entry.file_size();

        if (size < MAX_FILESIZE_BYTES) {
          std::vector<char> thing(size);
          std::ifstream file(entry.path(), std::ios::binary);

          file.read(thing.data(), size);
          file.close();

          auto md5 = calculateMD5(thing.data(), size);

          if (ext == ".mod") {
            auto result = gameRepository->getRomhackByChecksum(md5);
            if (result != nullptr) {
              auto core = "";
              if (result->platform == "gb" || result->platform == "gbc") {
                core = "./system/_cores/gambatte_libretro.dll";
              } else if (result->platform == "n64") {
                core = "./system/_cores/mupen64plus_next_libretro.dll";
              } else if (result->platform == "snes") {
                core = "./system/_cores/snes9x_libretro.dll";
              }

              // TODO: Need to differentiate between versions.....
              results.push_back({.id = "testing",
                                 .type = Library::ROMHACK,
                                 .gameName = result->name,
                                 .gameId = result->id,
                                 .sourceGameId = result->sourceGameId,
                                 .corePath = core,
                                 .romPath = entry.path()});
            } else {
              printf("didn't find the romhack with name %ls\n",
                     entry.path().c_str());
            }
          } else {
            auto result = gameRepository->getGameByChecksum(md5);
            if (result != nullptr) {
              auto core = "";
              if (result->platform == "gb" || result->platform == "gbc") {
                core = "./system/_cores/gambatte_libretro.dll";
              } else if (result->platform == "n64") {
                core = "./system/_cores/mupen64plus_next_libretro.dll";
              } else if (result->platform == "snes") {
                core = "./system/_cores/snes9x_libretro.dll";
              }

              results.push_back({.id = "testing",
                                 .type = Library::NORMAL_GAME,
                                 .gameName = result->gameName,
                                 .gameId = result->id,
                                 .corePath = core,
                                 .romPath = entry.path()});
            } else {
              printf("didn't find the game with name %ls\n",
                     entry.path().c_str());
            }
          }
        }
      }
    }
  }

  return results;
}
} // namespace FL::DB
