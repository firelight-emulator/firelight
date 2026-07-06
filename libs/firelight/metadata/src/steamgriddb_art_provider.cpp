#include <firelight/metadata/steamgriddb_art_provider.hpp>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <cctype>
#include <utility>

namespace firelight::metadata {
  namespace {
    constexpr auto API_BASE = "https://www.steamgriddb.com/api/v2";

    std::string urlEncode(const std::string &value) {
      static constexpr char hex[] = "0123456789ABCDEF";
      std::string out;
      out.reserve(value.size() * 3);
      for (const unsigned char c: value) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
          out += static_cast<char>(c);
        } else {
          out += '%';
          out += hex[c >> 4];
          out += hex[c & 0x0F];
        }
      }
      return out;
    }

    std::string endpointForType(const MediaType type) {
      switch (type) {
        case MediaType::Icon:
          return "icons";
        case MediaType::Logo:
          return "logos";
        case MediaType::Hero:
          return "heroes";
        case MediaType::BoxartFront:
        case MediaType::GridPortrait:
        case MediaType::GridBanner:
          return "grids";
        default:
          return "";
      }
    }
  } // namespace

  SteamGridDbArtProvider::SteamGridDbArtProvider(IHttpClient &http,
                                                 std::string apiKey)
    : m_http(http), m_apiKey(std::move(apiKey)) {
  }

  HttpResponse SteamGridDbArtProvider::apiGet(const std::string &path) const {
    return m_http.get(std::string(API_BASE) + path,
                      {{"Authorization", "Bearer " + m_apiKey}});
  }

  std::vector<SteamGridDbArtProvider::GameMatch>
  SteamGridDbArtProvider::resolveGames(const std::string &gameName,
                                       const int limit) const {
    std::vector<GameMatch> games;

    const auto response = apiGet("/search/autocomplete/" + urlEncode(gameName));
    if (!response.ok()) {
      return games;
    }

    try {
      const auto json = nlohmann::json::parse(response.body);
      if (!json.value("success", false)) {
        return games;
      }
      const auto &data = json.at("data");
      if (!data.is_array()) {
        return games;
      }
      for (const auto &item: data) {
        if (static_cast<int>(games.size()) >= limit) {
          break;
        }
        if (item.contains("id") && item.at("id").is_number_integer()) {
          games.push_back({
            item.at("id").get<int>(),
            item.value("name", std::string{})
          });
        }
      }
    } catch (const std::exception &e) {
      spdlog::warn("SteamGridDB autocomplete parse failed: {}", e.what());
    }

    return games;
  }

  void SteamGridDbArtProvider::collectArtForGame(const GameMatch &game,
                                                 const MediaType type,
                                                 const int remaining,
                                                 std::vector<ArtCandidate> &out) const {
    const std::string endpoint = endpointForType(type);
    const auto response =
        apiGet("/" + endpoint + "/game/" + std::to_string(game.id));

    if (!response.ok()) {
      return; // one game's art failing shouldn't sink the whole search
    }

    try {
      const auto json = nlohmann::json::parse(response.body);
      if (!json.value("success", false)) {
        return;
      }

      const auto &data = json.at("data");
      if (!data.is_array()) {
        return;
      }

      for (const auto &item: data) {
        if (static_cast<int>(out.size()) >= remaining) {
          break;
        }
        ArtCandidate candidate;
        candidate.type = type;
        candidate.gameName = game.name;
        candidate.url = item.value("url", std::string{});
        candidate.thumbUrl = item.value("thumb", candidate.url);
        candidate.width = item.value("width", 0);
        candidate.height = item.value("height", 0);
        if (item.contains("id") && !item.at("id").is_null()) {
          candidate.externalId = std::to_string(item.at("id").get<long long>());
        }
        if (!candidate.url.empty()) {
          out.push_back(std::move(candidate));
        }
      }
    } catch (const std::exception &e) {
      spdlog::warn("SteamGridDB {} parse failed: {}", endpoint, e.what());
    }
  }

  std::vector<ArtCandidate>
  SteamGridDbArtProvider::search(const std::string &gameName, int platformId,
                                 const MediaType type) {
    constexpr int MAX_GAMES = 6;

    std::vector<ArtCandidate> candidates;
    if (!isConfigured() || endpointForType(type).empty()) {
      return candidates;
    }

    const auto games = resolveGames(gameName, MAX_GAMES);
    for (const auto &game: games) {
      constexpr int MAX_CANDIDATES = 60;
      if (static_cast<int>(candidates.size()) >= MAX_CANDIDATES) {
        break;
      }
      collectArtForGame(game, type, MAX_CANDIDATES, candidates);
    }

    return candidates;
  }
} // namespace firelight::metadata
