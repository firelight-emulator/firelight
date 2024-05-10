#include "ra_client.hpp"

#include "../libretro/core.hpp"
#include "../manager_accessor.hpp"
#include "cpr/cpr.h"
#include "rcheevos/rc_consoles.h"
#include <QThreadPool>
#include <cstdio>
#include <spdlog/spdlog.h>

namespace firelight::achievements {

static RAClient *theClient;
static ::libretro::Core *theCore;

static void eventHandler(const rc_client_event_t *event, rc_client_t *client) {
  printf("Event! (%d)\n", event->type);
}

static uint32_t readMemoryCallback(uint32_t address, uint8_t *buffer,
                                   uint32_t num_bytes, rc_client_t *client) {
  if (theClient && theCore) {
    if (!theClient->m_memorySeemsGood) {
      const auto valid = rc_libretro_memory_init(
          &theClient->m_memoryRegions, nullptr,
          [](unsigned id, rc_libretro_core_memory_info_t *info) {
            info->data =
                static_cast<unsigned char *>(theCore->getMemoryData(id));
            info->size = theCore->getMemorySize(id);
          },
          RC_CONSOLE_NINTENDO_64);

      if (valid) {
        printf("Memory seems good!\n");
        theClient->m_memorySeemsGood = true;
      } else {

        printf("Memory seems badddddd!\n");
      }
    }

    return rc_libretro_memory_read(&theClient->m_memoryRegions, address, buffer,
                                   num_bytes);
  }
  // spdlog::info("read memory: {}", QThread::currentThreadId());
  return uint32_t(0);
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
  spdlog::info("{}", message);
}

RAClient::RAClient() {
  theClient = this;

  m_client = rc_client_create(readMemoryCallback, httpCallback);
  rc_client_enable_logging(m_client, RC_CLIENT_LOG_LEVEL_VERBOSE, logCallback);
  rc_client_set_event_handler(m_client, eventHandler);

  rc_client_set_hardcore_enabled(m_client, 0);

  m_idleTimer.setInterval(1000);
  connect(&m_idleTimer, &QTimer::timeout, this, [this] {
    printf("Calling idle\n");
    rc_client_idle(m_client);
  });
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

  // QMetaObject::invokeMethod(
  //     this, "loadAchievements", Qt::QueuedConnection,
  //     Q_ARG(QString, QString::fromStdString(contentId)));

  auto p = std::promise<bool>();

  rc_client_begin_load_game(
      m_client, contentMd5.toStdString().c_str(),
      [](int result, const char *error_message, rc_client_t *client,
         void *userdata) {
        // auto promise = static_cast<std::promise<bool> *>(userdata);
        const auto theThing = static_cast<RAClient *>(userdata);
        printf("Result: %d: %s\n", result, error_message);
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
      this);
}
void RAClient::unloadGame() {
  rc_client_unload_game(m_client);
  m_gameLoaded = false;
  emit gameUnloaded();
}

void RAClient::doFrame(::libretro::Core *core,
                       const db::LibraryEntry &currentEntry) {
  if (!theCore) {
    theCore = core;
  }
  m_frameNumber++;

  if (m_frameNumber == 5) {
    QMetaObject::invokeMethod(
        this, "loadGame", Qt::QueuedConnection,
        Q_ARG(QString, QString::fromStdString(currentEntry.contentId)));
  } else if (m_frameNumber > 5) {
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
        if (error_message != nullptr) {
          printf("We got an error: %s\n", error_message);
        }
        if (result == 0) {
          const auto userInfo = rc_client_get_user_info(client);
          const auto raClient = static_cast<RAClient *>(userdata);

          raClient->m_displayName = QString::fromUtf8(userInfo->display_name);
          raClient->m_loggedIn = true;
          emit raClient->loginSucceeded();
          emit raClient->loginStatusChanged();
        } else {
          printf("Error (%d): %s\n", result, error_message);
        }
      },
      this);
}

void RAClient::logout() {
  rc_client_logout(m_client);
  m_loggedIn = false;
  emit loginStatusChanged();
}

void RAClient::logInUserWithToken(const std::string &username,
                                  const std::string &token) {}

void RAClient::initializeMemory(::libretro::Core *core) { theCore = core; }

bool RAClient::loggedIn() const { return m_loggedIn; }
QString RAClient::displayName() { return m_displayName; }
} // namespace firelight::achievements