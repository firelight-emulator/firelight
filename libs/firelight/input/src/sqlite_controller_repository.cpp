#include <firelight/input/shortcut_mapping.hpp>
#include <firelight/input/shortcut_registry.hpp>
#include <firelight/input/sqlite_controller_repository.hpp>
#include <firelight/migrations/migration_runner.hpp>
#include <firelight/platforms/platform_service.hpp>

#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Statement.h>
#include <SQLiteCpp/Transaction.h>
#include <chrono>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <utility>

namespace firelight::input {
namespace {
int64_t nowMs() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

std::string serializeAnalog(const AnalogSettings &settings) { return nlohmann::json(settings).dump(); }

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

SqliteControllerRepository::SqliteControllerRepository(std::string dbFilePath,
                                                       platforms::PlatformService &platformService)
    : m_dbFilePath(std::move(dbFilePath)), m_platformService(platformService) {
  m_db = std::make_unique<SQLite::Database>(m_dbFilePath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

  // Forward-only schema migrations, stamped into PRAGMA user_version. A future
  // schema change adds the next-numbered migration
  const std::vector<migrations::Migration> schema = {
      {1,
       [this] {
         m_db->exec("CREATE TABLE IF NOT EXISTS profiles("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "name TEXT NOT NULL UNIQUE,"
                    "kind INTEGER NOT NULL DEFAULT 0,"
                    "builtin INTEGER NOT NULL DEFAULT 0,"
                    "based_on_type INTEGER NOT NULL DEFAULT -1,"
                    "icon TEXT,"
                    "analog_json TEXT,"
                    "preset_id TEXT NOT NULL DEFAULT '',"
                    "created_at INTEGER NOT NULL,"
                    "updated_at INTEGER NOT NULL);");

         m_db->exec("CREATE TABLE IF NOT EXISTS mappings("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "profile_id INTEGER NOT NULL,"
                    "platform_id INTEGER NOT NULL,"
                    "controller_type INTEGER NOT NULL,"
                    "mapping_data TEXT,"
                    "created_at INTEGER NOT NULL,"
                    "UNIQUE(profile_id, platform_id, controller_type));");

         m_db->exec("CREATE TABLE IF NOT EXISTS shortcuts("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "profile_id INTEGER NOT NULL UNIQUE,"
                    "mapping_data TEXT,"
                    "created_at INTEGER NOT NULL);");

         m_db->exec("CREATE TABLE IF NOT EXISTS devices("
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

         m_db->exec("CREATE TABLE IF NOT EXISTS game_overrides("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "content_hash TEXT NOT NULL UNIQUE,"
                    "profile_id INTEGER NOT NULL,"
                    "created_at INTEGER NOT NULL);");

         m_db->exec("CREATE TABLE IF NOT EXISTS platform_preferences("
                    "platform_id INTEGER PRIMARY KEY,"
                    "gamepad_type INTEGER NOT NULL);");
       }},
  };

  try {
    SQLite::Transaction transaction(*m_db);
    const int currentVersion = m_db->execAndGet("PRAGMA user_version").getInt();
    migrations::applyMigrations(currentVersion, schema,
                                [this](const int v) { m_db->exec("PRAGMA user_version = " + std::to_string(v)); });
    transaction.commit();
  } catch (const std::exception &e) {
    spdlog::error("Failed to initialize controller store: {}", e.what());
  }
}

SqliteControllerRepository::~SqliteControllerRepository() = default;

void SqliteControllerRepository::setPlatformPreferredType(const int platformId, const int gamepadType) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "INSERT OR REPLACE INTO platform_preferences(platform_id, gamepad_type) "
                                   "VALUES(:platformId, :gamepadType);");
    query.bind(":platformId", platformId);
    query.bind(":gamepadType", gamepadType);
    query.exec();
  } catch (const std::exception &e) {
    spdlog::error("Failed to set platform preference for {}: {}", platformId, e.what());
  }
}

void SqliteControllerRepository::clearPlatformPreferredType(const int platformId) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "DELETE FROM platform_preferences WHERE platform_id = :platformId;");
    query.bind(":platformId", platformId);
    query.exec();
  } catch (const std::exception &e) {
    spdlog::error("Failed to clear platform preference for {}: {}", platformId, e.what());
  }
}

