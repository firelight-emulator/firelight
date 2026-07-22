#include "cli/doctor_command.hpp"

#include "cli/data_dirs.hpp"
#include "db/database_inspector.hpp"
#include "libretro/core_registry.hpp"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <array>
#include <filesystem>
#include <iostream>

namespace firelight::cli {

namespace {

constexpr int EXIT_UNHEALTHY = 6;

// Every database the app opens under the data directory
constexpr std::array DATABASE_FILES = {
    "library.db", "settings.db", "rcheevos3.db", "activity.db", "userdata.db",
    "media.db",   "captures.db", "cheats.db",    "metadata.db", "controllers.db",
};

enum class Level { Ok, Info, Warn, Fail };

struct Finding {
  Level level = Level::Ok;
  std::string section;
  std::string message;
};

const char *labelFor(const Level level) {
  switch (level) {
  case Level::Ok:
    return "ok  ";
  case Level::Info:
    return "info";
  case Level::Warn:
    return "warn";
  default:
    return "FAIL";
  }
}

/** Cores that are registered but missing, and defaults that cannot resolve. */
void checkCores(std::vector<Finding> &findings) {
  for (const auto &core : CoreRegistry::instance().checkAvailability()) {
    if (!core.present) {
      if (!core.defaultForPlatforms.empty()) {
        findings.push_back({Level::Fail, "cores",
                            core.coreId + " is the default core for " +
                                std::to_string(core.defaultForPlatforms.size()) +
                                " platform(s) but is not installed at " + core.expectedPath});
      } else {
        findings.push_back({Level::Warn, "cores", core.coreId + " is registered but not installed"});
      }
      continue;
    }

    if (!core.reachable) {
      findings.push_back({Level::Warn, "cores", core.coreId + " is installed but supports no platforms"});
      continue;
    }

    findings.push_back({Level::Ok, "cores", core.coreId});
  }
}

/** Shipped data files the app degrades silently without. */
void checkShippedData(std::vector<Finding> &findings, const DataDirs &dirs) {
  namespace fs = std::filesystem;
  const auto appDir = QCoreApplication::applicationDirPath().toStdString();

  const auto require = [&](const std::string &relative, const std::string &consequence) {
    const auto path = appDir + "/system/" + relative;
    std::error_code ec;
    if (fs::exists(path, ec)) {
      findings.push_back({Level::Ok, "data", relative});
    } else {
      findings.push_back({Level::Fail, "data", relative + " is missing — " + consequence});
    }
  };

  require("settings_catalog.json", "friendly settings and core-option defaults are unavailable");
  require("shortcuts.json", "no hotkeys will work");

  // PPSSPP is seeded into the data directory on first GUI launch; this marker is
  // what the seeding checks for
  const auto ppssppMarker = dirs.appDataPath.toStdString() + "/core-system/PPSSPP/ppge_atlas.zim";
  std::error_code ec;
  if (fs::exists(ppssppMarker, ec)) {
    findings.push_back({Level::Ok, "data", "PPSSPP assets seeded"});
  } else {
    findings.push_back({Level::Warn, "data", "PPSSPP assets are not seeded — PSP games will fail inside the core"});
  }

  if (fs::exists(appDir + "/system/content.db", ec)) {
    findings.push_back({Level::Info, "data", "content.db is shipped but no code reads it"});
  }
}

/** Health of every database under the data directory. */
void checkDatabases(std::vector<Finding> &findings, const DataDirs &dirs) {
  const auto base = dirs.appDataPath.toStdString();

  for (const auto *name : DATABASE_FILES) {
    const auto info = db::inspect(base + "/" + std::string(name));

    if (!info.exists) {
      // Databases are created on demand, so absence is only notable for the one
      // that is meant to arrive populated
      const auto level = std::string(name) == "metadata.db" ? Level::Warn : Level::Info;
      const auto note = std::string(name) == "metadata.db" ? " is absent — offline metadata is unavailable"
                                                           : " has not been created yet";
      findings.push_back({level, "databases", std::string(name) + note});
      continue;
    }

    if (!info.opens) {
      findings.push_back({Level::Fail, "databases", std::string(name) + " could not be opened: " + info.error});
      continue;
    }

    if (!info.integrityOk) {
      findings.push_back({Level::Fail, "databases", std::string(name) + " failed its integrity check"});
      continue;
    }

    findings.push_back({Level::Ok, "databases",
                        std::string(name) + " (v" + std::to_string(info.userVersion) + ", " +
                            std::to_string(info.tableCount) + " tables)"});
  }
}

} // namespace

int runDoctor(int argc, char **argv, const CliOptions &options) {
  QCoreApplication app(argc, argv);

  const auto dirs = resolveDataDirs(options);

  std::vector<Finding> findings;
  checkCores(findings);
  checkShippedData(findings, dirs);
  checkDatabases(findings, dirs);

  int failures = 0;
  int warnings = 0;
  for (const auto &finding : findings) {
    if (finding.level == Level::Fail) {
      ++failures;
    } else if (finding.level == Level::Warn) {
      ++warnings;
    }
  }

  if (options.json) {
    QJsonArray items;
    for (const auto &finding : findings) {
      items.append(QJsonObject{{"level", QString::fromLatin1(labelFor(finding.level)).trimmed()},
                               {"section", QString::fromStdString(finding.section)},
                               {"message", QString::fromStdString(finding.message)}});
    }

    const QJsonObject root{{"failures", failures}, {"warnings", warnings}, {"findings", items}};
    std::cout << QJsonDocument(root).toJson(QJsonDocument::Indented).toStdString() << std::endl;
  } else {
    std::string section;
    for (const auto &finding : findings) {
      if (finding.section != section) {
        section = finding.section;
        std::cout << "\n" << section << "\n";
      }
      std::cout << "  " << labelFor(finding.level) << "  " << finding.message << "\n";
    }
    std::cout << "\ndoctor: " << failures << " failure(s), " << warnings << " warning(s)\n" << std::flush;
  }

  if (failures > 0) {
    return EXIT_UNHEALTHY;
  }

  return options.strict && warnings > 0 ? EXIT_UNHEALTHY : 0;
}

} // namespace firelight::cli
