#include "ra_client.hpp"

#include "../libretro/core.hpp"
#include "../manager_accessor.hpp"
#include "rcheevos/rc_consoles.h"
#include <QThreadPool>
#include <algorithm>
#include "cpr/cpr.h"
#include <cstdio>
#include <QJsonDocument>
#include <QJsonObject>
#include <spdlog/spdlog.h>

#include "RegularHttpClient.hpp"
#include "firelight/achievement.hpp"

#include "../../gui/achievement_list_model.hpp"
#include "../../gui/achievement_list_sort_filter_model.hpp"
#include "cache/ra_cache_dumb.hpp"

namespace firelight::achievements {
  static ::libretro::Core *theCore;

  static void eventHandler(const rc_client_event_t *event, rc_client_t *client) {
    auto raClient = static_cast<RAClient *>(rc_client_get_userdata(client));

    switch (event->type) {
      case RC_CLIENT_EVENT_ACHIEVEMENT_TRIGGERED: {
        char urlBuffer[256];

        auto success = rc_client_achievement_get_image_url(
          event->achievement, RC_CLIENT_ACHIEVEMENT_STATE_UNLOCKED, urlBuffer,
          256);
        emit raClient->achievementUnlocked(
          QString(urlBuffer), QString(event->achievement->title),
          QString(event->achievement->description));
        emit raClient->pointsChanged();
        break;
      }
      case RC_CLIENT_EVENT_ACHIEVEMENT_CHALLENGE_INDICATOR_SHOW: {
        char urlBuffer[256];

        auto success = rc_client_achievement_get_image_url(
          event->achievement, RC_CLIENT_ACHIEVEMENT_STATE_UNLOCKED, urlBuffer,
          256);
        emit raClient->showChallengeIndicator(
          event->achievement->id, QString(urlBuffer),
          QString(event->achievement->title),
          QString(event->achievement->description));
        break;
      }
      case RC_CLIENT_EVENT_ACHIEVEMENT_CHALLENGE_INDICATOR_HIDE: {
        emit raClient->hideChallengeIndicator(event->achievement->id);
        break;
      }
      case RC_CLIENT_EVENT_ACHIEVEMENT_PROGRESS_INDICATOR_SHOW:
      case RC_CLIENT_EVENT_ACHIEVEMENT_PROGRESS_INDICATOR_UPDATE:
        if (event->achievement->measured_progress[0] != '\0') {
          string progress(event->achievement->measured_progress);

          if (progress.find('/') != string::npos) {
            const auto slashPos = progress.find('/');
            auto current = progress.substr(0, slashPos);
            auto desired = progress.substr(slashPos + 1);

            erase(current, ' ');
            erase(desired, ' ');

            if (std::ranges::all_of(current, isdigit) &&
                std::ranges::all_of(desired, isdigit)) {
              char urlBuffer[256];

              auto success = rc_client_achievement_get_image_url(
                event->achievement, event->achievement->unlocked, urlBuffer, 256);

              emit raClient->achievementProgressUpdated(
                QString(urlBuffer), event->achievement->id,
                event->achievement->title, event->achievement->description,
                std::stoi(current), std::stoi(desired));
            }
          }
        }
        emit raClient->achievementProgressPercentUpdated(
          event->achievement->id, event->achievement->measured_percent);

        break;
      // Intentionally ignored as we dynamically update the same popup and set out own duration.
      case RC_CLIENT_EVENT_ACHIEVEMENT_PROGRESS_INDICATOR_HIDE:
      case RC_CLIENT_EVENT_GAME_COMPLETED:
      case RC_CLIENT_EVENT_LEADERBOARD_STARTED:
      case RC_CLIENT_EVENT_LEADERBOARD_FAILED:
      case RC_CLIENT_EVENT_LEADERBOARD_SUBMITTED:
      case RC_CLIENT_EVENT_LEADERBOARD_TRACKER_SHOW:
      case RC_CLIENT_EVENT_LEADERBOARD_TRACKER_UPDATE:
      case RC_CLIENT_EVENT_LEADERBOARD_TRACKER_HIDE:
      case RC_CLIENT_EVENT_LEADERBOARD_SCOREBOARD:
      case RC_CLIENT_EVENT_RESET:
      case RC_CLIENT_EVENT_SERVER_ERROR:
      case RC_CLIENT_EVENT_DISCONNECTED:
      case RC_CLIENT_EVENT_RECONNECTED:
        break;
      default:
        printf("Unhandled event! (%d)\n", event->type);
    }
  }

