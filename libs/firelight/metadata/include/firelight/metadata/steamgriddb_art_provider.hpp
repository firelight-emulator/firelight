#pragma once

#include <firelight/metadata/art_provider.hpp>
#include <firelight/metadata/http_client.hpp>

#include <optional>
#include <string>
#include <utility>

namespace firelight::metadata {
/**
 * Art provider that fetches art from SteamGridDB (https://www.steamgriddb.com/)
 * Requires an API key (user must register on the site to get one)
 */
class SteamGridDbArtProvider final : public IArtProvider {
public:
  SteamGridDbArtProvider(IHttpClient &http, std::string apiKey);

  [[nodiscard]] std::string name() const override { return "SteamGridDB"; }

  [[nodiscard]] bool isConfigured() const override { return !m_apiKey.empty(); }

  /**
   * @param apiKey The user's SteamGridDB API key (must be registered on the site to get one)
   */
  void setApiKey(std::string apiKey) { m_apiKey = std::move(apiKey); }

  [[nodiscard]] ArtSearchResult search(const std::string &gameName, int platformId, MediaType type) override;

private:
  struct GameMatch {
    int id = 0;
    std::string name;
  };

  /**
   * @param gameName The name of the game to search for
   * @param limit The maximum number of matches to return
   * @return List of game matches (id and name) for the given game name, up to the specified limit
   */
  /**
   * @param body The autocomplete response body
   * @param limit The maximum number of matches to return
   * @return The game matches named in the body, up to limit
   */
  [[nodiscard]] static std::vector<GameMatch> parseGames(const std::string &body, int limit);

  /**
   * Append art candidates for the given game to the output vector, up to the specified remaining count
   */
  /**
   * Appends one game's art to out.
   *
   * @param restrictToShape Whether to ask only for the sizes matching type. Dropping it is how a
   *   game with no art of the wanted shape still contributes rather than yielding to a worse-matching
   *   game that happens to have one
   */
  void collectArtForGame(const GameMatch &game, MediaType type, int remaining, bool restrictToShape, int matchScore,
                         std::vector<ArtCandidate> &out) const;

  [[nodiscard]] HttpResponse apiGet(const std::string &path) const;

  IHttpClient &m_http;
  std::string m_apiKey;
};
} // namespace firelight::metadata