std::optional<int> SqliteControllerRepository::getPlatformPreferredType(const int platformId) const {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "SELECT gamepad_type FROM platform_preferences WHERE platform_id = :platformId;");
    query.bind(":platformId", platformId);
    if (query.executeStep()) {
      return query.getColumn("gamepad_type").getInt();
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to read platform preference for {}: {}", platformId, e.what());
  }

  return std::nullopt;
}

std::optional<DeviceInfo> SqliteControllerRepository::getDeviceInfo(DeviceIdentifier identifier) const {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "SELECT * FROM devices WHERE vendor_id = :vendorId AND product_id = :productId "
                                   "AND product_version = :productVersion;");
    query.bind(":vendorId", identifier.vendorId);
    query.bind(":productId", identifier.productId);
    query.bind(":productVersion", identifier.productVersion);

    if (query.executeStep()) {
      return DeviceInfo{
          .displayName = query.getColumn("display_name").getString(),
          .profileId = query.getColumn("active_profile_id").getInt(),
      };
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to read device info for {}: {}", identifier.deviceName, e.what());
  }

  spdlog::debug("Device info not found for: {}", identifier.deviceName);
  return std::nullopt;
}

void SqliteControllerRepository::updateDeviceInfo(DeviceIdentifier identifier, const DeviceInfo &info) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "INSERT OR REPLACE INTO devices(device_name, device_type, vendor_id, product_id, "
                                   "product_version, display_name, active_profile_id, created_at) "
                                   "VALUES(:deviceName, :deviceType, :vendorId, :productId, :productVersion, "
                                   ":displayName, :activeProfileId, :createdAt);");
    query.bind(":deviceName", identifier.deviceName);
    query.bind(":deviceType", static_cast<int>(identifier.type));
    query.bind(":vendorId", identifier.vendorId);
    query.bind(":productId", identifier.productId);
    query.bind(":productVersion", identifier.productVersion);
    query.bind(":displayName", info.displayName);
    query.bind(":activeProfileId", info.profileId);
    query.bind(":createdAt", nowMs());
    query.exec();
    spdlog::debug("Updated device info for: {}", identifier.deviceName);
  } catch (const std::exception &e) {
    spdlog::error("Failed to update device info: {}", e.what());
  }
}

namespace {
// Copies a preset's shipped bindings into a brand-new profile's mapping
//
// A preset is a starting point, not a tier the engine resolves through: from
// here on the profile owns a flat mapping, so an action with no binding means
// "off" rather than "inherit", and resetting a row is a write
//
// Sources the controller hasn't got are copied in too, deliberately. They can
// never match (the engine only ever sees codes a device actually reports), and
// a preset lists alternates precisely so the next one takes over — an N64 pad
// simply never satisfies the L3+R3 entry and lands on Select+Start. Filtering
// here would need the device, which a profile isn't tied to: profiles are
// shared between controllers, so one pad's shape must not be baked in
void seedShortcuts(ShortcutMapping &mapping, const std::string &presetId, const DeviceType device) {
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
  // Mutating a mapping doesn't persist it; the caller syncs
  mapping.sync();
}
} // namespace