  static uint32_t readMemoryCallback(uint32_t address, uint8_t *buffer,
                                     uint32_t num_bytes, rc_client_t *client) {
    const auto raClient = static_cast<RAClient *>(rc_client_get_userdata(client));
    if (raClient && theCore) {
      if (!raClient->m_memorySeemsGood) {
        const auto valid = rc_libretro_memory_init(
          &raClient->m_memoryRegions, theCore->getMemoryMap(),
          [](unsigned id, rc_libretro_core_memory_info_t *info) {
            info->data =
                static_cast<unsigned char *>(theCore->getMemoryData(id));
            info->size = theCore->getMemorySize(id);
          },
          raClient->m_consoleId);

        if (valid) {
          raClient->m_memorySeemsGood = true;
        }
      }

      if (raClient->m_memorySeemsGood) {
        return rc_libretro_memory_read(&raClient->m_memoryRegions, address,
                                       buffer, num_bytes);
      }
      return 0;
    }

    return 0;
  }

  static void httpCallback(const rc_api_request_t *request,
                           rc_client_server_callback_t callback,
                           void *callback_data, rc_client_t *client) {
    const std::string url = request->url;
    const std::string postData = request->post_data;
    const std::string contentType = request->content_type;

    QThreadPool::globalInstance()->start([=] {
      if (!postData.empty()) {
        const auto raClient =
            static_cast<RAClient *>(rc_client_get_userdata(client));

        const auto response = raClient->m_httpClient->sendRequest(url, postData, contentType);

        if (callback) {
          callback(&response, callback_data);
        }
      }
    });
  }

  static void logCallback(const char *message, const rc_client_t *client) {
    spdlog::info("[RetroAchievements] {}", message);
  }

  RAClient::RAClient(db::IContentDatabase &contentDb) : m_contentDb(contentDb) {
    m_client = rc_client_create(readMemoryCallback, httpCallback);
    rc_client_enable_logging(m_client, RC_CLIENT_LOG_LEVEL_VERBOSE, logCallback);
    rc_client_set_event_handler(m_client, eventHandler);
    rc_client_set_userdata(m_client, this);

    m_cache = std::make_shared<DumbAchievementCache>();
    m_httpClient = std::make_unique<RegularHttpClient>(m_cache);

    m_idleTimer.setInterval(2000);
    connect(&m_idleTimer, &QTimer::timeout, this,
            [this] { rc_client_idle(m_client); });

    m_settings = std::make_unique<QSettings>();

    m_progressNotificationsEnabled =
        m_settings->value("retroachievements/progressNotificationsEnabled", true)
        .toBool();
    m_unlockNotificationsEnabled =
        m_settings->value("retroachievements/unlockNotificationsEnabled", true)
        .toBool();
    m_challengeIndicatorsEnabled =
        m_settings->value("retroachievements/challengeIndicatorsEnabled", true)
        .toBool();

    m_defaultToHardcore =
        m_settings->value("retroachievements/defaultToHardcore", true).toBool();
    rc_client_set_hardcore_enabled(m_client, m_defaultToHardcore);

    const auto user =
        m_settings->value("retroachievements/username", "").toString();
    const auto token =
        m_settings->value("retroachievements/token", "").toString();

    if (!user.isEmpty() && !token.isEmpty()) {
      logInUserWithToken(user, token);
    }
  }

  RAClient::~RAClient() {
    QMetaObject::invokeMethod(&m_idleTimer, "stop", Qt::QueuedConnection);
    if (m_client) {
      rc_client_destroy(m_client);
      m_client = nullptr;
    }
  }

