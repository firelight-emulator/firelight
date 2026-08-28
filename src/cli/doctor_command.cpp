#include "cli/doctor_command.hpp"

#include "cli/data_dirs.hpp"
#include "db/database_inspector.hpp"
#include "libretro/core_registry.hpp"

#include <firelight/library/accepted_extensions.hpp>
#include <firelight/platforms/platform_service.hpp>

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

std::string joinNames(const std::vector<std::string> &names, const std::string &separator) {
  std::string joined;

  for (const auto &name : names) {
    joined += (joined.empty() ? "" : separator) + name;
  }

  return joined;
}

/** System files the user has to supply themselves, and the extras a core can use if they do. */
void checkBios(std::vector<Finding> &findings, const DataDirs &dirs) {
  auto &registry = CoreRegistry::instance();
  registry.setSystemDirectory(dirs.coreSystemPath.toStdString());

  for (const auto platformId : registry.platformsWithBios()) {
    for (const auto &status : registry.biosStatusForPlatform(platformId)) {
      const auto &requirement = status.requirement;
      const auto perRegion = requirement.necessity == BiosNecessity::PerRegion;

      std::string gate;
      if (!requirement.coreOption.empty()) {
        gate = " (needs " + requirement.coreOption +
               (requirement.coreOptionValue.empty() ? " turned on" : " = " + requirement.coreOptionValue) + ")";
      }

      if (requirement.necessity == BiosNecessity::Optional) {
        const auto level = status.presentFiles.empty() ? Level::Info : Level::Ok;
        const auto state = status.presentFiles.empty() ? " is not installed" : " is installed";
        findings.push_back({level, "bios", requirement.description + state + " — optional" + gate});
        continue;
      }

      if (!status.isSatisfied) {
        // A core picks its region's file and never falls back, so joining these with "or" would
        // read as any one of them doing the whole job
        const auto detail =
            perRegion && requirement.filenames.size() > 1
                ? " — nothing will boot; it takes one per region: " + joinNames(requirement.filenames, ", ")
                : " (" + joinNames(requirement.filenames, " or ") + ") — those games won't boot";

        // Warn rather than Fail: these are the user's files, and only the platforms they own
        // matter
        findings.push_back({Level::Warn, "bios", requirement.description + " is missing" + detail});
        continue;
      }

      // Owning one region's copy boots that region's games and no others, so the gap is worth
      // naming even though nothing is blocked
      if (!status.missingFiles.empty()) {
        findings.push_back({Level::Info, "bios",
                            requirement.description + ": have " + joinNames(status.presentFiles, ", ") +
                                ", so games needing " + joinNames(status.missingFiles, " or ") + " won't boot"});
        continue;
      }

      findings.push_back({Level::Ok, "bios", requirement.description});
    }
  }
}

/** Formats a shipped core can open that Firelight does not take files in for. */
void checkFormatGaps(std::vector<Finding> &findings) {
  const platforms::PlatformService platformService;
  const auto accepted = library::acceptedExtensions(platformService);

  for (const auto &core : CoreRegistry::instance().cores()) {
    std::vector<std::string> unaccepted;

    for (const auto &extension : core.fileExtensions) {
      if (accepted.count(extension) == 0) {
        unaccepted.push_back(extension);
      }
    }

    if (unaccepted.empty()) {
      continue;
    }

    // Info, not a warning: a core reading more than Firelight accepts is work to pick up rather
    // than something broken, and each format needs its own decision about how it hashes
    findings.push_back(
        {Level::Info, "formats", core.displayName + " opens " + joinNames(unaccepted, ", ") + " - not scanned for"});
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

  require("settings", "friendly settings and core-option defaults are unavailable");

  // A present but empty folder loads nothing, which reads the same to the user as a missing one
  if (std::error_code ec; fs::is_directory(appDir + "/system/settings", ec)) {
    auto hasAny = false;

    for (const auto &entry : fs::directory_iterator(appDir + "/system/settings", ec)) {
      if (entry.path().extension() == ".json") {
        hasAny = true;
        break;
      }
    }

    if (!hasAny) {
      findings.push_back({Level::Fail, "data", "settings holds no .json files — no settings are declared"});
    }
  }
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
  checkBios(findings, dirs);
  checkFormatGaps(findings);
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
