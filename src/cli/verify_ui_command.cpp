#include "cli/verify_ui_command.hpp"

#include "app/qml_message_log.hpp"

#include <QAbstractItemModel>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>
#include <iostream>

namespace firelight::cli {

namespace {

constexpr int POLL_INTERVAL_MS = 50;
// Incubation is asynchronous, so a route that reports "not loading" immediately
// after navigating has not started building yet
constexpr int MINIMUM_DWELL_MS = 300;
constexpr int EXIT_ROUTES_FAILED = 5;

// The settings screen parses its own tail, so its sections are probed directly
constexpr std::array SETTINGS_SECTIONS = {
    "appearance", "system", "emulation", "controllers", "notifications", "captures", "retroachievements", "about",
};

// Routes that only resolve while a game is running. The sweep never launches
// one, so it reports these rather than pretending to check them
constexpr std::array GAMEPLAY_ROUTES = {"/quick-menu"};

// TODO
/** Whether this route needs a running game to mount */
bool needsGame(const QString &path) {
  for (const auto *route : GAMEPLAY_ROUTES) {
    if (path == QLatin1String(route)) {
      return true;
    }
  }

  return false;
}

// TODO
/** Pulls the first entry id out of a library model, or -1 when it is empty */
int firstEntryId(QQmlApplicationEngine &engine) {
  const auto value = engine.rootContext()->contextProperty("LibraryEntryModel");
  auto *model = value.value<QAbstractItemModel *>();
  if (model == nullptr || model->rowCount() == 0) {
    return -1;
  }

  const auto roles = model->roleNames();
  for (auto it = roles.constBegin(); it != roles.constEnd(); ++it) {
    if (it.value() == "id" || it.value() == "entryId") {
      const auto id = model->data(model->index(0, 0), it.key());
      if (id.isValid()) {
        return id.toInt();
      }
    }
  }

  return -1;
}

// TODO
/** Drives the route sweep off the event loop and exits when it is done */
class Sweeper : public QObject {
public:
  Sweeper(QQmlApplicationEngine &engine, QObject *rootObject, std::vector<QString> routes, const int settleMs,
          const bool json, QObject *parent)
      : QObject(parent), m_engine(engine), m_rootObject(rootObject), m_routes(std::move(routes)), m_settleMs(settleMs),
        m_json(json) {
    m_router = engine.singletonInstance<QObject *>("QMLFirelight", "Router");
    m_routeView = rootObject != nullptr ? rootObject->findChild<QObject *>("RouteView") : nullptr;
    m_realEntryId = firstEntryId(engine);
  }

  void run() {
    if (m_router == nullptr || m_routeView == nullptr) {
      std::cerr << "verify-ui: could not reach Router or RouteView; is the shell loaded?\n";
      QCoreApplication::exit(EXIT_ROUTES_FAILED);
      return;
    }

    next();
  }

private:
  // TODO
  /** Substitutes a real library id into a path parameter where one is needed */
  QString resolve(const QString &path, bool &syntheticOut) {
    if (!path.contains(':')) {
      return path;
    }

    syntheticOut = m_realEntryId < 0;
    const auto id = m_realEntryId >= 0 ? QString::number(m_realEntryId) : QStringLiteral("1");

    auto resolved = path;
    resolved.replace(QStringLiteral(":entryId"), id);
    resolved.replace(QStringLiteral(":modId"), id);
    resolved.replace(QStringLiteral(":playerNumber"), QStringLiteral("1"));
    resolved.replace(QStringLiteral(":gameContentHash"), QStringLiteral("0"));
    return resolved;
  }

  void next() {
    if (m_index >= static_cast<int>(m_routes.size())) {
      report();
      return;
    }

    bool synthetic = false;
    m_currentPath = resolve(m_routes[m_index], synthetic);
    m_currentSynthetic = synthetic;

    gui::QmlMessageLog::setCurrentRoute(m_currentPath);
    QMetaObject::invokeMethod(m_router, "navigate", Q_ARG(QVariant, QVariant(m_currentPath)));

    m_elapsed.restart();
    QTimer::singleShot(POLL_INTERVAL_MS, this, &Sweeper::poll);
  }

  void poll() {
    const auto waited = m_elapsed.elapsed();
    const auto loading = m_routeView->property("loading").toBool();

    if (waited < MINIMUM_DWELL_MS || (loading && waited < m_settleMs)) {
      QTimer::singleShot(POLL_INTERVAL_MS, this, &Sweeper::poll);
      return;
    }

    evaluate(loading, waited);
  }

