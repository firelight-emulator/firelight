#include <firelight/settings/sqlite_settings_repository.hpp>

#include <QApplication>
#include <QTest>
#include <QtQuickTest>
#include <firelight/saves/sqlite_save_database.hpp>
#include <gtest/gtest.h>
#include <service_accessor.hpp>
#include <firelight/saves/save_manager_impl.hpp>
#include <firelight/settings/settings_catalog.hpp>
#include <firelight/settings/settings_service.hpp>
#include <firelight/settings/sqlite_settings_repository.hpp>

int main(int argc, char **argv) {
  QApplication app(argc, argv);

  firelight::saves::SqliteSaveDatabase userdata(":memory:");
  firelight::ServiceAccessor::setSaveManager(new firelight::saves::SaveManager(
      std::filesystem::temp_directory_path().string(), userdata));

  firelight::settings::SqliteSettingsRepository settings(":memory:");
  auto settingsService =
      std::make_shared<firelight::settings::SettingsService>(settings);
  firelight::settings::SettingsService::setInstance(settingsService.get());

  // Load the friendly-settings catalog so emulation-default resolution matches
  // the app (the catalog is the single source of truth for those defaults).
  // Resolve relative to the executable, not the cwd, so it works however the
  // test binary is launched.
  const auto catalogPath =
      QCoreApplication::applicationDirPath() + "/system/settings_catalog.json";
  firelight::settings::SettingsCatalog::instance().loadFromFile(
      catalogPath.toStdString());

  ::testing::InitGoogleTest(&argc, argv);
  auto result = RUN_ALL_TESTS();

  return result;
}
