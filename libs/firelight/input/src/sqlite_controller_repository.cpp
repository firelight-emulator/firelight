#include <firelight/input/sqlite_controller_repository.hpp>
#include <firelight/input/shortcut_mapping.hpp>
#include <firelight/input/shortcut_registry.hpp>
#include <firelight/migrations/migration_runner.hpp>
#include <firelight/platforms/platform_service.hpp>

#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <utility>

constexpr auto DATABASE_PREFIX = "controllers_";

namespace firelight::input {
  namespace {
    std::string serializeAnalog(const AnalogSettings &settings) {
      return nlohmann::json(settings).dump();
    }

    AnalogSettings deserializeAnalog(const std::string &data) {
      if (data.empty()) {
        return {};
      }
      const auto j = nlohmann::json::parse(data, nullptr, false);
      if (j.is_discarded()) {
        return {};
      }
      return j.get<AnalogSettings>();
    }
  } // namespace

  SqliteControllerRepository::SqliteControllerRepository(
    QString dbFilePath, platforms::PlatformService &platformService)
    : m_dbFilePath(std::move(dbFilePath)), m_platformService(platformService) {
    auto db = getDatabase();

    const auto exec = [&db](const char *sql) {
      QSqlQuery query(db);
      query.prepare(sql);
      if (!query.exec()) {
        spdlog::error("Table creation failed: {}",
                      query.lastError().text().toStdString());
      }
    };

    // Forward-only schema migrations, stamped into PRAGMA user_version. A future
    // schema change adds the next-numbered migration instead of renaming tables
    // (the old `_v3`-suffix approach).
    const auto createSchemaV1 = [&exec] {
      exec("CREATE TABLE IF NOT EXISTS profiles("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL UNIQUE,"
        "kind INTEGER NOT NULL DEFAULT 0,"
        "builtin INTEGER NOT NULL DEFAULT 0,"
        "based_on_type INTEGER NOT NULL DEFAULT -1,"
        "icon TEXT,"
        "analog_json TEXT,"
        "created_at INTEGER NOT NULL,"
        "updated_at INTEGER NOT NULL);");

      exec("CREATE TABLE IF NOT EXISTS mappings("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "profile_id INTEGER NOT NULL,"
        "platform_id INTEGER NOT NULL,"
        "controller_type INTEGER NOT NULL,"
        "mapping_data TEXT,"
        "created_at INTEGER NOT NULL,"
        "UNIQUE(profile_id, platform_id, controller_type));");

      exec("CREATE TABLE IF NOT EXISTS shortcuts("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "profile_id INTEGER NOT NULL UNIQUE,"
        "mapping_data TEXT,"
        "created_at INTEGER NOT NULL);");

      exec("CREATE TABLE IF NOT EXISTS devices("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "device_name TEXT,"
        "device_type INTEGER NOT NULL DEFAULT 0,"
        "vendor_id INTEGER NOT NULL,"
        "product_id INTEGER NOT NULL,"
        "product_version INTEGER NOT NULL,"
        "display_name TEXT,"
        "active_profile_id INTEGER NOT NULL,"
        "created_at INTEGER NOT NULL,"
        "UNIQUE(vendor_id, product_id, product_version));");

      exec("CREATE TABLE IF NOT EXISTS game_overrides("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "content_hash TEXT NOT NULL UNIQUE,"
        "profile_id INTEGER NOT NULL,"
        "created_at INTEGER NOT NULL);");

      exec("CREATE TABLE IF NOT EXISTS platform_preferences("
        "platform_id INTEGER PRIMARY KEY,"
        "gamepad_type INTEGER NOT NULL);");
    };

    // The preset a profile's shortcuts were seeded from, so the editor can show
    // which rows have been changed since and reset them back.
    const auto addPresetIdV2 = [&exec] {
      exec("ALTER TABLE profiles ADD COLUMN preset_id TEXT NOT NULL DEFAULT '';");
    };

    const std::vector<migrations::Migration> schema = {{1, createSchemaV1},
                                                       {2, addPresetIdV2}};

    QSqlQuery versionQuery("PRAGMA user_version", db);
    const int currentVersion =
        versionQuery.next() ? versionQuery.value(0).toInt() : 0;

    db.transaction();
    applyMigrations(currentVersion, schema, [&db](const int version) {
      QSqlQuery(db).exec(QString("PRAGMA user_version = %1").arg(version));
    });
    db.commit();
  }

