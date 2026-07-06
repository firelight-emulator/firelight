#pragma once

#include <firelight/metadata/game_metadata.hpp>

#include <optional>
#include <string>

namespace firelight::metadata {
  /**
   * Source of game metadata (e.g. title, platform, release date) for a given content hash.
   * Implementations may resolve metadata from a local database, a remote service, or any other source.
   */
  class IGameMetadataSource {
  public:
    virtual ~IGameMetadataSource() = default;

    /**
     * @param contentHash The content hash of the game for which to look up metadata.
     * @return The metadata for the game, or std::nullopt if no metadata could be found for the given content hash.
     */
    [[nodiscard]] virtual std::optional<GameMetadata>
    lookup(const std::string &contentHash) = 0;
  };
}
