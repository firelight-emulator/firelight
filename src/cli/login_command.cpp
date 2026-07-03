#include "cli/login_command.hpp"

#include "cli/data_dirs.hpp"

#include <cstdio>
#include <iostream>
#include <string>

#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>
#include <QSettings>
#include <QString>
#include <QTimer>

#include <firelight/achievement_service.hpp>
#include <rcheevos/ra_client.hpp>
#include <rcheevos/rcheevos_offline_client.hpp>
#include <sqlite_achievement_repository.hpp>

namespace firelight::cli {

namespace {
// Login can take a network round trip; give up (as a failure) after this.
constexpr int kLoginTimeoutMs = 30000;

std::string promptLine(const char *prompt) {
  std::fputs(prompt, stderr);
  std::fflush(stderr);
  std::string line;
  std::getline(std::cin, line);
  return line;
}
} // namespace

int runLogin(int argc, char **argv, const CliOptions &opts) {
  QCoreApplication app(argc, argv);

  const auto dirs = resolveDataDirs(opts);
  QDir().mkpath(dirs.appDataPath);
  // RAClient persists the login token via QSettings; point it at the same INI
  // the GUI reads (organization/application names are set by the caller).
  QSettings::setPath(QSettings::Format::IniFormat, QSettings::Scope::UserScope,
                     dirs.appDataPath);

  QString username = QString::fromStdString(opts.raUsername);
  if (username.isEmpty()) {
    username = QString::fromStdString(promptLine("RetroAchievements username: "));
  }
  if (username.isEmpty()) {
    std::fprintf(stderr, "Error: a username is required to log in.\n");
    return 2;
  }

  const bool useToken = !opts.raToken.empty();
  QString password = QString::fromStdString(opts.raPassword);
  if (!useToken && password.isEmpty()) {
    password = QString::fromStdString(promptLine("Password: "));
  }

  firelight::achievements::SqliteAchievementRepository achievementRepo(
      (dirs.appDataPath + "/rcheevos3.db").toStdString());
  firelight::achievements::AchievementService achievementService(achievementRepo);
  firelight::achievements::RetroAchievementsOfflineClient offlineRaClient(
      achievementService);
  firelight::achievements::RAClient raClient(offlineRaClient, achievementService);

  QEventLoop loop;
  bool done = false;
  int result = 1;

  QObject::connect(&raClient, &firelight::achievements::RAClient::loginSucceeded,
                   &loop, [&] {
                     result = 0;
                     done = true;
                     loop.quit();
                   });
  const auto fail = [&](const char *why) {
    std::fprintf(stderr, "Login failed: %s\n", why);
    result = 1;
    done = true;
    loop.quit();
  };
  QObject::connect(
      &raClient,
      &firelight::achievements::RAClient::loginFailedWithInvalidCredentials,
      &loop, [&] { fail("invalid credentials"); });
  QObject::connect(
      &raClient, &firelight::achievements::RAClient::loginFailedWithExpiredToken,
      &loop, [&] { fail("expired token"); });
  QObject::connect(
      &raClient, &firelight::achievements::RAClient::loginFailedWithAccessDenied,
      &loop, [&] { fail("access denied"); });
  QObject::connect(
      &raClient, &firelight::achievements::RAClient::loginFailedWithInternalError,
      &loop, [&] { fail("internal error"); });

  // Defer the login into the event loop so we don't miss a synchronous result.
  QTimer::singleShot(0, &raClient, [&] {
    if (useToken) {
      raClient.logInUserWithToken(username, QString::fromStdString(opts.raToken));
    } else {
      raClient.logInUserWithPassword(username, password);
    }
  });
  QTimer::singleShot(kLoginTimeoutMs, &loop, [&] {
    if (!done) {
      fail("timed out");
    }
  });

  if (!done) {
    loop.exec();
  }

  if (result == 0) {
    std::printf("Logged in to RetroAchievements as %s.\n",
                username.toStdString().c_str());
  }
  return result;
}

} // namespace firelight::cli