void SqliteControllerRepository::loadProfileContents(const std::shared_ptr<GamepadProfile> &profile) {
  std::lock_guard lock(m_mutex);

  for (const auto &platform : m_platformService.listPlatforms()) {
    for (const auto &controller : platform.controllerTypes) {
      auto mapping = getOrCreateMapping(profile->getId(), platform.id, controller.id);
      if (!mapping) {
        spdlog::error("Failed to create or retrieve mapping for profile ID {} "
                      "on platform {} with controller type {}",
                      profile->getId(), platform.id, controller.id);
        continue;
      }
      profile->addMapping(mapping);
    }
  }

  const auto syncShortcuts = [id = profile->getId(), this](const ShortcutMapping &mapping) {
    std::lock_guard syncLock(m_mutex);
    try {
      SQLite::Statement update(*m_db,
                               "UPDATE shortcuts SET mapping_data = :mappingData WHERE profile_id = :profileId;");
      update.bind(":mappingData", mapping.serialize());
      update.bind(":profileId", id);
      update.exec();
    } catch (const std::exception &e) {
      spdlog::error("Failed to update shortcuts: {}", e.what());
    }
  };

  bool hasRow = false;
  std::string mappingData;
  try {
    SQLite::Statement query(*m_db, "SELECT mapping_data FROM shortcuts WHERE profile_id = :profileId;");
    query.bind(":profileId", profile->getId());
    if (query.executeStep()) {
      hasRow = true;
      mappingData = query.getColumn("mapping_data").getString();
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to read shortcuts for profile {}: {}", profile->getId(), e.what());
  }

  if (hasRow) {
    auto shortcutMapping = std::make_shared<ShortcutMapping>(syncShortcuts);
    shortcutMapping->deserialize(mappingData);
    profile->setShortcutMapping(shortcutMapping);
  } else {
    try {
      SQLite::Statement insert(*m_db, "INSERT INTO shortcuts(profile_id, mapping_data, created_at) "
                                      "VALUES(:profileId, :mappingData, :createdAt);");
      insert.bind(":profileId", profile->getId());
      insert.bind(":mappingData", "");
      insert.bind(":createdAt", nowMs());
      insert.exec();
    } catch (const std::exception &e) {
      spdlog::error("Failed to create shortcuts for profile ID {}: {}", profile->getId(), e.what());
    }

    auto shortcutMapping = std::make_shared<ShortcutMapping>(syncShortcuts);
    seedShortcuts(*shortcutMapping, profile->getPresetId(),
                  profile->isKeyboardProfile() ? DeviceType::Keyboard : DeviceType::Gamepad);
    profile->setShortcutMapping(shortcutMapping);
  }
}

std::shared_ptr<GamepadProfile> SqliteControllerRepository::createProfile(const std::string name,
                                                                          const DeviceType device) {
  std::lock_guard lock(m_mutex);

  const auto presetId = ShortcutRegistry::instance().defaultPresetId();

  int newId = -1;
  try {
    SQLite::Statement query(*m_db, "INSERT INTO profiles(name, kind, builtin, based_on_type, icon, analog_json, "
                                   "preset_id, created_at, updated_at) "
                                   "VALUES(:name, :kind, 0, -1, '', :analog, :presetId, :createdAt, :updatedAt);");
    const auto now = nowMs();
    query.bind(":name", name);
    query.bind(":kind", device == DeviceType::Keyboard ? 1 : 0);
    query.bind(":analog", serializeAnalog(AnalogSettings{}));
    query.bind(":presetId", presetId);
    query.bind(":createdAt", now);
    query.bind(":updatedAt", now);
    query.exec();
    newId = static_cast<int>(m_db->getLastInsertRowid());
  } catch (const std::exception &e) {
    spdlog::error("Failed to create new profile: {}", e.what());
    return nullptr;
  }

  auto profile = std::make_shared<GamepadProfile>(newId);
  profile->setName(name);
  // Both are set before loadProfileContents, which seeds the profile's
  // shortcuts from the preset for its kind of device
  profile->setIsKeyboardProfile(device == DeviceType::Keyboard);
  profile->setPresetId(presetId);
  loadProfileContents(profile);

  m_profiles.emplace_back(profile);
  return profile;
}

std::shared_ptr<GamepadProfile> SqliteControllerRepository::getProfile(const int id) {
  std::lock_guard lock(m_mutex);

  for (const auto &profile : m_profiles) {
    if (profile->getId() == id) {
      return profile;
    }
  }

  std::shared_ptr<GamepadProfile> profile;
  try {
    SQLite::Statement query(*m_db, "SELECT * FROM profiles WHERE id = :id;");
    query.bind(":id", id);
    if (!query.executeStep()) {
      spdlog::error("Profile with ID {} not found", id);
      return nullptr;
    }

    profile = std::make_shared<GamepadProfile>(query.getColumn("id").getInt());
    profile->setName(query.getColumn("name").getString());
    profile->setIsKeyboardProfile(query.getColumn("kind").getInt() == 1);
    profile->setBuiltin(query.getColumn("builtin").getInt() != 0);
    profile->setBasedOnType(query.getColumn("based_on_type").getInt());
    profile->setIcon(query.getColumn("icon").getString());
    profile->setPresetId(query.getColumn("preset_id").getString());
    profile->setDefaultAnalogSettings(deserializeAnalog(query.getColumn("analog_json").getString()));
  } catch (const std::exception &e) {
    spdlog::error("Failed to read profile {}: {}", id, e.what());
    return nullptr;
  }

  loadProfileContents(profile);

  m_profiles.emplace_back(profile);
  return profile;
}

std::vector<std::shared_ptr<GamepadProfile>> SqliteControllerRepository::listProfiles() {
  std::lock_guard lock(m_mutex);

  std::vector<int> ids;
  try {
    SQLite::Statement query(*m_db, "SELECT id FROM profiles ORDER BY id;");
    while (query.executeStep()) {
      ids.push_back(query.getColumn("id").getInt());
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to list profiles: {}", e.what());
    return {};
  }

  std::vector<std::shared_ptr<GamepadProfile>> result;
  for (const int id : ids) {
    if (auto profile = getProfile(id)) {
      result.emplace_back(profile);
    }
  }
  return result;
}

std::shared_ptr<GamepadProfile> SqliteControllerRepository::cloneProfile(const int sourceId, std::string newName) {
  std::lock_guard lock(m_mutex);

  const auto source = getProfile(sourceId);
  if (!source) {
    spdlog::error("Cannot clone: source profile {} not found", sourceId);
    return nullptr;
  }

  auto clone = createProfile(std::move(newName));
  if (!clone) {
    return nullptr;
  }

  // Copy the profile-wide analog defaults
  setProfileAnalogSettings(clone->getId(), source->getDefaultAnalogSettings());

  // Copy each platform's bindings (and any per-platform analog override) by
  // round-tripping the serialized mapping
  for (const auto &platform : m_platformService.listPlatforms()) {
    for (const auto &controller : platform.controllerTypes) {
      const auto src = source->getMappingForPlatformAndController(platform.id, controller.id);
      auto dst = clone->getMappingForPlatformAndController(platform.id, controller.id);
      if (src && dst) {
        dst->deserialize(src->serialize());
        dst->sync();
      }
    }
  }

  // Copy the shortcut mapping
  if (source->getShortcutMapping() && clone->getShortcutMapping()) {
    clone->getShortcutMapping()->deserialize(source->getShortcutMapping()->serialize());
    clone->getShortcutMapping()->sync();
  }

  return clone;
}

bool SqliteControllerRepository::deleteProfile(const int id) {
  std::lock_guard lock(m_mutex);

  const auto profile = getProfile(id);
  if (profile && profile->isBuiltin()) {
    spdlog::warn("Refusing to delete built-in profile {}", id);
    return false;
  }

  try {
    for (const char *sql : {"DELETE FROM profiles WHERE id = :id;", "DELETE FROM mappings WHERE profile_id = :id;",
                            "DELETE FROM shortcuts WHERE profile_id = :id;"}) {
      SQLite::Statement query(*m_db, sql);
      query.bind(":id", id);
      query.exec();
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to delete profile {}: {}", id, e.what());
    return false;
  }

  std::erase_if(m_profiles, [id](const std::shared_ptr<GamepadProfile> &p) { return p->getId() == id; });
  std::erase_if(m_inputMappings, [id](const std::shared_ptr<InputMapping> &m) {
    return static_cast<int>(m->getControllerProfileId()) == id;
  });
  return true;
}

bool SqliteControllerRepository::renameProfile(const int id, std::string newName) {
  std::lock_guard lock(m_mutex);

  try {
    SQLite::Statement query(*m_db, "UPDATE profiles SET name = :name, updated_at = :updatedAt WHERE id = :id;");
    query.bind(":name", newName);
    query.bind(":updatedAt", nowMs());
    query.bind(":id", id);
    query.exec();
  } catch (const std::exception &e) {
    spdlog::error("Failed to rename profile {}: {}", id, e.what());
    return false;
  }

  if (const auto profile = getProfile(id)) {
    profile->setName(std::move(newName));
  }
  return true;
}

void SqliteControllerRepository::setProfileAnalogSettings(const int profileId, const AnalogSettings &settings) {
  std::lock_guard lock(m_mutex);

  try {
    SQLite::Statement query(*m_db,
                            "UPDATE profiles SET analog_json = :analog, updated_at = :updatedAt WHERE id = :id;");
    query.bind(":analog", serializeAnalog(settings));
    query.bind(":updatedAt", nowMs());
    query.bind(":id", profileId);
    query.exec();
  } catch (const std::exception &e) {
    spdlog::error("Failed to update analog settings for profile {}: {}", profileId, e.what());
    return;
  }

  if (const auto profile = getProfile(profileId)) {
    profile->setDefaultAnalogSettings(settings);
  }
}

void SqliteControllerRepository::setProfilePresetId(const int profileId, const std::string &presetId) {
  std::lock_guard lock(m_mutex);

  try {
    SQLite::Statement query(*m_db,
                            "UPDATE profiles SET preset_id = :presetId, updated_at = :updatedAt WHERE id = :id;");
    query.bind(":presetId", presetId);
    query.bind(":updatedAt", nowMs());
    query.bind(":id", profileId);
    query.exec();
  } catch (const std::exception &e) {
    spdlog::error("Failed to update preset for profile {}: {}", profileId, e.what());
    return;
  }

  if (const auto profile = getProfile(profileId)) {
    profile->setPresetId(presetId);
  }
}

std::string SqliteControllerRepository::exportProfile(const int id) {
  std::lock_guard lock(m_mutex);

  const auto profile = getProfile(id);
  if (!profile) {
    return {};
  }

  nlohmann::json mappings = nlohmann::json::array();
  for (const auto &platform : m_platformService.listPlatforms()) {
    for (const auto &controller : platform.controllerTypes) {
      const auto mapping = profile->getMappingForPlatformAndController(platform.id, controller.id);
      if (!mapping) {
        continue;
      }
      mappings.push_back({{"platform_id", platform.id},
                          {"controller_type", controller.id},
                          {"data", nlohmann::json::parse(mapping->serialize(), nullptr, false)}});
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

std::shared_ptr<GamepadProfile> SqliteControllerRepository::importProfile(const std::string &json) {
  std::lock_guard lock(m_mutex);

  const auto j = nlohmann::json::parse(json, nullptr, false);
  if (j.is_discarded() || !j.is_object()) {
    spdlog::error("Cannot import profile: invalid JSON");
    return nullptr;
  }

  // Ensure the imported name does not collide with an existing one
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
    setProfileAnalogSettings(profile->getId(), j.at("analog").get<AnalogSettings>());
  }

  if (j.contains("mappings")) {
    for (const auto &item : j.at("mappings")) {
      const auto platformId = item.value("platform_id", -1);
      const auto controllerType = item.value("controller_type", -1);
      auto dst = profile->getMappingForPlatformAndController(platformId, controllerType);
      if (dst && item.contains("data")) {
        dst->deserialize(item.at("data").dump());
        dst->sync();
      }
    }
  }

  if (j.contains("shortcuts") && profile->getShortcutMapping()) {
    profile->getShortcutMapping()->deserialize(j.at("shortcuts").get<std::string>());
    profile->getShortcutMapping()->sync();
  }

  return profile;
}

std::optional<int> SqliteControllerRepository::getGameProfileOverride(const std::string &contentHash) const {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "SELECT profile_id FROM game_overrides WHERE content_hash = :hash;");
    query.bind(":hash", contentHash);
    if (query.executeStep()) {
      return query.getColumn("profile_id").getInt();
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to read game override for {}: {}", contentHash, e.what());
  }

  return std::nullopt;
}

void SqliteControllerRepository::setGameProfileOverride(const std::string &contentHash, const int profileId) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "INSERT OR REPLACE INTO game_overrides(content_hash, profile_id, created_at) "
                                   "VALUES(:hash, :profileId, :createdAt);");
    query.bind(":hash", contentHash);
    query.bind(":profileId", profileId);
    query.bind(":createdAt", nowMs());
    query.exec();
  } catch (const std::exception &e) {
    spdlog::error("Failed to set game override for {}: {}", contentHash, e.what());
  }
}

void SqliteControllerRepository::clearGameProfileOverride(const std::string &contentHash) {
  std::lock_guard lock(m_mutex);
  try {
    SQLite::Statement query(*m_db, "DELETE FROM game_overrides WHERE content_hash = :hash;");
    query.bind(":hash", contentHash);
    query.exec();
  } catch (const std::exception &e) {
    spdlog::error("Failed to clear game override for {}: {}", contentHash, e.what());
  }
}

std::shared_ptr<InputMapping> SqliteControllerRepository::getOrCreateMapping(int profileId, const int platformId,
                                                                             const int controllerTypeId) {
  std::lock_guard lock(m_mutex);

  for (const auto &mapping : m_inputMappings) {
    if (mapping->getControllerProfileId() == static_cast<unsigned>(profileId) &&
        mapping->getPlatformId() == static_cast<unsigned>(platformId) &&
        mapping->getControllerType() == static_cast<unsigned>(controllerTypeId)) {
      return mapping;
    }
  }

  const auto syncMapping = [this](InputMapping &m) {
    std::lock_guard syncLock(m_mutex);
    try {
      SQLite::Statement update(*m_db, "UPDATE mappings SET mapping_data = :mappingData WHERE id = :id;");
      update.bind(":mappingData", m.serialize());
      update.bind(":id", m.getId());
      update.exec();
    } catch (const std::exception &e) {
      spdlog::error("Update failed: {}", e.what());
    }
  };

  bool hasRow = false;
  int mappingId = -1;
  std::string mappingData;
  try {
    SQLite::Statement query(*m_db, "SELECT id, mapping_data FROM mappings WHERE profile_id = :profileId AND "
                                   "platform_id = :platformId AND controller_type = :controllerType;");
    query.bind(":profileId", profileId);
    query.bind(":platformId", platformId);
    query.bind(":controllerType", controllerTypeId);
    if (query.executeStep()) {
      hasRow = true;
      mappingId = query.getColumn("id").getInt();
      mappingData = query.getColumn("mapping_data").getString();
    }
  } catch (const std::exception &e) {
    spdlog::error("Failed to fetch mappings for profile ID {}: {}", profileId, e.what());
    return nullptr;
  }

  if (hasRow) {
    auto mapping = std::make_shared<InputMapping>(mappingId, profileId, platformId, controllerTypeId, syncMapping);
    mapping->deserialize(mappingData);
    m_inputMappings.emplace_back(mapping);
    return mapping;
  }

  int newId = -1;
  try {
    SQLite::Statement insert(*m_db, "INSERT INTO mappings(profile_id, platform_id, controller_type, created_at) "
                                    "VALUES(:profileId, :platformId, :controllerType, :createdAt);");
    insert.bind(":profileId", profileId);
    insert.bind(":platformId", platformId);
    insert.bind(":controllerType", controllerTypeId);
    insert.bind(":createdAt", nowMs());
    insert.exec();
    newId = static_cast<int>(m_db->getLastInsertRowid());
  } catch (const std::exception &e) {
    spdlog::error("Insert failed: {}", e.what());
    return nullptr;
  }

  auto mapping = std::make_shared<InputMapping>(newId, profileId, platformId, controllerTypeId, syncMapping);
  m_inputMappings.emplace_back(mapping);
  return mapping;
}
} // namespace firelight::input
