#include "cli/list_command.hpp"

#include "libretro/core_registry.hpp"

#include <firelight/settings/setting_definition.hpp>
#include <firelight/settings/settings_catalog.hpp>

#include <QCoreApplication>
#include <cstdio>
#include <string>

namespace firelight::cli {

namespace {

const char *typeName(firelight::settings::SettingType type) {
  switch (type) {
  case firelight::settings::SettingType::BOOLEAN:
    return "boolean";
  case firelight::settings::SettingType::OPTIONS:
    return "options";
  case firelight::settings::SettingType::INTEGER:
    return "integer";
  case firelight::settings::SettingType::STRING:
    return "string";
  case firelight::settings::SettingType::CUSTOM:
    return "custom";
  }
  return "unknown";
}

void printSetting(const firelight::settings::SettingDefinition &s) {
  std::printf("  %-28s %-8s default=%s", s.key.c_str(), typeName(s.type),
              s.defaultValue.empty() ? "\"\"" : s.defaultValue.c_str());
  if (s.type == firelight::settings::SettingType::OPTIONS && !s.options.empty()) {
    std::string values;
    for (const auto &opt : s.options) {
      if (!values.empty()) {
        values += "|";
      }
      values += opt.value;
    }
    std::printf("  values=%s", values.c_str());
  } else if (s.type == firelight::settings::SettingType::INTEGER) {
    std::printf("  range=%g..%g", s.minValue, s.maxValue);
  }
  std::printf("\n");
}

bool loadCatalog() {
  const auto path = (QCoreApplication::applicationDirPath() + "/system/settings_catalog.json").toStdString();
  return firelight::settings::SettingsCatalog::instance().loadFromFile(path);
}

} // namespace

int runListSettings(int argc, char **argv) {
  QCoreApplication app(argc, argv);
  if (!loadCatalog()) {
    std::fprintf(stderr, "Warning: could not load the settings catalog; only "
                         "core-declared defaults would apply.\n");
  }

  const auto &catalog = firelight::settings::SettingsCatalog::instance();

  if (!catalog.appSettings().empty()) {
    std::printf("App settings (single-valued; --set does not apply to these):\n");
    for (const auto &s : catalog.appSettings()) {
      printSetting(s);
    }
    std::printf("\n");
  }

  std::printf("Common settings (apply to every core):\n");
  for (const auto &s : catalog.commonSettings()) {
    printSetting(s);
  }

  for (const auto &core : CoreRegistry::instance().cores()) {
    const auto &settings = catalog.coreSpecificSettings(core.id);
    if (settings.empty()) {
      continue;
    }
    std::printf("\n%s (%s):\n", core.displayName.c_str(), core.id.c_str());
    for (const auto &s : settings) {
      printSetting(s);
    }
  }

  std::printf("\nUse: firelight --set KEY=VALUE ...  (raw core option keys are "
              "also accepted)\n");
  return 0;
}

int runListCores(int argc, char **argv) {
  QCoreApplication app(argc, argv);

  std::printf("Known cores:\n");
  for (const auto &core : CoreRegistry::instance().cores()) {
    std::string platforms;
    for (const auto id : core.supportedPlatformIds) {
      if (!platforms.empty()) {
        platforms += ", ";
      }
      platforms += std::to_string(id);
    }
    std::printf("  %-30s %-20s platforms=[%s]\n", core.id.c_str(), core.displayName.c_str(), platforms.c_str());
  }
  std::printf("\nUse: firelight --core <id> <rom>\n");
  return 0;
}

} // namespace firelight::cli
