#pragma once

#include "firelight/game.hpp"
#include "firelight/platform.hpp"
#include "firelight/rom.hpp"
#include "firelight/romhack.hpp"

#include <memory>
#include <optional>

class IContentDatabase {
public:
  virtual ~IContentDatabase() = default;
  virtual std::optional<ROM> getRomByMd5(const std::string &md5) = 0;
  virtual std::optional<Game> getGameByRomId(int romId) = 0;
  virtual std::optional<Romhack> getRomhackByMd5(const std::string &md5) = 0;
  virtual std::optional<Platform> getPlatformByExtension(std::string ext) = 0;
};
