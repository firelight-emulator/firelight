#include "rcheevos_offline_client.hpp"

#include "../award_achievement_response.hpp"
#include "../gameid_response.hpp"
#include "../login2_response.hpp"
#include "../patch_response.hpp"
#include "../ra_constants.h"
#include "../startsession_response.hpp"
#include "cached_achievement.hpp"

#include <QDateTime>
#include <cpr/api.h>
#include <cpr/body.h>
#include <cpr/cprtypes.h>
#include <qcryptographichash.h>
#include <sstream>
#include <unordered_map>

#include <spdlog/spdlog.h>

namespace firelight::achievements {
static const std::string GAMEID = "gameid";
static const std::string PATCH = "patch";
static const std::string START_SESSION = "startsession";
static const std::string AWARD_ACHIEVEMENT = "awardachievement";
static const std::string LOGIN2 = "login2";
static const std::string PING = "ping";
static const std::string SUBMIT_LB_ENTRY = "submitlbentry";

static const rc_api_server_response_t GENERIC_SERVER_ERROR = {"", 0, 500};

static const rc_api_server_response_t GENERIC_SUCCESS = {"{\"Success\":true}",
                                                         16, 200};

std::unordered_map<std::string, std::string>
parseQueryParams(const std::string &query) {
  std::unordered_map<std::string, std::string> params;
  std::istringstream stream(query);
  std::string param;

  while (std::getline(stream, param, '&')) {
    std::size_t pos = param.find('=');
    if (pos != std::string::npos) {
      std::string key = param.substr(0, pos);
      std::string value = param.substr(pos + 1);
      params[key] = value;
    }
  }

  return params;
}

rc_api_server_response_t
RetroAchievementsOfflineClient::handleRequest(const std::string &url,
                                              const std::string &postBody,
                                              const std::string &contentType) {
  auto params = parseQueryParams(postBody);
  // const auto duration = std::chrono::system_clock::now().time_since_epoch();
  // const auto nowEpochMillis =
  // std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

  if (params["r"] == GAMEID) {
    return handleGameIdRequest(params["m"]);
  }

  if (params["r"] == PATCH) {
    return handlePatchRequest(stoi(params["g"]));
  }

  if (params["r"] == START_SESSION) {
    const auto user = params["u"];
    const auto gameId = stoi(params["g"]);
    const auto hardcore = params["h"] == "1";

    return handleStartSessionRequest(user, gameId, hardcore);
  }

  if (params["r"] == AWARD_ACHIEVEMENT) {
    const auto user = params["u"];
    const auto token = params["t"];
    const auto achievementId = stoi(params["a"]);
    const auto hardcore = params["h"] == "1";

    return handleAwardAchievementRequest(user, token, achievementId, hardcore);
  }

  if (params["r"] == LOGIN2) {
    return handleLogin2Request(params["u"], params["p"], params["t"]);
  }

  if (params["r"] == PING) {
    return handlePingRequest();
  }

  if (params["r"] == SUBMIT_LB_ENTRY) {
    return handleSubmitLbEntryRequest();
  }

  return GENERIC_SERVER_ERROR;
}

void RetroAchievementsOfflineClient::processResponse(
    const std::string &request, const std::string &response) const {
  auto params = parseQueryParams(request);
  if (params["r"] == "login2") {
    processLogin2Response(params["u"], params["t"], response);
  } else if (params["r"] == "gameid") {
    processGameIdResponse(params["m"], response);
  } else if (params["r"] == "patch") {
    processPatchResponse(params["u"], stoi(params["g"]), response);
  } else if (params["r"] == "startsession") {
    processStartSessionResponse(params["u"], stoi(params["g"]), response);
  } else if (params["r"] == "awardachievement") {
    processAwardAchievementResponse(params["u"], params["h"] == "1", response);
  }
}
void RetroAchievementsOfflineClient::syncOfflineAchievements() {

  // For each user... get all unsynced achievements. If they have any, try
  // logging in and syncing them If we can't log in we need to prompt the user
  // to re-login.

  const auto headers =
      cpr::Header{{"User-Agent", OFFLINE_USER_AGENT},
                  {"Content-Type", "application/x-www-form-urlencoded"}};

  for (const auto &user : m_cache.getUsers()) {
    auto unsynced = m_cache.getUnsyncedAchievements(user.username);
    if (unsynced.empty()) {
      continue;
    }

    // Try logging in for current user
    auto postBody = "r=login2&u=" + user.username + "&t=" + user.token;

    const auto response =
        Post(cpr::Url{RA_DOREQUEST_URL}, headers, cpr::Body{postBody});

    if (response.error) {
      spdlog::warn("Failed to log in user: {} ({})", user.username,
                   response.error.message);
      continue;
    }

    auto json = nlohmann::json::parse(response.text);
    if (json.contains("Success") && json["Success"].is_boolean() &&
        json["Success"].get<bool>()) {
      spdlog::info("Logged in for user {}", user.username);
    } else {
      spdlog::error("Login was not successful for user {}", user.username);
      continue;
    }

    // Logged in successfully

    int lastScore = m_cache.getUserScore(user.username, false);
    int lastHardcoreScore = m_cache.getUserScore(user.username, true);

    for (auto &achieve : unsynced) {
      auto timestamp = achieve.When;
      auto hardcore = false;

      auto unmarkHardcoreUnlock = false;

      // Stop those dang cheaters that manually modified the database
      if (achieve.EarnedHardcore) {
        if (std::ranges::find_if(m_currentSessionAchievements,
                                 [&achieve](const CachedAchievement &current) {
                                   return current.ID == achieve.ID;
                                 }) == m_currentSessionAchievements.end()) {
          spdlog::info("Achievement earned offline in hardcore mode is not in "
                       "current session; demoting to casual");
          achieve.EarnedHardcore = false;
          achieve.Earned = true;
          achieve.When = achieve.WhenHardcore;
          achieve.WhenHardcore = 0;

          timestamp = achieve.When;
          unmarkHardcoreUnlock = true;
        } else {
          hardcore = true;
          timestamp = achieve.WhenHardcore;
        }
      }

      spdlog::info("Syncing achievement with ID {} for user {}", achieve.ID,
                   user.username);

      auto secondsSinceUnlock = QDateTime::currentSecsSinceEpoch() - timestamp;

      auto hashContent = std::to_string(achieve.ID) + user.username +
                         std::to_string(hardcore ? 1 : 0) +
                         std::to_string(achieve.ID) +
                         std::to_string(secondsSinceUnlock);
      hashContent =
          QCryptographicHash::hash(hashContent, QCryptographicHash::Md5)
              .toHex()
              .toStdString();
      auto gameHash = m_cache.getHashFromGameId(achieve.GameID);

      if (!gameHash.has_value()) {
        spdlog::warn("No game hash found for game ID: {}", achieve.GameID);
        break;
      }

      auto postBody =
          "r=awardachievement&u=" + user.username + "&t=" + user.token +
          "&a=" + std::to_string(achieve.ID) +
          "&h=" + std::to_string(hardcore ? 1 : 0) + "&m=" + gameHash.value() +
          "&o=" + std::to_string(secondsSinceUnlock) + "&v=" + hashContent;

      const auto response =
          Post(cpr::Url{RA_DOREQUEST_URL}, headers, cpr::Body{postBody});

      if (response.error) {
        spdlog::warn("Failed to award achievement; will try later: {}",
                     response.error.message);
        continue;
      }

      auto json = nlohmann::json::parse(response.text);

      if (!json.contains("Success") || !json["Success"].is_boolean() ||
          !json["Success"].get<bool>()) {
        if (json.contains("Error") && json["Error"].is_string()) {
          auto errorString = json["Error"].get<std::string>();
          if (errorString.find("already has") == std::string::npos) {
            spdlog::warn("Got error: {}", errorString);
            continue;
          } else {
            spdlog::info("Server already knows user {} has achievement {}",
                         user.username, achieve.ID);
          }
        } else {
          spdlog::error("Unsuccessful response did not contain Error flag...");
          continue;
        }
      }

      if (json.contains("Score") && json["Score"].is_number()) {
        lastHardcoreScore = json["Score"];
      }

      if (json.contains("SoftcoreScore") && json["SoftcoreScore"].is_number()) {
        lastScore = json["SoftcoreScore"];
      }

      if (unmarkHardcoreUnlock) {
        if (!m_cache.markAchievementLocked(user.username, achieve.ID, true)) {
          spdlog::warn("Failed to unmark hardcore unlock for achievement: {}",
                       achieve.ID);
        }
      }

      if (!m_cache.markAchievementUnlocked(user.username, achieve.ID, hardcore,
                                           timestamp)) {
        spdlog::warn("Failed to mark achievement synced: {}", achieve.ID);
      }
    }

    m_cache.setUserScore(user.username, lastScore, false);
    m_cache.setUserScore(user.username, lastHardcoreScore, true);
  }
}

void RetroAchievementsOfflineClient::clearSessionAchievements() {
  m_inHardcoreSession = false;
  m_currentSessionAchievements.clear();
}

void RetroAchievementsOfflineClient::startOnlineHardcoreSession() {
  m_inHardcoreSession = true;
}

rc_api_server_response_t RetroAchievementsOfflineClient::handleGameIdRequest(
    const std::string &hash) const {
  const auto cached = m_cache.getGameIdFromHash(hash);
  if (!cached.has_value()) {
    return GENERIC_SERVER_ERROR;
  }

  GameIdResponse gameIdResponse{.Success = true, .GameID = cached.value()};

  const auto json = nlohmann::json(gameIdResponse).dump();

  rc_api_server_response_t rcResponse;
  rcResponse.http_status_code = 200;
  rcResponse.body = strdup(json.c_str());
  rcResponse.body_length = strlen(rcResponse.body);
  return rcResponse;
}

rc_api_server_response_t
RetroAchievementsOfflineClient::handlePatchRequest(const int gameId) const {
  auto cached = m_cache.getPatchResponse(gameId);
  if (!cached.has_value()) {
    return GENERIC_SERVER_ERROR;
  }

  const auto json = nlohmann::json(cached.value()).dump();

  rc_api_server_response_t rcResponse;
  rcResponse.http_status_code = 200;
  rcResponse.body = strdup(json.c_str());
  rcResponse.body_length = strlen(rcResponse.body);
  return rcResponse;
}

rc_api_server_response_t
RetroAchievementsOfflineClient::handleStartSessionRequest(
    const std::string &username, const int gameId, bool hardcore) const {

  // TODO: If hardcore.... disallow?

  // Simulate some latency...
  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  const auto duration = std::chrono::system_clock::now().time_since_epoch();
  const auto epochMillis =
      std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

  StartSessionResponse startSessionResponse{.Success = true,
                                            .HardcoreUnlocks = {},
                                            .Unlocks = {},
                                            .ServerNow = epochMillis};

  for (const auto &achieve : m_cache.getUserAchievements(username, gameId)) {
    if (achieve.Earned) {
      startSessionResponse.Unlocks.emplace_back(
          Unlock{.ID = achieve.ID, .When = achieve.When});
    }

    if (achieve.EarnedHardcore) {
      startSessionResponse.HardcoreUnlocks.emplace_back(
          Unlock{.ID = achieve.ID, .When = achieve.WhenHardcore});
    }
  }

  const auto json = nlohmann::json(startSessionResponse).dump();

  rc_api_server_response_t rcResponse;
  rcResponse.http_status_code = 200;
  rcResponse.body = strdup(json.c_str());
  rcResponse.body_length = strlen(rcResponse.body);
  return rcResponse;
}

rc_api_server_response_t
RetroAchievementsOfflineClient::handleAwardAchievementRequest(
    const std::string &username, const std::string &token,
    const int achievementId, const bool hardcore) {

  // If we don't know the game ID, we can't award the achievement, return
  auto gameId = m_cache.getGameIdFromAchievementId(achievementId);
  if (!gameId.has_value()) {
    spdlog::error("Failed to get game ID from achievement ID: {}",
                  achievementId);
    return GENERIC_SUCCESS; // TODO
  }

  // If there is no status for the achievement, return
  auto status = m_cache.getUserAchievementStatus(username, achievementId);
  if (!status.has_value()) {
    spdlog::error("Failed to get user achievement status: {}", achievementId);
    return GENERIC_SUCCESS; // TODO
  }

  // If the user already has the achievement, return
  if ((hardcore && status->achievedHardcore) ||
      (!hardcore && status->achieved)) {
    spdlog::info("User {} already has achievement {}", username, achievementId);
    return GENERIC_SUCCESS; // TODO
  }

  // Get achievement info
  auto achievement = m_cache.getAchievement(achievementId);
  if (!achievement.has_value()) {
    return GENERIC_SUCCESS; // TODO
  }

  // Update user points
  const auto currentPoints = m_cache.getUserScore(username, hardcore);
  m_cache.setUserScore(username, currentPoints + achievement->points, hardcore);

  if (hardcore) {
    status->achievedHardcore = true;
    status->timestampHardcore = QDateTime::currentSecsSinceEpoch();
  } else {
    status->achieved = true;
    status->timestamp = QDateTime::currentSecsSinceEpoch();
  }

  // Update unlock status
  m_cache.updateUserAchievementStatus(username, achievementId, status.value());

  // TODO: MARK THE ACHIEVEMENT AS UNSYNCED

  // If in hardcore session, update the current session achievements
  if (m_inHardcoreSession) {
    m_currentSessionAchievements.emplace_back(CachedAchievement{
        .ID = achievementId,
        .GameID = gameId.value(),
        .When = hardcore ? 0 : QDateTime::currentSecsSinceEpoch(),
        .WhenHardcore = hardcore ? QDateTime::currentSecsSinceEpoch() : 0,
        .Points = 0,
        .Earned = !hardcore,
        .EarnedHardcore = hardcore});
  }

  // Build the response
  AwardAchievementResponse resp{
      .Success = true,
      .Score = m_cache.getUserScore(username, true),
      .SoftcoreScore = m_cache.getUserScore(username, false),
      .AchievementID = achievementId,
      .AchievementsRemaining = m_cache.getNumRemainingAchievements(
          username, gameId.value(), hardcore)};

  const auto json = nlohmann::json(resp).dump();

  rc_api_server_response_t rcResponse;
  rcResponse.http_status_code = 200;
  rcResponse.body = strdup(json.c_str());
  rcResponse.body_length = strlen(rcResponse.body);
  return rcResponse;
}

rc_api_server_response_t RetroAchievementsOfflineClient::handleLogin2Request(
    const std::string &username, const std::string &password,
    const std::string &token) const {
  // TODO: How to handle unknown user.....
  if (!password.empty()) {
    return GENERIC_SERVER_ERROR;
  }

  Login2Response response{.AccountType = "1",
                          .Messages = 0,
                          .Permissions = 0,
                          .Score = m_cache.getUserScore(username, true),
                          .SoftcoreScore =
                              m_cache.getUserScore(username, false),
                          .Success = true,
                          .Token = token, // TODO: uhh
                          .User = username};

  const auto json = nlohmann::json(response).dump();

  rc_api_server_response_t rcResponse;
  rcResponse.http_status_code = 200;
  rcResponse.body = strdup(json.c_str());
  rcResponse.body_length = strlen(rcResponse.body);
  return rcResponse;
}

rc_api_server_response_t RetroAchievementsOfflineClient::handlePingRequest() {
  return GENERIC_SUCCESS;
}

rc_api_server_response_t
RetroAchievementsOfflineClient::handleSubmitLbEntryRequest() {
  return GENERIC_SUCCESS;
}

void RetroAchievementsOfflineClient::processLogin2Response(
    const std::string &username, const std::string &token,
    const std::string &response) const {

  auto json = nlohmann::json::parse(response);
  if (json.contains("Success") && json["Success"].is_boolean() &&
      json["Success"].get<bool>()) {
    if (!m_cache.addUser(username, token)) {
      spdlog::error("Failed to create user: {}", username);
    }
  } else {
    spdlog::error("Login response was not success");
    return;
  }

  if (json.contains("Score") && json["Score"].is_number()) {
    m_cache.setUserScore(username, json["Score"], true);
  }

  if (json.contains("SoftcoreScore") && json["SoftcoreScore"].is_number()) {
    m_cache.setUserScore(username, json["SoftcoreScore"], false);
  }
}

void RetroAchievementsOfflineClient::processGameIdResponse(
    const std::string &hash, const std::string &response) const {
  const auto json = nlohmann::json::parse(response);
  const auto gameidResponse = json.get<GameIdResponse>();
  m_cache.setGameId(hash, gameidResponse.GameID);
}

void RetroAchievementsOfflineClient::processPatchResponse(
    const std::string &username, const int gameId,
    const std::string &response) const {
  const auto json = nlohmann::json::parse(response);
  const auto patchResponse = json.get<PatchResponse>();
  m_cache.setPatchResponse(username, gameId, patchResponse);

  // TODO: Only insert ones that have the flags set right
  // TODO: Remove ones that aren't present in the patch response
  for (const auto &a : patchResponse.PatchData.Achievements) {
    auto achievement = Achievement{.id = a.ID,
                                   .name = a.Title,
                                   .description = a.Description,
                                   .points = a.Points,
                                   .setId = gameId,
                                   .flags = a.Flags};
    if (m_cache.createAchievement(achievement) == -1) {
      spdlog::error("Failed to create achievement: {}", achievement.id);
      continue;
    }

    if (!m_cache.getUserAchievementStatus(username, a.ID).has_value()) {
      auto newStatus = UserAchievementStatus{.achievementId = a.ID,
                                             .achieved = false,
                                             .achievedHardcore = false,
                                             .timestamp = 0,
                                             .timestampHardcore = 0};

      m_cache.updateUserAchievementStatus(username, a.ID, newStatus);
    }
  }
}

void RetroAchievementsOfflineClient::processStartSessionResponse(
    const std::string &username, const int gameId,
    const std::string &response) const {
  auto json = nlohmann::json::parse(response);
  // nlohmann lib doesn't support optional/nullable fields...
  if (!json.contains("Unlocks")) {
    json["Unlocks"] = std::vector<Unlock>();
  }

  if (!json.contains("HardcoreUnlocks")) {
    json["HardcoreUnlocks"] = std::vector<Unlock>();
  }

  auto startSessionResponse = json.get<StartSessionResponse>();

  auto foundUnsupportedEmu = false;

  // Non-hardcore unlocks
  for (const auto &a : startSessionResponse.Unlocks) {
    if (a.ID == UNSUPPORTED_EMULATOR_ACHIEVEMENT_ID) {
      foundUnsupportedEmu = true;
      continue;
    }
    if (!m_cache.markAchievementUnlocked(username, a.ID, false, a.When)) {
      spdlog::error("Failed to mark achievement unlocked: {}", a.ID);
    }
  }

  // Non-hardcore re-locks, in case user cleared in the RA site
  for (const auto &a : m_cache.getUserAchievements(username, gameId)) {
    // If the achievement is not in the startSessionResponse, mark it as NOT
    // unlocked
    if (std::ranges::find_if(startSessionResponse.Unlocks,
                             [&a](const Unlock &u) { return u.ID == a.ID; }) ==
        startSessionResponse.Unlocks.end()) {
      if (!m_cache.markAchievementLocked(username, a.ID, false)) {
        spdlog::error("Failed to mark achievement locked: {}", a.ID);
      }
    }
  }

  // Hardcore unlocks
  for (const auto &a : startSessionResponse.HardcoreUnlocks) {
    if (a.ID == UNSUPPORTED_EMULATOR_ACHIEVEMENT_ID) {
      foundUnsupportedEmu = true;
      continue;
    }
    if (!m_cache.markAchievementUnlocked(username, a.ID, true, a.When)) {
      spdlog::error("Failed to mark achievement unlocked: {}", a.ID);
    }
  }

  // Hardcore re-locks, in case user cleared in the RA site
  for (const auto &a : m_cache.getUserAchievements(username, gameId)) {
    // If the achievement is not in the startSessionResponse, mark it as NOT
    // unlocked
    if (std::ranges::find_if(startSessionResponse.HardcoreUnlocks,
                             [&a](const Unlock &u) { return u.ID == a.ID; }) ==
        startSessionResponse.HardcoreUnlocks.end()) {
      if (!m_cache.markAchievementLocked(username, a.ID, true)) {
        spdlog::error("Failed to mark achievement locked: {}", a.ID);
      }
    }
  }

  if (!foundUnsupportedEmu) {
    m_cache.markAchievementLocked(username, UNSUPPORTED_EMULATOR_ACHIEVEMENT_ID,
                                  false);
    m_cache.markAchievementLocked(username, UNSUPPORTED_EMULATOR_ACHIEVEMENT_ID,
                                  true);
  }

  // TODO: Go through all the user's unlocks and if it's synced but NOT in the
  // list above, mark it as NOT unlocked
}

void RetroAchievementsOfflineClient::processAwardAchievementResponse(
    const std::string &username, const bool hardcore,
    const std::string &response) const {
  // TODO: CHECK FOR ATTEMPTING HARDCORE UNLOCK BUT GETTING SOFTCORE RESPONSE
  const auto duration = std::chrono::system_clock::now().time_since_epoch();
  const auto epochSeconds =
      std::chrono::duration_cast<std::chrono::seconds>(duration).count();
  auto json = nlohmann::json::parse(response);

  if (json.contains("Score") && json["Score"].is_number()) {
    m_cache.setUserScore(username, json["Score"], true);
  }

  if (json.contains("SoftcoreScore") && json["SoftcoreScore"].is_number()) {
    m_cache.setUserScore(username, json["SoftcoreScore"], false);
  }

  if (json.contains("AchievementID") && json["AchievementID"].is_number()) {
    if (auto id = json["AchievementID"];
        id != UNSUPPORTED_EMULATOR_ACHIEVEMENT_ID) {
      if (!m_cache.markAchievementUnlocked(username, id, hardcore,
                                           epochSeconds)) {
        spdlog::warn("Failed to mark achievement unlocked: {}",
                     static_cast<int>(id));
      }
    }
  }
}
} // namespace firelight::achievements