  void RAClient::loadGame(int platformId, const QString &contentMd5) {
    if (!m_loggedIn) {
      return;
    }

    switch (platformId) {
      case 1:
        m_consoleId = RC_CONSOLE_GAMEBOY;
        break;
      case 2:
        m_consoleId = RC_CONSOLE_GAMEBOY_COLOR;
        break;
      case 3:
        m_consoleId = RC_CONSOLE_GAMEBOY_ADVANCE;
        break;
      case 4:
        m_consoleId = RC_CONSOLE_VIRTUAL_BOY;
        break;
      case 5:
        m_consoleId = RC_CONSOLE_NINTENDO;
        break;
      case 6:
        m_consoleId = RC_CONSOLE_SUPER_NINTENDO;
        break;
      case 7:
        m_consoleId = RC_CONSOLE_NINTENDO_64;
        break;
      case 10:
        m_consoleId = RC_CONSOLE_NINTENDO_DS;
        break;
      case 12:
        m_consoleId = RC_CONSOLE_MASTER_SYSTEM;
        break;
      case 13:
        m_consoleId = RC_CONSOLE_MEGA_DRIVE;
        break;
      case 14:
        m_consoleId = RC_CONSOLE_GAME_GEAR;
        break;
      case 15:
        m_consoleId = RC_CONSOLE_SATURN;
        break;
      case 16:
        m_consoleId = RC_CONSOLE_SEGA_32X;
        break;
      case 17:
        m_consoleId = RC_CONSOLE_SEGA_CD;
        break;
      case 18:
        m_consoleId = RC_CONSOLE_PLAYSTATION;
        break;
      case 19:
        m_consoleId = RC_CONSOLE_PLAYSTATION_2;
        break;
      case 20:
        m_consoleId = RC_CONSOLE_PSP;
        break;
      default:
        m_consoleId = RC_CONSOLE_UNKNOWN;
    }

    rc_client_begin_load_game(
      m_client, contentMd5.toStdString().c_str(),
      [](int result, const char *error_message, rc_client_t *client,
         void *userdata) {
        const auto theThing =
            static_cast<RAClient *>(rc_client_get_userdata(client));
        if (result == 0) {
          theThing->m_gameLoaded = true;
          rc_client_user_game_summary_t gameSummary;
          rc_client_get_user_game_summary(client, &gameSummary);

          auto numEarned = gameSummary.num_unlocked_achievements;
          auto numTotal = gameSummary.num_core_achievements;

          char buffer[256];

          auto gameInfo = rc_client_get_game_info(client);
          rc_client_game_get_image_url(gameInfo, buffer, 256);
          emit theThing->gameLoadSucceeded(buffer, QString(gameInfo->title),
                                           numEarned, numTotal);
        } else {
          theThing->m_gameLoaded = false;
          emit theThing->gameLoadFailed();
        }
      },
      nullptr);
  }

  void RAClient::unloadGame() {
    rc_client_unload_game(m_client);
    m_gameLoaded = false;
    m_frameNumber = 0;
    m_memorySeemsGood = false;
    theCore = nullptr;
    QMetaObject::invokeMethod(&m_idleTimer, "stop", Qt::QueuedConnection);
    emit gameUnloaded();
  }

  void RAClient::doFrame(::libretro::Core *core,
                         const db::LibraryEntry &currentEntry) {
    if (!m_loggedIn) {
      return;
    }

    if (!theCore) {
      theCore = core;
    }

    if (m_gameLoaded) {
      rc_client_do_frame(m_client);
      QMetaObject::invokeMethod(&m_idleTimer, "start", Qt::QueuedConnection);
    }
  }

  void RAClient::setDefaultToHardcore(const bool hardcore) {
    m_defaultToHardcore = hardcore;
    rc_client_set_hardcore_enabled(m_client, hardcore);
    m_settings->setValue("retroachievements/defaultToHardcore", hardcore);
  }

  void RAClient::setUnlockNotificationsEnabled(const bool enabled) {
    m_unlockNotificationsEnabled = enabled;
    m_settings->setValue("retroachievements/unlockNotificationsEnabled", enabled);
  }

  void RAClient::setProgressNotificationsEnabled(bool enabled) {
    m_progressNotificationsEnabled = enabled;
    m_settings->setValue("retroachievements/progressNotificationsEnabled",
                         enabled);
  }

  void RAClient::setChallengeIndicatorsEnabled(bool enabled) {
    m_challengeIndicatorsEnabled = enabled;
    m_settings->setValue("retroachievements/challengeIndicatorsEnabled", enabled);
  }

