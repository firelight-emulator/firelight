#include <firelight/metadata/steamgriddb_art_provider.hpp>
#include <firelight/util/strings.hpp>

#include <algorithm>
#include <cctype>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <utility>

namespace firelight::metadata {
namespace {
constexpr auto API_BASE = "https://www.steamgriddb.com/api/v2";

std::string urlEncode(const std::string &value) {
  static constexpr char hex[] = "0123456789ABCDEF";
  std::string out;
  out.reserve(value.size() * 3);
  for (const unsigned char c : value) {
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
  case MediaType::GridSquare:
  case MediaType::GridVertical:
    return "grids";
  default:
    return "";
  }
}

// The sizes the grid endpoint serves for each shape. Without this every grid request
// comes back with all four aspects mixed together, and a tile laid out for one
// cannot use art cut for another
std::string dimensionsForType(const MediaType type) {
  switch (type) {
  case MediaType::GridSquare:
    return "512x512,1024x1024";
  case MediaType::GridVertical:
    return "342x482,660x930";
  case MediaType::GridPortrait:
  case MediaType::BoxartFront:
    return "600x900";
  case MediaType::GridBanner:
    return "460x215,920x430";
  default:
    return "";
  }
}

// Comparable form of a title: case, punctuation and spacing all differ between what a
// filename says and what the database calls a game. The ampersand is folded before the
// punctuation goes, so "Ratchet & Clank" still reads as "Ratchet and Clank"
std::string normalizeTitle(const std::string &value) {
  const auto folded = strings::stripPunctuation(strings::foldAmpersand(value));
  return strings::replaceAll(strings::toLower(folded), " ", "");
}

// How well a result answers the question that was asked. Higher is better
int scoreGameMatch(const std::string &query, const std::string &candidate) {
  const auto wanted = normalizeTitle(query);
  const auto got = normalizeTitle(candidate);

  if (wanted.empty() || got.empty()) {
    return 0;
  }

  if (wanted == got) {
    return 3;
  }

  if (got.starts_with(wanted) || wanted.starts_with(got)) {
    return 2;
  }

  if (got.find(wanted) != std::string::npos || wanted.find(got) != std::string::npos) {
    return 1;
  }

  return 0;
}

// The shape an image actually is, so a candidate is not labelled with the shape that
// was asked for when the request had to be relaxed
MediaType gridTypeForSize(const int width, const int height) {
  if (width <= 0 || height <= 0) {
    return MediaType::GridSquare;
  }

  const auto ratio = static_cast<double>(width) / height;

  if (ratio > 1.5) {
    return MediaType::GridBanner;
  }

  if (ratio > 0.9) {
    return MediaType::GridSquare;
  }

  return ratio > 0.68 ? MediaType::GridVertical : MediaType::GridPortrait;
}

} // namespace

SteamGridDbArtProvider::SteamGridDbArtProvider(IHttpClient &http, std::string apiKey)
    : m_http(http), m_apiKey(std::move(apiKey)) {}

HttpResponse SteamGridDbArtProvider::apiGet(const std::string &path) const {
  return m_http.get(std::string(API_BASE) + path, {{"Authorization", "Bearer " + m_apiKey}});
}

std::vector<SteamGridDbArtProvider::GameMatch> SteamGridDbArtProvider::parseGames(const std::string &body,
                                                                                  const int limit) {
  std::vector<GameMatch> games;

  try {
    const auto json = nlohmann::json::parse(body);
    if (!json.value("success", false)) {
      return games;
    }
    const auto &data = json.at("data");
    if (!data.is_array()) {
      return games;
    }
    for (const auto &item : data) {
      if (static_cast<int>(games.size()) >= limit) {
        break;
      }
      if (item.contains("id") && item.at("id").is_number_integer()) {
        games.push_back({item.at("id").get<int>(), item.value("name", std::string{})});
      }
    }
  } catch (const std::exception &e) {
    spdlog::warn("SteamGridDB autocomplete parse failed: {}", e.what());
  }

  return games;
}

void SteamGridDbArtProvider::collectArtForGame(const GameMatch &game, const MediaType type, const int remaining,
                                               const bool restrictToShape, const int matchScore,
                                               std::vector<ArtCandidate> &out) const {
  const std::string endpoint = endpointForType(type);
  const auto dimensions = restrictToShape ? dimensionsForType(type) : std::string{};
  const auto query = dimensions.empty() ? std::string{} : "?dimensions=" + dimensions;
  const auto response = apiGet("/" + endpoint + "/game/" + std::to_string(game.id) + query);

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

    for (const auto &item : data) {
      if (static_cast<int>(out.size()) >= remaining) {
        break;
      }
      ArtCandidate candidate;
      candidate.gameName = game.name;
      candidate.matchScore = matchScore;
      candidate.url = item.value("url", std::string{});
      candidate.thumbUrl = item.value("thumb", candidate.url);
      candidate.width = item.value("width", 0);
      candidate.height = item.value("height", 0);
      candidate.type = endpoint == "grids" ? gridTypeForSize(candidate.width, candidate.height) : type;
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

ArtSearchResult SteamGridDbArtProvider::search(const std::string &gameName, int platformId, const MediaType type) {
  constexpr int MAX_GAMES = 6;

  ArtSearchResult result;
  if (!isConfigured() || endpointForType(type).empty()) {
    result.status = ArtSearchStatus::Failed;
    return result;
  }

  // The name lookup is the request whose failure means the search never happened.
  // Art for an individual game failing after that still leaves a usable answer
  const auto response = apiGet("/search/autocomplete/" + urlEncode(gameName));

  if (!response.ok()) {
    result.status = response.status == 429 ? ArtSearchStatus::RateLimited : ArtSearchStatus::Failed;
    return result;
  }

  auto games = parseGames(response.body, MAX_GAMES);

  // TODO
  // Autocomplete order is not match order, and the caller takes the first candidate.
  // Stable so equally-good matches keep the order the database gave them
  std::stable_sort(games.begin(), games.end(), [&](const GameMatch &left, const GameMatch &right) {
    return scoreGameMatch(gameName, left.name) > scoreGameMatch(gameName, right.name);
  });

  for (const auto &game : games) {
    constexpr int MAX_CANDIDATES = 60;
    if (static_cast<int>(result.candidates.size()) >= MAX_CANDIDATES) {
      break;
    }

    const auto score = scoreGameMatch(gameName, game.name);
    const auto before = result.candidates.size();
    collectArtForGame(game, type, MAX_CANDIDATES, true, score, result.candidates);

    // TODO
    // The right game in the wrong shape beats the wrong game in the right shape, so a
    // game with no art of the asked-for shape gives up the shape rather than its turn
    if (result.candidates.size() == before) {
      collectArtForGame(game, type, MAX_CANDIDATES, false, score, result.candidates);
    }
  }

  return result;
}
} // namespace firelight::metadata
