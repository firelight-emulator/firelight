//
// Created by alexs on 10/17/2023.
//

#ifndef FIRELIGHT_GAME_HPP
#define FIRELIGHT_GAME_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

using std::string;

namespace libretro {

class Game {
public:
  explicit Game(const std::vector<unsigned char> &data);
  explicit Game(const string &filePath);

  string getPath();

  void *getData();

  size_t getSize();

private:
  std::vector<unsigned char> data;
  string path;
};

} // namespace libretro

#endif // FIRELIGHT_GAME_HPP