  void SqliteControllerRepository::setPlatformPreferredType(const int platformId,
                                                            const int gamepadType) {
    QSqlQuery query(getDatabase());
    query.prepare("INSERT OR REPLACE INTO platform_preferences (platform_id, "
      "gamepad_type) VALUES (:platformId, :gamepadType)");
    query.bindValue(":platformId", platformId);
    query.bindValue(":gamepadType", gamepadType);
    if (!query.exec()) {
      spdlog::error("Failed to set platform preference for {}: {}", platformId,
                    query.lastError().text().toStdString());
    }
  }

  void SqliteControllerRepository::clearPlatformPreferredType(
    const int platformId) {
    QSqlQuery query(getDatabase());
    query.prepare(
      "DELETE FROM platform_preferences WHERE platform_id = :platformId");
    query.bindValue(":platformId", platformId);
    if (!query.exec()) {
      spdlog::error("Failed to clear platform preference for {}: {}", platformId,
                    query.lastError().text().toStdString());
    }
  }

  std::optional<int>
  SqliteControllerRepository::getPlatformPreferredType(const int platformId) const {
    QSqlQuery query(getDatabase());
    query.prepare("SELECT gamepad_type FROM platform_preferences WHERE "
      "platform_id = :platformId");
    query.bindValue(":platformId", platformId);
    if (!query.exec() || !query.next()) {
      return std::nullopt;
    }
    return query.value("gamepad_type").toInt();
  }

