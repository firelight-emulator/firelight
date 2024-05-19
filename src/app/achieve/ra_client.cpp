#include "ra_client.hpp"

#include "../libretro/core.hpp"
#include "../manager_accessor.hpp"
#include "cpr/cpr.h"
#include "rcheevos/rc_consoles.h"
#include <QThreadPool>
#include <algorithm>
#include <cstdio>
#include <spdlog/spdlog.h>

namespace firelight::achievements {

static ::libretro::Core *theCore;

static void eventHandler(const rc_client_event_t *event, rc_client_t *client) {
  auto raClient = static_cast<RAClient *>(rc_client_get_userdata(client));

  switch (event->type) {
  case RC_CLIENT_EVENT_ACHIEVEMENT_TRIGGERED:
    emit raClient->achievementUnlocked(
        QString(event->achievement->title),
        QString(event->achievement->description));
    emit raClient->pointsChanged();
    break;
  case RC_CLIENT_EVENT_ACHIEVEMENT_CHALLENGE_INDICATOR_SHOW:
    printf("Challenge show: %s: %s\n", event->achievement->title,
           event->achievement->description);
    break;
  case RC_CLIENT_EVENT_ACHIEVEMENT_CHALLENGE_INDICATOR_HIDE:
    printf("Challenge hide: %s: %s\n", event->achievement->title,
           event->achievement->description);
    break;
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
    raClient->achievementProgressPercentUpdated(
        event->achievement->id, event->achievement->measured_percent);

    break;
  case RC_CLIENT_EVENT_ACHIEVEMENT_PROGRESS_INDICATOR_HIDE:
    // printf("progress: %s\n", event->achievement->measured_progress);
    break;
  case RC_CLIENT_EVENT_GAME_COMPLETED:
    break;
  case RC_CLIENT_EVENT_LEADERBOARD_STARTED:
    break;
  case RC_CLIENT_EVENT_LEADERBOARD_FAILED:
    break;
  case RC_CLIENT_EVENT_LEADERBOARD_SUBMITTED:
    break;
  case RC_CLIENT_EVENT_LEADERBOARD_TRACKER_SHOW:
    break;
  case RC_CLIENT_EVENT_LEADERBOARD_TRACKER_UPDATE:
    break;
  case RC_CLIENT_EVENT_LEADERBOARD_TRACKER_HIDE:
    break;
  case RC_CLIENT_EVENT_LEADERBOARD_SCOREBOARD:
    break;
  case RC_CLIENT_EVENT_RESET:
    break;
  case RC_CLIENT_EVENT_SERVER_ERROR:
    break;
  case RC_CLIENT_EVENT_DISCONNECTED:
    break;
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
          RC_CONSOLE_SUPER_NINTENDO);

      if (valid) {
        raClient->m_memorySeemsGood = true;
      }
    }

    if (raClient->m_memorySeemsGood) {
      return rc_libretro_memory_read(&raClient->m_memoryRegions, address, buffer,
                                     num_bytes);
    }
    return 0;
  }

  return 0;
}

static void httpCallback(const rc_api_request_t *request,
                         rc_client_server_callback_t callback,
                         void *callback_data, rc_client_t *client) {
  const auto url = cpr::Url{std::string(request->url)};
  const auto headers = cpr::Header{{"User-Agent", "FirelightEmulator/1.0"},
                                   {"Content-Type", request->content_type}};
  const auto postData = cpr::Body{request->post_data};

  QThreadPool::globalInstance()->start([=] {
    if (postData != "") {
      const auto response = Post(url, headers, postData);

      rc_api_server_response_t rcResponse;
      if (response.error) {
        printf("NOOO GOT AN ERROR: %s\n", response.error.message.c_str());
      } else {
        rcResponse.body = response.text.c_str();
        rcResponse.body_length = response.text.size();
      }

      rcResponse.http_status_code = response.status_code;

      if (callback) {
        callback(&rcResponse, callback_data);
      }
    }
  });
}

static void logCallback(const char *message, const rc_client_t *client) {
  spdlog::info("[RetroAchievements] {}", message);
}

RAClient::RAClient() {
  m_client = rc_client_create(readMemoryCallback, httpCallback);
  rc_client_enable_logging(m_client, RC_CLIENT_LOG_LEVEL_VERBOSE, logCallback);
  rc_client_set_event_handler(m_client, eventHandler);
  rc_client_set_userdata(m_client, this);

  rc_client_set_hardcore_enabled(m_client, 0);

  m_idleTimer.setInterval(2000);
  connect(&m_idleTimer, &QTimer::timeout, this,
          [this] { rc_client_idle(m_client); });

  m_settings = std::make_unique<QSettings>();
  const auto user =
      m_settings->value("retroachievements/username", "").toString();
  const auto token =
      m_settings->value("retroachievements/token", "").toString();

  if (!user.isEmpty() && !token.isEmpty()) {
    logInUserWithToken(user, token);
  }
}

RAClient::~RAClient() {
  if (m_client) {
    rc_client_destroy(m_client);
    m_client = nullptr;
  }
}

void RAClient::loadGame(const QString &contentMd5) {
  if (!m_loggedIn) {
    emit gameLoadSucceeded();
    return;
  }

  rc_client_begin_load_game(
      m_client, contentMd5.toStdString().c_str(),
      [](int result, const char *error_message, rc_client_t *client,
         void *userdata) {
        // auto promise = static_cast<std::promise<bool> *>(userdata);
        const auto theThing =
            static_cast<RAClient *>(rc_client_get_userdata(client));
        if (result == 0) {
          // promise->set_value(true);
          theThing->m_gameLoaded = true;
          emit theThing->gameLoadSucceeded();
        } else {
          // promise->set_value(false);
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

  if (m_frameNumber < 6) {
    m_frameNumber++;
  } else if (m_frameNumber == 6) {
    m_frameNumber++;
    QMetaObject::invokeMethod(
        this, "loadGame", Qt::QueuedConnection,
        Q_ARG(QString, QString::fromStdString(currentEntry.contentId)));
  } else if (m_frameNumber > 2) {
    rc_client_do_frame(m_client);
    QMetaObject::invokeMethod(&m_idleTimer, "start", Qt::QueuedConnection);
  }
}
// bool RAClient::gameLoaded() const { return m_gameLoaded; }

void RAClient::logInUserWithPassword(const QString &username,
                                     const QString &password) {
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
  rc_client_logout(m_client);
  m_loggedIn = false;
  emit loginStatusChanged();
}

void RAClient::logInUserWithToken(const QString &username,
                                  const QString &token) {
  rc_client_begin_login_with_token(
      m_client, username.toStdString().c_str(), token.toStdString().c_str(),
      [](int result, const char *error_message, rc_client_t *client,
         void *userdata) {
        const auto userInfo = rc_client_get_user_info(client);
        const auto raClient =
            static_cast<RAClient *>(rc_client_get_userdata(client));

        switch (result) {
        case RC_OK:
          raClient->m_displayName =
          QString::fromUtf8(userInfo->display_name); raClient->m_loggedIn =
          true; emit raClient->loginSucceeded(); emit
          raClient->loginStatusChanged(); emit raClient->pointsChanged();
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

bool RAClient::loggedIn() const { return m_loggedIn; }
QString RAClient::displayName() { return m_displayName; }
int RAClient::numPoints() const {
  const auto userInfo = rc_client_get_user_info(m_client);
  if (userInfo == nullptr) {
    return 0;
  }

  return userInfo->score_softcore;
}
} // namespace firelight::achievements