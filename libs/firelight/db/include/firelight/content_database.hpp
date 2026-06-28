#pragma once

#include "db/game.hpp"
#include "db/mod.hpp"
#include "db/patch.hpp"
#include "db/region.hpp"
#include "db/rom.hpp"
#include "firelight/db/platform.hpp"

#include <memory>
#include <optional>
#include <vector>

namespace firelight::db {
    class IContentDatabase {
    public:
        virtual ~IContentDatabase() = default;

        /**
         * @param tableName The name of the table to check for.
         * @return true if the table exists, false otherwise.
         */
        virtual bool tableExists(const std::string &tableName) = 0;

        virtual std::optional<Game> getGame(int id) = 0;

        virtual std::optional<ROM> getRom(int id) = 0;

        virtual std::vector<ROM> getMatchingRoms(const ROM &rom) = 0;

        virtual std::optional<int> getRetroAchievementsIdForGame(int id) = 0;

        virtual std::optional<Mod> getMod(int id) = 0;

        virtual std::vector<Mod> getAllMods() = 0;

        virtual std::optional<Patch> getPatch(int id) = 0;

        virtual std::vector<Patch> getMatchingPatches(const Patch &patch) = 0;

        virtual std::optional<Platform> getPlatform(int id) = 0;

        virtual std::vector<Platform>
        getMatchingPlatforms(const Platform &platform) = 0;

        virtual std::optional<Region> getRegion(int id) = 0;
    };
} // namespace firelight::db