  void evaluate(const bool stillLoading, const qint64 waited) {
    VerifyUiRunner::RouteResult result;
    result.path = m_currentPath;
    result.synthetic = m_currentSynthetic;

    for (const auto &entry : gui::QmlMessageLog::getEntriesForRoute(m_currentPath)) {
      if (entry.fatal) {
        ++result.fatalMessages;
        if (result.failure.isEmpty()) {
          result.failure = entry.text;
        }
      }
    }

    if (needsGame(m_currentPath)) {
      result.skipped = true;
      result.mounted = true;
      result.failure = QStringLiteral("requires a running game");
    } else if (stillLoading) {
      result.failure = QStringLiteral("still building after %1ms").arg(waited);
    } else if (m_router->property("overlay").toBool()) {
      auto *loader = m_rootObject->findChild<QObject *>("RouteOverlayLoader");
      const auto *item = loader != nullptr ? loader->property("item").value<QObject *>() : nullptr;
      result.mounted = item != nullptr;
      if (!result.mounted && result.failure.isEmpty()) {
        result.failure = QStringLiteral("overlay did not load");
      }
    } else {
      const auto *item = m_routeView->property("currentItem").value<QObject *>();
      const auto name = item != nullptr ? item->objectName() : QString();
      result.mounted = item != nullptr && name != QStringLiteral("RouteNotFound");
      if (!result.mounted && result.failure.isEmpty()) {
        result.failure = name == QStringLiteral("RouteNotFound") ? QStringLiteral("no component for this route")
                                                                 : QStringLiteral("nothing mounted");
      }
    }

    m_results.push_back(result);
    ++m_index;
    next();
  }

  void report() {
    gui::QmlMessageLog::setCurrentRoute(QString());

    int failed = 0;
    int skipped = 0;
    for (const auto &result : m_results) {
      if (result.skipped) {
        ++skipped;
      } else if (!result.mounted || result.fatalMessages > 0) {
        ++failed;
      }
    }

    if (m_json) {
      QJsonArray routes;
      for (const auto &result : m_results) {
        routes.append(QJsonObject{{"path", result.path},
                                  {"mounted", result.mounted},
                                  {"skipped", result.skipped},
                                  {"syntheticParams", result.synthetic},
                                  {"fatalMessages", result.fatalMessages},
                                  {"failure", result.failure}});
      }

      const QJsonObject root{
          {"total", static_cast<int>(m_results.size())}, {"failed", failed}, {"skipped", skipped}, {"routes", routes}};
      std::cout << QJsonDocument(root).toJson(QJsonDocument::Indented).toStdString() << std::endl;
    } else {
      std::cout << "\nverify-ui: " << m_results.size() << " route(s), " << failed << " failed, " << skipped
                << " skipped\n";
      for (const auto &result : m_results) {
        const auto *status = result.skipped                                  ? "  skip "
                             : (result.mounted && result.fatalMessages == 0) ? "  ok   "
                                                                             : "  FAIL ";
        std::cout << status << result.path.toStdString();
        if (result.synthetic) {
          std::cout << "  [synthetic params - no library entry to use]";
        }
        if (!result.failure.isEmpty()) {
          std::cout << "\n           " << result.failure.toStdString();
        }
        std::cout << "\n";
      }
      std::cout << std::flush;
    }

    QCoreApplication::exit(failed > 0 ? EXIT_ROUTES_FAILED : 0);
  }

  QQmlApplicationEngine &m_engine;
  QObject *m_rootObject = nullptr;
  QObject *m_router = nullptr;
  QObject *m_routeView = nullptr;
  std::vector<QString> m_routes;
  std::vector<VerifyUiRunner::RouteResult> m_results;
  QElapsedTimer m_elapsed;
  QString m_currentPath;
  int m_settleMs = 1500;
  int m_index = 0;
  int m_realEntryId = -1;
  bool m_json = false;
  bool m_currentSynthetic = false;
};

} // namespace

std::vector<QString> VerifyUiRunner::getDefaultRoutes() {
  std::vector<QString> routes = {
      QStringLiteral("/library"),
      QStringLiteral("/library/entries/:entryId"),
      QStringLiteral("/shop"),
      QStringLiteral("/shop/mods/:modId"),
      QStringLiteral("/controllers"),
      QStringLiteral("/controllers/profiles/:playerNumber"),
      QStringLiteral("/controllers/manage"),
      QStringLiteral("/gallery"),
      QStringLiteral("/gallery/games/:gameContentHash"),
      QStringLiteral("/activity"),
      QStringLiteral("/netplay"),
      QStringLiteral("/help"),
      QStringLiteral("/dev/gallery"),
      QStringLiteral("/quick-menu"),
      QStringLiteral("/settings"),
  };

  for (const auto *section : SETTINGS_SECTIONS) {
    routes.push_back(QStringLiteral("/settings/") + QLatin1String(section));
  }

  return routes;
}

void VerifyUiRunner::start(QQmlApplicationEngine &engine, QObject *rootObject, const CliOptions &options) {
  auto routes = options.verifyRoutes.empty() ? getDefaultRoutes() : std::vector<QString>();
  for (const auto &route : options.verifyRoutes) {
    routes.push_back(QString::fromStdString(route));
  }

  auto *sweeper = new Sweeper(engine, rootObject, std::move(routes), options.settleMs, options.json, &engine);
  QTimer::singleShot(0, sweeper, &Sweeper::run);
}

} // namespace firelight::cli
