//
// Created by alexs on 12/22/2023.
//

#ifndef FIRELIGHT_CONTENT_DATABASE_HPP
#define FIRELIGHT_CONTENT_DATABASE_HPP

#include "daos/game.hpp"
#include "daos/platform.hpp"
#include "daos/rom.hpp"
#include "daos/romhack.hpp"

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

#endif // FIRELIGHT_CONTENT_DATABASE_HPP
