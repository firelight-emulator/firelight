#include "sqlite_controller_repository.hpp"

#include "platforms/platform_service.hpp"
#include "profiles/shortcut_mapping.hpp"

#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <spdlog/spdlog.h>
#include <utility>

constexpr auto DATABASE_PREFIX = "controllers_";

namespace firelight::input {
SqliteControllerRepository::SqliteControllerRepository(QString dbFilePath)
    : m_dbFilePath(std::move(dbFilePath)) {
  const auto db = getDatabase();

  QSqlQuery createProfiles2(db);
  createProfiles2.prepare("CREATE TABLE IF NOT EXISTS profilesv2("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                          "name TEXT NOT NULL UNIQUE,"
                          "created_at INTEGER NOT NULL);");

  if (!createProfiles2.exec()) {
    spdlog::error("Table creation failed: {}",
                  createProfiles2.lastError().text().toStdString());
  }

  QSqlQuery createMappings2(db);
  createMappings2.prepare("CREATE TABLE IF NOT EXISTS mappingsv2("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                          "profile_id INTEGER NOT NULL,"
                          "platform_id INTEGER NOT NULL,"
                          "controller_type INTEGER NOT NULL,"
                          "mapping_data TEXT,"
                          "created_at INTEGER NOT NULL, "
                          "UNIQUE(profile_id, platform_id, controller_type));");

  if (!createMappings2.exec()) {
    spdlog::error("Table creation failed: {}",
                  createMappings2.lastError().text().toStdString());
  }

  QSqlQuery createShortcuts(db);
  createShortcuts.prepare("CREATE TABLE IF NOT EXISTS shortcutsv2("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                          "profile_id INTEGER NOT NULL UNIQUE,"
                          "mapping_data TEXT,"
                          "created_at INTEGER NOT NULL);");

  if (!createShortcuts.exec()) {
    spdlog::error("Table creation failed: {}",
                  createShortcuts.lastError().text().toStdString());
  }

  QSqlQuery deviceInfo(db);
  deviceInfo.prepare(
      "CREATE TABLE IF NOT EXISTS devicesv2("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "device_name TEXT NOT NULL,"
      "vendor_id INTEGER NOT NULL,"
      "product_id INTEGER NOT NULL,"
      "product_version INTEGER NOT NULL,"
      "display_name TEXT,"
      "active_profile_id INTEGER NOT NULL,"
      "created_at INTEGER NOT NULL, "
      "UNIQUE(device_name, vendor_id, product_id, product_version));");

  if (!deviceInfo.exec()) {
    spdlog::error("Table creation failed: {}",
                  deviceInfo.lastError().text().toStdString());
  }

  QSqlQuery createProfiles(db);
  createProfiles.prepare("CREATE TABLE IF NOT EXISTS profilesv1("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                         "controller_name TEXT NOT NULL,"
                         "vendor_id INTEGER NOT NULL,"
                         "product_id INTEGER NOT NULL,"
                         "product_version_id INTEGER NOT NULL,"
                         "created_at INTEGER NOT NULL);");

  if (!createProfiles.exec()) {
    spdlog::error("Table creation failed: {}",
                  createProfiles.lastError().text().toStdString());
  }

  QSqlQuery createMappings(db);
  createMappings.prepare("CREATE TABLE IF NOT EXISTS mappingsv1("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                         "profile_id INTEGER NOT NULL,"
                         "platform_id INTEGER NOT NULL,"
                         "mapping_data TEXT,"
                         "created_at INTEGER NOT NULL);");

  if (!createMappings.exec()) {
    spdlog::error("Table creation failed: {}",
                  createMappings.lastError().text().toStdString());
  }

  // QSqlQuery createHotkeys(db);
  // createHotkeys.prepare("CREATE TABLE IF NOT EXISTS hotkeysv1("
  //                       "id INTEGER PRIMARY KEY AUTOINCREMENT,"
  //                       "profile_id INTEGER NOT NULL,"
  //                       "hotkey_mappings TEXT,"
  //                       "created_at INTEGER NOT NULL);");
  //
  // if (!createHotkeys.exec()) {
  //   spdlog::error("Table creation failed: {}",
  //                 createHotkeys.lastError().text().toStdString());
  // }
}

std::optional<DeviceInfo>
SqliteControllerRepository::getDeviceInfo(DeviceIdentifier identifier) const {
  QSqlQuery query(getDatabase());
  query.prepare(
      "SELECT * FROM devicesv2 WHERE vendor_id = :vendorId AND "
      "product_id = :productId AND product_version = :productVersion");
  query.bindValue(":vendorId", identifier.vendorId);
  query.bindValue(":productId", identifier.productId);
  query.bindValue(":productVersion", identifier.productVersion);

  if (!query.exec() || !query.next()) {
    spdlog::debug("Device info not found for: {}", identifier.deviceName);
    return std::nullopt;
  }

  return DeviceInfo{
      .displayName = query.value("display_name").toString().toStdString(),
      .profileId = query.value("active_profile_id").toInt(),
  };
}

void SqliteControllerRepository::updateDeviceInfo(DeviceIdentifier identifier,
                                                  const DeviceInfo &info) {
  QSqlQuery query(getDatabase());
  query.prepare(
      "INSERT OR REPLACE INTO devicesv2 (device_name, vendor_id, product_id, "
      "product_version, display_name, active_profile_id, created_at) "
      "VALUES (:deviceName, :vendorId, :productId, :productVersion, "
      ":displayName, :activeProfileId, :createdAt)");
  query.bindValue(":deviceName", QString::fromStdString(identifier.deviceName));
  query.bindValue(":vendorId", identifier.vendorId);
  query.bindValue(":productId", identifier.productId);
  query.bindValue(":productVersion", identifier.productVersion);
  query.bindValue(":displayName", QString::fromStdString(info.displayName));
  query.bindValue(":activeProfileId", info.profileId);
  query.bindValue(":createdAt", QDateTime::currentMSecsSinceEpoch());

  if (!query.exec()) {
    spdlog::error("Failed to update device info: {}",
                  query.lastError().text().toStdString());
  } else {
    spdlog::debug("Updated device info for: {}", identifier.deviceName);
  }
}

std::shared_ptr<GamepadProfile>
SqliteControllerRepository::createProfile(const std::string name) {
  QSqlQuery query(getDatabase());
  query.prepare(
      "INSERT INTO profilesv2 (name, created_at) VALUES (:name, :createdAt)");
  query.bindValue(":name", QString::fromStdString(name));
  query.bindValue(":createdAt",
                  QVariant::fromValue(QDateTime::currentMSecsSinceEpoch()));

  if (!query.exec()) {
    spdlog::error("Failed to create new profile: {}",
                  query.lastError().text().toStdString());
    return nullptr;
  }

  auto profile = std::make_shared<GamepadProfile>(query.lastInsertId().toInt());
  profile->setName(name);

  for (const auto &platform :
       platforms::PlatformService::getInstance().listPlatforms()) {
    for (const auto &controller : platform.controllerTypes) {
      auto mapping =
          getOrCreateMapping(profile->getId(), platform.id, controller.id);
      if (!mapping) {
        spdlog::error("Failed to create or retrieve mapping for profile ID {} "
                      "on platform {} with controller type {}",
                      profile->getId(), platform.id, controller.id);
        continue;
      }
      profile->addMapping(mapping);
    }
  }

  QSqlQuery shortcutQuery(getDatabase());
  shortcutQuery.prepare(
      "SELECT * FROM shortcutsv2 WHERE profile_id = :profileId");
  shortcutQuery.bindValue(":profileId", profile->getId());

  if (shortcutQuery.exec() && shortcutQuery.next()) {
    auto mappingData =
        shortcutQuery.value("mapping_data").toString().toStdString();
    auto shortcutMapping = std::make_shared<ShortcutMapping>(
        [id = profile->getId(), this](const ShortcutMapping &mapping) {
          QSqlQuery updateQuery(getDatabase());
          updateQuery.prepare(
              "UPDATE shortcutsv2 SET mapping_data = :mappingData WHERE "
              "profile_id = :profileId");
          updateQuery.bindValue(":mappingData",
                                QString::fromStdString(mapping.serialize()));
          updateQuery.bindValue(":profileId", id);

          if (!updateQuery.exec()) {
            spdlog::error("Failed to update shortcuts: {}",
                          updateQuery.lastError().text().toStdString());
          }
        });

    shortcutMapping->deserialize(mappingData);
    profile->setShortcutMapping(shortcutMapping);
    spdlog::info("Loaded shortcuts for profile ID {}", profile->getId());
  } else {
    QSqlQuery createShortcutsQuery(getDatabase());
    createShortcutsQuery.prepare(
        "INSERT INTO shortcutsv2 (profile_id, mapping_data, created_at) "
        "VALUES (:profileId, :mappingData, :createdAt)");
    createShortcutsQuery.bindValue(":profileId", profile->getId());
    createShortcutsQuery.bindValue(":mappingData", QString());
    createShortcutsQuery.bindValue(":createdAt",
                                   QDateTime::currentMSecsSinceEpoch());

    if (!createShortcutsQuery.exec()) {
      spdlog::error("Failed to create shortcuts for profile ID {}: {}",
                    profile->getId(),
                    createShortcutsQuery.lastError().text().toStdString());
    } else {
      spdlog::info("Created empty shortcuts for profile ID {}",
                   profile->getId());
    }

    const auto shortcutMapping = std::make_shared<ShortcutMapping>(
        [id = profile->getId(), this](const ShortcutMapping &mapping) {
          QSqlQuery updateQuery(getDatabase());
          updateQuery.prepare(
              "UPDATE shortcutsv2 SET mapping_data = :mappingData WHERE "
              "profile_id = :profileId");
          updateQuery.bindValue(":mappingData",
                                QString::fromStdString(mapping.serialize()));
          updateQuery.bindValue(":profileId", id);

          if (!updateQuery.exec()) {
            spdlog::error("Failed to update shortcuts: {}",
                          updateQuery.lastError().text().toStdString());
          }
        });
    profile->setShortcutMapping(shortcutMapping);
  }

  m_profiles.emplace_back(profile);

  return profile;
}

std::shared_ptr<GamepadProfile>
SqliteControllerRepository::getProfile(const int id) {
  for (const auto &profile : m_profiles) {
    if (profile->getId() == id) {
      return profile;
    }
  }

  QSqlQuery query(getDatabase());
  query.prepare("SELECT * FROM profilesv2 WHERE id = :id");
  query.bindValue(":id", id);

  if (!query.exec() || !query.next()) {
    spdlog::error("Profile with ID {} not found", id);
    return nullptr;
  }

  auto profile = std::make_shared<GamepadProfile>(query.value("id").toInt());
  profile->setName(query.value("name").toString().toStdString());

  for (const auto &platform :
       platforms::PlatformService::getInstance().listPlatforms()) {
    for (const auto &controller : platform.controllerTypes) {
      auto mapping =
          getOrCreateMapping(profile->getId(), platform.id, controller.id);
      if (!mapping) {
        spdlog::error("Failed to create or retrieve mapping for profile ID {} "
                      "on platform {} with controller type {}",
                      profile->getId(), platform.id, controller.id);
        continue;
      }
      profile->addMapping(mapping);
    }
  }

  QSqlQuery shortcutQuery(getDatabase());
  shortcutQuery.prepare(
      "SELECT * FROM shortcutsv2 WHERE profile_id = :profileId");
  shortcutQuery.bindValue(":profileId", profile->getId());

  if (shortcutQuery.exec() && shortcutQuery.next()) {
    auto mappingData =
        shortcutQuery.value("mapping_data").toString().toStdString();
    auto shortcutMapping = std::make_shared<ShortcutMapping>(
        [id, this](const ShortcutMapping &mapping) {
          QSqlQuery updateQuery(getDatabase());
          updateQuery.prepare(
              "UPDATE shortcutsv2 SET mapping_data = :mappingData WHERE "
              "profile_id = :profileId");
          updateQuery.bindValue(":mappingData",
                                QString::fromStdString(mapping.serialize()));
          updateQuery.bindValue(":profileId", id);

          if (!updateQuery.exec()) {
            spdlog::error("Failed to update shortcuts: {}",
                          updateQuery.lastError().text().toStdString());
          }
        });

    shortcutMapping->deserialize(mappingData);
    profile->setShortcutMapping(shortcutMapping);
    spdlog::info("Loaded shortcuts for profile ID {}", profile->getId());
  } else {
    QSqlQuery createShortcutsQuery(getDatabase());
    createShortcutsQuery.prepare(
        "INSERT INTO shortcutsv2 (profile_id, mapping_data, created_at) "
        "VALUES (:profileId, :mappingData, :createdAt)");
    createShortcutsQuery.bindValue(":profileId", profile->getId());
    createShortcutsQuery.bindValue(":mappingData", QString());
    createShortcutsQuery.bindValue(":createdAt",
                                   QDateTime::currentMSecsSinceEpoch());

    if (!createShortcutsQuery.exec()) {
      spdlog::error("Failed to create shortcuts for profile ID {}: {}",
                    profile->getId(),
                    createShortcutsQuery.lastError().text().toStdString());
    } else {
      spdlog::info("Created empty shortcuts for profile ID {}",
                   profile->getId());
    }

    const auto shortcutMapping = std::make_shared<ShortcutMapping>(
        [id, this](const ShortcutMapping &mapping) {
          QSqlQuery updateQuery(getDatabase());
          updateQuery.prepare(
              "UPDATE shortcutsv2 SET mapping_data = :mappingData WHERE "
              "profile_id = :profileId");
          updateQuery.bindValue(":mappingData",
                                QString::fromStdString(mapping.serialize()));
          updateQuery.bindValue(":profileId", id);

          if (!updateQuery.exec()) {
            spdlog::error("Failed to update shortcuts: {}",
                          updateQuery.lastError().text().toStdString());
          }
        });
    profile->setShortcutMapping(shortcutMapping);
  }

  m_profiles.emplace_back(profile);
  return profile;
}

QSqlDatabase SqliteControllerRepository::getDatabase() const {
  const auto name =
      DATABASE_PREFIX +
      QString::number(reinterpret_cast<quint64>(QThread::currentThread()), 16);
  if (QSqlDatabase::contains(name)) {
    return QSqlDatabase::database(name);
  }

  spdlog::debug("Database connection with name {} does not exist; creating",
                name.toStdString());
  auto db = QSqlDatabase::addDatabase("QSQLITE", name);
  db.setDatabaseName(m_dbFilePath);
  db.open();
  return db;
}

std::shared_ptr<InputMapping> SqliteControllerRepository::getOrCreateMapping(
    int profileId, const int platformId, const int controllerTypeId) {
  for (const auto &mapping : m_inputMappings) {
    if (mapping->getControllerProfileId() == profileId &&
        mapping->getPlatformId() == platformId &&
        mapping->getControllerType() == controllerTypeId) {
      return mapping;
    }
  }

  // If not found, check the database
  QSqlQuery mappingsQuery(getDatabase());
  mappingsQuery.prepare(
      "SELECT * FROM mappingsv2 WHERE profile_id = :profileId AND "
      "platform_id = :platformId AND controller_type = :controllerType");
  mappingsQuery.bindValue(":profileId", profileId);
  mappingsQuery.bindValue(":platformId", platformId);
  mappingsQuery.bindValue(":controllerType", controllerTypeId);

  if (!mappingsQuery.exec()) {
    spdlog::error("Failed to fetch mappings for profile ID {}: {}", profileId,
                  mappingsQuery.lastError().text().toStdString());
    return nullptr;
  }

  if (mappingsQuery.next()) {
    const auto mappingId = mappingsQuery.value("id").toInt();
    auto mapping = std::make_shared<InputMapping>(
        mappingId, profileId, platformId, controllerTypeId,
        [&](InputMapping &m) {
          QSqlQuery updateQuery(getDatabase());
          updateQuery.prepare("UPDATE mappingsv2 SET mapping_data = "
                              ":mappingData WHERE id = :id");
          updateQuery.bindValue(":mappingData",
                                QString::fromStdString(m.serialize()));
          updateQuery.bindValue(":id", m.getId());

          if (!updateQuery.exec()) {
            spdlog::error("Update failed: {}",
                          updateQuery.lastError().text().toStdString());
          }
        });
    mapping->deserialize(
        mappingsQuery.value("mapping_data").toString().toStdString());

    m_inputMappings.emplace_back(mapping);
    return mapping;
  }

  QSqlQuery insertQuery(getDatabase());
  insertQuery.prepare(
      "INSERT INTO mappingsv2 (profile_id, platform_id, controller_type, "
      "created_at) VALUES (:profileId, :platformId, :controllerType, "
      ":createdAt)");
  insertQuery.bindValue(":profileId", profileId);
  insertQuery.bindValue(":platformId", platformId);
  insertQuery.bindValue(":controllerType", controllerTypeId);
  insertQuery.bindValue(":createdAt", QDateTime::currentMSecsSinceEpoch());

  if (!insertQuery.exec()) {
    spdlog::error("Insert failed: {}",
                  insertQuery.lastError().text().toStdString());
    return nullptr;
  }

  auto lastId = insertQuery.lastInsertId().toInt();
  auto mapping = std::make_shared<InputMapping>(
      lastId, profileId, platformId, controllerTypeId, [&](InputMapping &m) {
        QSqlQuery updateQuery(getDatabase());
        updateQuery.prepare("UPDATE mappingsv2 SET mapping_data = "
                            ":mappingData WHERE id = :id");
        updateQuery.bindValue(":mappingData",
                              QString::fromStdString(m.serialize()));
        updateQuery.bindValue(":id", m.getId());

        if (!updateQuery.exec()) {
          spdlog::error("Update failed: {}",
                        updateQuery.lastError().text().toStdString());
        }
      });

  m_inputMappings.emplace_back(mapping);
  return mapping;
}
} // namespace firelight::input