  std::optional<DeviceInfo>
  SqliteControllerRepository::getDeviceInfo(DeviceIdentifier identifier) const {
    QSqlQuery query(getDatabase());
    query.prepare("SELECT * FROM devices WHERE vendor_id = :vendorId AND "
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
      "INSERT OR REPLACE INTO devices (device_name, device_type, vendor_id, "
      "product_id, product_version, display_name, active_profile_id, "
      "created_at) VALUES (:deviceName, :deviceType, :vendorId, :productId, "
      ":productVersion, :displayName, :activeProfileId, :createdAt)");
    query.bindValue(":deviceName", QString::fromStdString(identifier.deviceName));
    query.bindValue(":deviceType", static_cast<int>(identifier.type));
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

  namespace {
    // Copies a preset's shipped bindings into a brand-new profile's mapping.
    //
    // A preset is a starting point, not a tier the engine resolves through: from
    // here on the profile owns a flat mapping, so an action with no binding means
    // "off" rather than "inherit", and resetting a row is a write.
    //
    // Sources the controller hasn't got are copied in too, deliberately. They can
    // never match (the engine only ever sees codes a device actually reports), and
    // a preset lists alternates precisely so the next one takes over — an N64 pad
    // simply never satisfies the L3+R3 entry and lands on Select+Start. Filtering
    // here would need the device, which a profile isn't tied to: profiles are
    // shared between controllers, so one pad's shape must not be baked in.
    void seedShortcuts(ShortcutMapping &mapping, const std::string &presetId,
                       const DeviceType device) {
      const auto *preset = ShortcutRegistry::instance().findPreset(presetId);
      if (!preset) {
        if (!presetId.empty()) {
          spdlog::warn("Profile asks for shortcut preset '{}', which the catalog "
                       "doesn't declare; it starts unbound",
                       presetId);
        }
        return;
      }

      if (!preset->bindings.contains(device)) {
        return;
      }
      for (const auto &[id, sources] : preset->bindings.at(device)) {
        if (!sources.empty()) {
          mapping.setBindings(id, sources);
        }
      }
      // Mutating a mapping doesn't persist it; the caller syncs.
      mapping.sync();
    }
  } // namespace

  void SqliteControllerRepository::loadProfileContents(
    const std::shared_ptr<GamepadProfile> &profile) {
    for (const auto &platform:
         m_platformService.listPlatforms()) {
      for (const auto &controller: platform.controllerTypes) {
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

    const auto syncShortcuts = [id = profile->getId(),
          this](const ShortcutMapping &mapping) {
      QSqlQuery updateQuery(getDatabase());
      updateQuery.prepare("UPDATE shortcuts SET mapping_data = :mappingData "
        "WHERE profile_id = :profileId");
      updateQuery.bindValue(":mappingData",
                            QString::fromStdString(mapping.serialize()));
      updateQuery.bindValue(":profileId", id);
      if (!updateQuery.exec()) {
        spdlog::error("Failed to update shortcuts: {}",
                      updateQuery.lastError().text().toStdString());
      }
    };

    QSqlQuery shortcutQuery(getDatabase());
    shortcutQuery.prepare(
      "SELECT * FROM shortcuts WHERE profile_id = :profileId");
    shortcutQuery.bindValue(":profileId", profile->getId());

    if (shortcutQuery.exec() && shortcutQuery.next()) {
      const auto mappingData =
          shortcutQuery.value("mapping_data").toString().toStdString();
      auto shortcutMapping = std::make_shared<ShortcutMapping>(syncShortcuts);
      shortcutMapping->deserialize(mappingData);
      profile->setShortcutMapping(shortcutMapping);
    } else {
      QSqlQuery createShortcutsQuery(getDatabase());
      createShortcutsQuery.prepare(
        "INSERT INTO shortcuts (profile_id, mapping_data, created_at) "
        "VALUES (:profileId, :mappingData, :createdAt)");
      createShortcutsQuery.bindValue(":profileId", profile->getId());
      createShortcutsQuery.bindValue(":mappingData", QString());
      createShortcutsQuery.bindValue(":createdAt",
                                     QDateTime::currentMSecsSinceEpoch());
      if (!createShortcutsQuery.exec()) {
        spdlog::error("Failed to create shortcuts for profile ID {}: {}",
                      profile->getId(),
                      createShortcutsQuery.lastError().text().toStdString());
      }
      auto shortcutMapping = std::make_shared<ShortcutMapping>(syncShortcuts);
      seedShortcuts(*shortcutMapping, profile->getPresetId(),
                    profile->isKeyboardProfile() ? DeviceType::Keyboard
                                                 : DeviceType::Gamepad);
      profile->setShortcutMapping(shortcutMapping);
    }
  }

  std::shared_ptr<GamepadProfile>
  SqliteControllerRepository::createProfile(const std::string name,
                                            const DeviceType device) {
    const auto now = QDateTime::currentMSecsSinceEpoch();
    const auto presetId = ShortcutRegistry::instance().defaultPresetId();

    QSqlQuery query(getDatabase());
    query.prepare("INSERT INTO profiles (name, kind, builtin, based_on_type, "
      "icon, analog_json, preset_id, created_at, updated_at) VALUES (:name, "
      ":kind, 0, -1, '', :analog, :presetId, :createdAt, :updatedAt)");
    query.bindValue(":name", QString::fromStdString(name));
    query.bindValue(":kind", device == DeviceType::Keyboard ? 1 : 0);
    query.bindValue(":analog",
                    QString::fromStdString(serializeAnalog(AnalogSettings{})));
    query.bindValue(":presetId", QString::fromStdString(presetId));
    query.bindValue(":createdAt", now);
    query.bindValue(":updatedAt", now);

    if (!query.exec()) {
      spdlog::error("Failed to create new profile: {}",
                    query.lastError().text().toStdString());
      return nullptr;
    }

    auto profile = std::make_shared<GamepadProfile>(query.lastInsertId().toInt());
    profile->setName(name);
    // Both are set before loadProfileContents, which seeds the profile's
    // shortcuts from the preset for its kind of device.
    profile->setIsKeyboardProfile(device == DeviceType::Keyboard);
    profile->setPresetId(presetId);
    loadProfileContents(profile);

    m_profiles.emplace_back(profile);
    return profile;
  }

  std::shared_ptr<GamepadProfile>
  SqliteControllerRepository::getProfile(const int id) {
    for (const auto &profile: m_profiles) {
      if (profile->getId() == id) {
        return profile;
      }
    }

    QSqlQuery query(getDatabase());
    query.prepare("SELECT * FROM profiles WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec() || !query.next()) {
      spdlog::error("Profile with ID {} not found", id);
      return nullptr;
    }

    auto profile = std::make_shared<GamepadProfile>(query.value("id").toInt());
    profile->setName(query.value("name").toString().toStdString());
    profile->setIsKeyboardProfile(query.value("kind").toInt() == 1);
    profile->setBuiltin(query.value("builtin").toInt() != 0);
    profile->setBasedOnType(query.value("based_on_type").toInt());
    profile->setIcon(query.value("icon").toString().toStdString());
    profile->setPresetId(query.value("preset_id").toString().toStdString());
    profile->setDefaultAnalogSettings(
      deserializeAnalog(query.value("analog_json").toString().toStdString()));

    loadProfileContents(profile);

    m_profiles.emplace_back(profile);
    return profile;
  }

  std::vector<std::shared_ptr<GamepadProfile> >
  SqliteControllerRepository::listProfiles() {
    std::vector<std::shared_ptr<GamepadProfile> > result;

    QSqlQuery query(getDatabase());
    query.prepare("SELECT id FROM profiles ORDER BY id");
    if (!query.exec()) {
      spdlog::error("Failed to list profiles: {}",
                    query.lastError().text().toStdString());
      return result;
    }

    while (query.next()) {
      if (auto profile = getProfile(query.value("id").toInt())) {
        result.emplace_back(profile);
      }
    }
    return result;
  }

  std::shared_ptr<GamepadProfile>
  SqliteControllerRepository::cloneProfile(const int sourceId,
                                           std::string newName) {
    const auto source = getProfile(sourceId);
    if (!source) {
      spdlog::error("Cannot clone: source profile {} not found", sourceId);
      return nullptr;
    }

    auto clone = createProfile(std::move(newName));
    if (!clone) {
      return nullptr;
    }

    // Copy the profile-wide analog defaults.
    setProfileAnalogSettings(clone->getId(), source->getDefaultAnalogSettings());

    // Copy each platform's bindings (and any per-platform analog override) by
    // round-tripping the serialized mapping.
    for (const auto &platform:
         m_platformService.listPlatforms()) {
      for (const auto &controller: platform.controllerTypes) {
        const auto src =
            source->getMappingForPlatformAndController(platform.id, controller.id);
        auto dst =
            clone->getMappingForPlatformAndController(platform.id, controller.id);
        if (src && dst) {
          dst->deserialize(src->serialize());
          dst->sync();
        }
      }
    }

    // Copy the shortcut mapping.
    if (source->getShortcutMapping() && clone->getShortcutMapping()) {
      clone->getShortcutMapping()->deserialize(
        source->getShortcutMapping()->serialize());
      clone->getShortcutMapping()->sync();
    }

    return clone;
  }

  bool SqliteControllerRepository::deleteProfile(const int id) {
    const auto profile = getProfile(id);
    if (profile && profile->isBuiltin()) {
      spdlog::warn("Refusing to delete built-in profile {}", id);
      return false;
    }

    QSqlQuery query(getDatabase());
    for (const char *sql:
         {
           "DELETE FROM profiles WHERE id = :id",
           "DELETE FROM mappings WHERE profile_id = :id",
           "DELETE FROM shortcuts WHERE profile_id = :id"
         }) {
      query.prepare(sql);
      query.bindValue(":id", id);
      if (!query.exec()) {
        spdlog::error("Failed to delete profile {}: {}", id,
                      query.lastError().text().toStdString());
        return false;
      }
    }

    std::erase_if(m_profiles, [id](const std::shared_ptr<GamepadProfile> &p) {
      return p->getId() == id;
    });
    std::erase_if(m_inputMappings,
                  [id](const std::shared_ptr<InputMapping> &m) {
                    return static_cast<int>(m->getControllerProfileId()) == id;
                  });
    return true;
  }

  bool SqliteControllerRepository::renameProfile(const int id,
                                                 std::string newName) {
    QSqlQuery query(getDatabase());
    query.prepare("UPDATE profiles SET name = :name, updated_at = :updatedAt "
      "WHERE id = :id");
    query.bindValue(":name", QString::fromStdString(newName));
    query.bindValue(":updatedAt", QDateTime::currentMSecsSinceEpoch());
    query.bindValue(":id", id);
    if (!query.exec()) {
      spdlog::error("Failed to rename profile {}: {}", id,
                    query.lastError().text().toStdString());
      return false;
    }

    if (const auto profile = getProfile(id)) {
      profile->setName(std::move(newName));
    }
    return true;
  }

  void SqliteControllerRepository::setProfileAnalogSettings(
    const int profileId, const AnalogSettings &settings) {
    QSqlQuery query(getDatabase());
    query.prepare("UPDATE profiles SET analog_json = :analog, updated_at = "
      ":updatedAt WHERE id = :id");
    query.bindValue(":analog", QString::fromStdString(serializeAnalog(settings)));
    query.bindValue(":updatedAt", QDateTime::currentMSecsSinceEpoch());
    query.bindValue(":id", profileId);
    if (!query.exec()) {
      spdlog::error("Failed to update analog settings for profile {}: {}",
                    profileId, query.lastError().text().toStdString());
      return;
    }

    if (const auto profile = getProfile(profileId)) {
      profile->setDefaultAnalogSettings(settings);
    }
  }

  void SqliteControllerRepository::setProfilePresetId(
    const int profileId, const std::string &presetId) {
    QSqlQuery query(getDatabase());
    query.prepare("UPDATE profiles SET preset_id = :presetId, updated_at = "
      ":updatedAt WHERE id = :id");
    query.bindValue(":presetId", QString::fromStdString(presetId));
    query.bindValue(":updatedAt", QDateTime::currentMSecsSinceEpoch());
    query.bindValue(":id", profileId);
    if (!query.exec()) {
      spdlog::error("Failed to update preset for profile {}: {}", profileId,
                    query.lastError().text().toStdString());
      return;
    }

    if (const auto profile = getProfile(profileId)) {
      profile->setPresetId(presetId);
    }
  }

  std::string SqliteControllerRepository::exportProfile(const int id) {
    const auto profile = getProfile(id);
    if (!profile) {
      return {};
    }

    nlohmann::json mappings = nlohmann::json::array();
    for (const auto &platform:
         m_platformService.listPlatforms()) {
      for (const auto &controller: platform.controllerTypes) {
        const auto mapping =
            profile->getMappingForPlatformAndController(platform.id, controller.id);
        if (!mapping) {
          continue;
        }
        mappings.push_back(
          {
            {"platform_id", platform.id},
            {"controller_type", controller.id},
            {"data", nlohmann::json::parse(mapping->serialize(), nullptr, false)}
          });
      }
    }

    nlohmann::json j;
    j["version"] = 3;
    j["name"] = profile->getName();
    j["analog"] = profile->getDefaultAnalogSettings();
    j["mappings"] = mappings;
    if (profile->getShortcutMapping()) {
      j["shortcuts"] = profile->getShortcutMapping()->serialize();
    }
    return j.dump(2);
  }

  std::shared_ptr<GamepadProfile>
  SqliteControllerRepository::importProfile(const std::string &json) {
    const auto j = nlohmann::json::parse(json, nullptr, false);
    if (j.is_discarded() || !j.is_object()) {
      spdlog::error("Cannot import profile: invalid JSON");
      return nullptr;
    }

    // Ensure the imported name does not collide with an existing one.
    std::string baseName = j.value("name", std::string("Imported Profile"));
    std::string name = baseName;
    std::shared_ptr<GamepadProfile> profile;
    for (int attempt = 1; attempt <= 100; ++attempt) {
      profile = createProfile(name);
      if (profile) {
        break;
      }
      name = baseName + " (" + std::to_string(attempt) + ")";
    }
    if (!profile) {
      return nullptr;
    }

    if (j.contains("analog")) {
      setProfileAnalogSettings(profile->getId(),
                               j.at("analog").get<AnalogSettings>());
    }

    if (j.contains("mappings")) {
      for (const auto &item: j.at("mappings")) {
        const auto platformId = item.value("platform_id", -1);
        const auto controllerType = item.value("controller_type", -1);
        auto dst = profile->getMappingForPlatformAndController(platformId,
                                                               controllerType);
        if (dst && item.contains("data")) {
          dst->deserialize(item.at("data").dump());
          dst->sync();
        }
      }
    }

    if (j.contains("shortcuts") && profile->getShortcutMapping()) {
      profile->getShortcutMapping()->deserialize(
        j.at("shortcuts").get<std::string>());
      profile->getShortcutMapping()->sync();
    }

    return profile;
  }

  std::optional<int> SqliteControllerRepository::getGameProfileOverride(
    const std::string &contentHash) const {
    QSqlQuery query(getDatabase());
    query.prepare(
      "SELECT profile_id FROM game_overrides WHERE content_hash = :hash");
    query.bindValue(":hash", QString::fromStdString(contentHash));
    if (!query.exec() || !query.next()) {
      return std::nullopt;
    }
    return query.value("profile_id").toInt();
  }

  void SqliteControllerRepository::setGameProfileOverride(
    const std::string &contentHash, const int profileId) {
    QSqlQuery query(getDatabase());
    query.prepare("INSERT OR REPLACE INTO game_overrides (content_hash, "
      "profile_id, created_at) VALUES (:hash, :profileId, :createdAt)");
    query.bindValue(":hash", QString::fromStdString(contentHash));
    query.bindValue(":profileId", profileId);
    query.bindValue(":createdAt", QDateTime::currentMSecsSinceEpoch());
    if (!query.exec()) {
      spdlog::error("Failed to set game override for {}: {}", contentHash,
                    query.lastError().text().toStdString());
    }
  }

  void SqliteControllerRepository::clearGameProfileOverride(
    const std::string &contentHash) {
    QSqlQuery query(getDatabase());
    query.prepare("DELETE FROM game_overrides WHERE content_hash = :hash");
    query.bindValue(":hash", QString::fromStdString(contentHash));
    if (!query.exec()) {
      spdlog::error("Failed to clear game override for {}: {}", contentHash,
                    query.lastError().text().toStdString());
    }
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
    if (!db.open()) {
      spdlog::error("Failed to open controller database connection '{}'",
                    name.toStdString());
    }
    return db;
  }

  std::shared_ptr<InputMapping> SqliteControllerRepository::getOrCreateMapping(
    int profileId, const int platformId, const int controllerTypeId) {
    for (const auto &mapping: m_inputMappings) {
      if (mapping->getControllerProfileId() == static_cast<unsigned>(profileId) &&
          mapping->getPlatformId() == static_cast<unsigned>(platformId) &&
          mapping->getControllerType() ==
          static_cast<unsigned>(controllerTypeId)) {
        return mapping;
      }
    }

    const auto syncMapping = [this](InputMapping &m) {
      QSqlQuery updateQuery(getDatabase());
      updateQuery.prepare(
        "UPDATE mappings SET mapping_data = :mappingData WHERE id = :id");
      updateQuery.bindValue(":mappingData",
                            QString::fromStdString(m.serialize()));
      updateQuery.bindValue(":id", m.getId());
      if (!updateQuery.exec()) {
        spdlog::error("Update failed: {}",
                      updateQuery.lastError().text().toStdString());
      }
    };

    QSqlQuery mappingsQuery(getDatabase());
    mappingsQuery.prepare(
      "SELECT * FROM mappings WHERE profile_id = :profileId AND "
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
        mappingId, profileId, platformId, controllerTypeId, syncMapping);
      mapping->deserialize(
        mappingsQuery.value("mapping_data").toString().toStdString());
      m_inputMappings.emplace_back(mapping);
      return mapping;
    }

    QSqlQuery insertQuery(getDatabase());
    insertQuery.prepare(
      "INSERT INTO mappings (profile_id, platform_id, controller_type, "
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

    const auto lastId = insertQuery.lastInsertId().toInt();
    auto mapping = std::make_shared<InputMapping>(
      lastId, profileId, platformId, controllerTypeId, syncMapping);
    m_inputMappings.emplace_back(mapping);
    return mapping;
  }
} // namespace firelight::input