  QAbstractItemModel *RAClient::getAchievementsModelForGameId(int gameId) {
    if (!m_loggedIn) {
      return {};
    }

    const auto id = m_contentDb.getRetroAchievementsIdForGame(gameId);
    if (!id.has_value()) {
      printf("No value\n");
      return {};
    }

    gui::AchievementListSortFilterModel *sortModel = nullptr;
    gui::AchievementListModel *listModel = nullptr;
    if (!m_achievementModels.contains(gameId)) {
      sortModel = new gui::AchievementListSortFilterModel(this);

      listModel = new gui::AchievementListModel(this);
      sortModel->setSourceModel(listModel);

      m_achievementModels.insert(gameId, std::shared_ptr<gui::AchievementListSortFilterModel>(sortModel));
    } else {
      sortModel = m_achievementModels.value(gameId).get();
      listModel = dynamic_cast<gui::AchievementListModel *>(sortModel->sourceModel());
    }

    QThreadPool::globalInstance()->start([&, listModel, gameId, id] {
      const auto url = cpr::Url{"https://retroachievements.org/API/API_GetGameInfoAndUserProgress.php"};
      const auto headers = cpr::Header{
        {"User-Agent", "FirelightEmulator/1.0"},
        {"Content-Type", "application/json"}
      };

      const auto userInfo = rc_client_get_user_info(m_client);

      cpr::Parameters params = {
        {"g", std::to_string(*id)},
        {"z", userInfo->username},
        {"y", "5wMVoSCizWMtXXYJHx5ZpJhCttTuQk84"},
        {"u", userInfo->username}
      };


      const auto response = Post(url, headers, params);

      if (response.status_code != 200) {
        spdlog::error("[RetroAchievements] Failed to get achievements for game ID: {}",
                      gameId);
        return;
      }

      auto json = QJsonDocument::fromJson(QByteArray::fromStdString(response.text)).object();
      if (json.isEmpty()) {
        printf("json is empty\n");
        return;
      }
      printf("GOT JSON\n");

      auto achievements = json["Achievements"].toObject();

      QVector<Achievement> results;
      for (auto val = achievements.begin(); val != achievements.end(); ++val) {
        auto jsonObj = val.value().toObject();

        Achievement achievement;
        achievement.id = jsonObj["ID"].toInt();
        achievement.name = jsonObj["Title"].toString().toStdString();
        achievement.description = jsonObj["Description"].toString().toStdString();
        achievement.points = jsonObj["Points"].toInt();
        achievement.earnedDate = jsonObj["DateEarned"].toString().toStdString();
        achievement.earnedDateHardcore = jsonObj["DateEarnedHardcore"].toString().toStdString();
        achievement.displayOrder = jsonObj["DisplayOrder"].toInt();

        QString imageUrl;
        if (jsonObj.contains("DateEarnedHardcore") && !jsonObj["DateEarnedHardcore"].toString().isEmpty()) {
          achievement.earned = true;
          imageUrl = "https://media.retroachievements.org/Badge/" + jsonObj["BadgeName"].toString() + ".png";
        } else {
          achievement.earned = false;
          imageUrl = "https://media.retroachievements.org/Badge/" + jsonObj["BadgeName"].toString() + "_lock.png";
        }

        achievement.imageUrl = imageUrl.toStdString();

        results.append(achievement);
      }

      std::ranges::sort(results, [](const Achievement &a, const Achievement &b) {
        return a.displayOrder < b.displayOrder;
      });

      QMetaObject::invokeMethod(listModel, "refreshAchievements", Qt::QueuedConnection,
                                Q_ARG(QVector<Achievement>, results));
    });

    return sortModel;
  }

  void RAClient::getAchievementsOverview(int gameId) {
    if (!m_loggedIn) {
      return;
    }

    const auto id = m_contentDb.getRetroAchievementsIdForGame(gameId);
    if (!id.has_value()) {
      return;
    }

    QThreadPool::globalInstance()->start([this, id, gameId] {
      const auto url = cpr::Url{"https://retroachievements.org/API/API_GetUserProgress.php"};
      const auto headers = cpr::Header{
        {"User-Agent", "FirelightEmulator/1.0"},
        {"Content-Type", "application/json"}
      };

      const auto userInfo = rc_client_get_user_info(m_client);

      cpr::Parameters params = {
        {"i", std::to_string(*id)},
        {"z", userInfo->username},
        {"y", "5wMVoSCizWMtXXYJHx5ZpJhCttTuQk84"},
        {"u", userInfo->username}
      };

      const auto response = Post(url, headers, params);

      if (response.status_code != 200) {
        spdlog::error("[RetroAchievements] Failed to get achievements for game ID: {}",
                      gameId);
        return;
      }


      auto json = QJsonDocument::fromJson(QByteArray::fromStdString(response.text)).object();
      if (json.isEmpty()) {
        printf("json is empty\n");
        return;
      }

      if (json.keys().length() != 1) {
        printf("what the\n");
        return;
      }

      emit achievementSummaryAvailable(json.value(json.keys().at(0)).toObject());
    });
  }

