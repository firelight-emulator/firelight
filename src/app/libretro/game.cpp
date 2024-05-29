//
// Created by alexs on 10/17/2023.
//

#include "game.hpp"
#include <filesystem>
#include <fstream>
#include <vector>

using std::vector;

using std::string;

namespace libretro {
  Game::Game(const string &filePath, const std::vector<unsigned char> &data) {
    // this->path = filePath;
    // this->path =
    // R"(/c/Users/alexs/git/firelight/build/roms/6149 - Pokemon - Black Version 2 (U) (frieNDS).nds)";
    this->data = data; // todo
  }

  Game::Game(const string &filePath) {
    this->path = filePath;

    const auto path = std::filesystem::path(filePath);
    const auto size = file_size(path);
    std::ifstream file(path, std::ios::binary);

    this->data.clear();
    this->data.resize(size);

    file.read(reinterpret_cast<char *>(this->data.data()), size);
    file.close();

    if (this->data.empty()) {
      // TODO
    }
  }

  string Game::getPath() { return this->path; }

  void *Game::getData() { return &this->data[0]; }

  size_t Game::getSize() {
    printf("size: %llu\n", this->data.size());
    return this->data.size();
  }
} // namespace libretro
