#pragma once

#include "db/game_release.hpp"
#include "db/mod.hpp"
#include "db/mod_release.hpp"
#include "db/patch.hpp"
#include "db/region.hpp"
#include "firelight/db/platform.hpp"
#include "firelight/game.hpp"
#include "firelight/rom.hpp"
#include "firelight/romhack.hpp"

#include <memory>
#include <optional>

class IContentDatabase {
public:
  virtual ~IContentDatabase() = default;

  /**
   * @param tableName The name of the table to check for.
   * @return true if the table exists, false otherwise.
   */
  virtual bool tableExists(const std::string &tableName) = 0;

  virtual std::optional<firelight::db::GameRelease> getGameRelease(int id) = 0;
  virtual std::optional<ROM> getRom(int id) = 0;
  virtual std::optional<firelight::db::Mod> getMod(int id) = 0;
  virtual std::optional<firelight::db::ModRelease> getModRelease(int id) = 0;
  virtual std::optional<firelight::db::Patch> getPatch(int id) = 0;
  virtual std::optional<Platform> getPlatform(int id) = 0;
  virtual std::optional<firelight::db::Region> getRegion(int id) = 0;
  virtual std::vector<firelight::db::Mod> getAllMods() = 0;

  virtual std::vector<ROM> getMatchingRoms(const ROM &rom) = 0;
  virtual std::vector<firelight::db::Patch>
  getMatchingPatches(const firelight::db::Patch &patch) = 0;

  virtual std::optional<ROM> getRomByMd5(const std::string &md5) = 0;
  virtual std::optional<Game> getGameByRomId(int romId) = 0;
  virtual std::optional<Romhack> getRomhackByMd5(const std::string &md5) = 0;
  virtual std::optional<Platform> getPlatformByExtension(std::string ext) = 0;
};