  // bool RAClient::gameLoaded() const { return m_gameLoaded; }

  void RAClient::logInUserWithPassword(const QString &username,
                                       const QString &password) {
    m_expectToBeLoggedIn = true;
    rc_client_begin_login_with_password(
      m_client, username.toStdString().c_str(), password.toStdString().c_str(),
      [](int result, const char *error_message, rc_client_t *client,
         void *userdata) {
        const auto userInfo = rc_client_get_user_info(client);
        const auto raClient =
            static_cast<RAClient *>(rc_client_get_userdata(client));

        switch (result) {
          case RC_OK:
            raClient->m_displayName = QString::fromUtf8(userInfo->display_name);
            raClient->m_loggedIn = true;
            emit raClient->loginSucceeded();
            emit raClient->loginStatusChanged();
            emit raClient->pointsChanged();

            raClient->m_settings->setValue("retroachievements/username",
                                           QString(userInfo->username));
            raClient->m_settings->setValue("retroachievements/token",
                                           QString(userInfo->token));
            break;
          case RC_INVALID_CREDENTIALS:
            emit raClient->loginFailedWithInvalidCredentials();
            raClient->m_loggedIn = false;
            break;
          case RC_EXPIRED_TOKEN:
            emit raClient->loginFailedWithExpiredToken();
            raClient->m_loggedIn = false;
            break;
          case RC_ACCESS_DENIED:
            emit raClient->loginFailedWithAccessDenied();
            raClient->m_loggedIn = false;
            break;
          default:
            emit raClient->loginFailedWithInternalError();
            raClient->m_loggedIn = false;
        }
      },
      nullptr);
  }

  void RAClient::logout() {
    m_expectToBeLoggedIn = false;
    rc_client_logout(m_client);
    m_loggedIn = false;
    m_settings->remove("retroachievements/username");
    m_settings->remove("retroachievements/token");

    emit loginStatusChanged();
  }

  void RAClient::logInUserWithToken(const QString &username,
                                    const QString &token) {
    m_expectToBeLoggedIn = true;
    rc_client_begin_login_with_token(
      m_client, username.toStdString().c_str(), token.toStdString().c_str(),
      [](int result, const char *error_message, rc_client_t *client,
         void *userdata) {
        const auto userInfo = rc_client_get_user_info(client);
        const auto raClient =
            static_cast<RAClient *>(rc_client_get_userdata(client));

        switch (result) {
          case RC_OK:
            raClient->m_displayName = QString::fromUtf8(userInfo->display_name);
            raClient->m_loggedIn = true;
            emit raClient->loginSucceeded();
            emit raClient->loginStatusChanged();
            emit raClient->pointsChanged();
            break;
          case RC_INVALID_CREDENTIALS:
            emit raClient->loginFailedWithInvalidCredentials();
            raClient->m_loggedIn = false;
            break;
          case RC_EXPIRED_TOKEN:
            emit raClient->loginFailedWithExpiredToken();
            raClient->m_loggedIn = false;
            break;
          case RC_ACCESS_DENIED:
            emit raClient->loginFailedWithAccessDenied();
            raClient->m_loggedIn = false;
            break;
          default:
            emit raClient->loginFailedWithInternalError();
            raClient->m_loggedIn = false;
        }
      },
      nullptr);
  }

  int RAClient::numPoints() const {
    const auto userInfo = rc_client_get_user_info(m_client);
    if (userInfo == nullptr) {
      return 0;
    }

    return userInfo->score_softcore;
  }

  QString RAClient::avatarUrl() const {
    const auto userInfo = rc_client_get_user_info(m_client);
    if (userInfo == nullptr) {
      return {};
    }

    if (char buf[255];
      rc_client_user_get_image_url(userInfo, buf, 255) == RC_OK) {
      return {buf};
    }

    return {};
  }

  bool RAClient::expectToBeLoggedIn() const {
    return m_expectToBeLoggedIn;
  }
} // namespace firelight::achievements
