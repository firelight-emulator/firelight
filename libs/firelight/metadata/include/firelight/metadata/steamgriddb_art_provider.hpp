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

    [[nodiscard]] std::vector<ArtCandidate>
    search(const std::string &gameName, int platformId, MediaType type) override;

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
    [[nodiscard]] std::vector<GameMatch> resolveGames(const std::string &gameName,
                                                      int limit) const;

    /**
     * Append art candidates for the given game to the output vector, up to the specified remaining count
     */
    void collectArtForGame(const GameMatch &game, MediaType type, int remaining,
                           std::vector<ArtCandidate> &out) const;

    [[nodiscard]] HttpResponse apiGet(const std::string &path) const;

    IHttpClient &m_http;
    std::string m_apiKey;
  };
}
