#include "settings/sqlite_emulation_settings_manager.hpp"

#include <QGuiApplication>
#include <QTest>
#include <QtQuickTest>
#include <db/sqlite_userdata_database.hpp>
#include <gtest/gtest.h>
#include <manager_accessor.hpp>

int main(int argc, char **argv) {
  QGuiApplication app(argc, argv);

  firelight::db::SqliteUserdataDatabase userdata(":memory:");
  firelight::gui::GameImageProvider gameImageProvider;
  firelight::ManagerAccessor::setSaveManager(new firelight::saves::SaveManager(
      QString::fromStdString(std::filesystem::temp_directory_path().string()),
      userdata, gameImageProvider));

  firelight::settings::SqliteEmulationSettingsManager settings(":memory:");
  auto emulatorConfigManager =
      std::make_shared<EmulatorConfigManager>(userdata);
  firelight::ManagerAccessor::setEmulatorConfigManager(emulatorConfigManager);

  ::testing::InitGoogleTest(&argc, argv);
  auto result = RUN_ALL_TESTS();
  //
  // QQuickView view;
  // view.setSource(QUrl("qrc:/tests.qml"));
  // view.show();

  // Enter the Qt event loop to run QML tests
  // QGuiApplication::exec();
  // QUICK_TEST_MAIN(example);

  return result;
}
