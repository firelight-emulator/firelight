#include "sqlite_userdata_database.hpp"

#include <QSqlError>
#include <QSqlQuery>
#include <spdlog/spdlog.h>

namespace firelight::db {
  constexpr auto DATABASE_PREFIX = "userdata_";

  const std::map<int, std::map<std::string, std::string> > defaultPlatformValues = {
    {
      // GB
      1, {
        {"gambatte_gbc_color_correction", "GBC only"},
        {"gambatte_gbc_color_correction_mode", "accurate"},
        {"gambatte_gbc_frontlight_position", "central"},
        {"gambatte_dark_filter_level", "0"},
        {"gambatte_up_down_allowed", "disabled"},
        {"gambatte_mix_frames", "disabled"},
        {"gambatte_gb_colorization", "auto"},
        {"gambatte_gb_internal_palette", "GB - DMG"},
        {"gambatte_gb_bootloader", "disabled"},
        {"gambatte_gb_hwmode", "auto"}
      }
    },
    // GBC
    {
      2, {
        {"gambatte_gbc_color_correction", "GBC only"},
        {"gambatte_gbc_color_correction_mode", "accurate"},
        {"gambatte_gbc_frontlight_position", "central"},
        {"gambatte_dark_filter_level", "0"},
        {"gambatte_up_down_allowed", "disabled"},
        {"gambatte_mix_frames", "disabled"},
        {"gambatte_gb_colorization", "auto"},
        {"gambatte_gb_internal_palette", "GB - DMG"},
        {"gambatte_gb_bootloader", "disabled"},
        {"gambatte_gb_hwmode", "auto"}
      }
    },
    // GBA
    {
      3, {
        {"mgba_solar_sensor_level", "0"},
        {"mgba_gb_model", "Autodetect"},
        {"mgba_sgb_borders", "ON"},
        {"mgba_gb_colors_preset", "0"},
        {"mgba_gb_colors", "Grayscale"},
        {"mgba_use_bios", "OFF"},
        {"mgba_skip_bios", "ON"},
        {"mgba_sgb_borders", "ON"},
        {"mgba_frameskip", "disabled"},
        {"mgba_frameskip_threshold", "33"},
        {"mgba_frameskip_interval", "0"},
        {"mgba_audio_low_pass_filter", "disabled"},
        {"mgba_audio_low_pass_range", "60"},
        {"mgba_idle_optimization", "Remove Known"},
        {"mgba_force_gbp", "OFF"},
        {"mgba_color_correction", "OFF"},
        {"mgba_interframe_blending", "OFF"}
      }
    },
    // NES
    // {
    //   5, {
    //     {"gambatte_gbc_color_correction", "GBC only"},
    //     {"gambatte_gbc_color_correction_mode", "accurate"},
    //     {"gambatte_gbc_frontlight_position", "central"},
    //     {"gambatte_dark_filter_level", "0"},
    //     {"gambatte_up_down_allowed", "disabled"},
    //     {"gambatte_mix_frames", "disabled"},
    //     {"gambatte_gb_colorization", "disabled"}
    //   }
    // },
    // SNES
    {
      6, {
        {"snes9x_hires_blend", "disabled"},
        {"snes9x_overclock_superfx", "100%"},
        {"snes9x_up_down_allowed", "disabled"},
        {"snes9x_sndchan_1", "enabled"},
        {"snes9x_sndchan_2", "enabled"},
        {"snes9x_sndchan_3", "enabled"},
        {"snes9x_sndchan_4", "enabled"},
        {"snes9x_sndchan_5", "enabled"},
        {"snes9x_sndchan_6", "enabled"},
        {"snes9x_sndchan_7", "enabled"},
        {"snes9x_sndchan_8", "enabled"},
        {"snes9x_layer_1", "enabled"},
        {"snes9x_layer_2", "enabled"},
        {"snes9x_layer_3", "enabled"},
        {"snes9x_layer_4", "enabled"},
        {"snes9x_layer_5", "enabled"},
        {"snes9x_gfx_clip", "enabled"},
        {"snes9x_gfx_transp", "enabled"},
        {"snes9x_audio_interpolation", "gaussian"},
        {"snes9x_overclock_cycles", "disabled"},
        {"snes9x_reduce_sprite_flicker", "disabled"},
        {"snes9x_randomize_memory", "disabled"},
        {"snes9x_overscan", "enabled"},
        {"snes9x_aspect", "4:3"},
        {"snes9x_region", "auto"},
        {"snes9x_lightgun_mode", "Lightgun"},
        {"snes9x_superscope_reverse_buttons", "disabled"},
        {"snes9x_superscope_crosshair", "2"},
        {"snes9x_superscope_color", "White"},
        {"snes9x_justifier1_crosshair", "4"},
        {"snes9x_justifier1_color", "Blue"},
        {"snes9x_justifier2_crosshair", "4"},
        {"snes9x_justifier2_color", "Pink"},
        {"snes9x_rifle_crosshair", "2"},
        {"snes9x_rifle_color", "White"},
        {"snes9x_block_invalid_vram_access", "enabled"},
        {"snes9x_echo_buffer_hack", "disabled"},
        {"snes9x_blargg", "disabled"}
      }
    },
    // N64
    {
      7, {
        {"mupen64plus-rdp-plugin", "gliden64"},
        {"mupen64plus-rsp-plugin", "hle"},
        {"mupen64plus-ThreadedRenderer", "False"},
        {"mupen64plus-BilinearMode", "standard"},
        {"mupen64plus-HybridFilter", "True"},
        {"mupen64plus-DitheringPattern", "False"},
        {"mupen64plus-DitheringQuantization", "False"},
        {"mupen64plus-RDRAMImageDitheringMode", "False"},
        {"mupen64plus-FXAA", "0"},
        {"mupen64plus-MultiSampling", "0"},
        {"mupen64plus-FrameDuping", "False"},
        {"mupen64plus-Framerate", "Original"},
        {"mupen64plus-virefresh", "Auto"},
        {"mupen64plus-EnableLODEmulation", "True"},
        {"mupen64plus-EnableFBEmulation", "True"},
        {"mupen64plus-EnableN64DepthCompare", "False"},
        {"mupen64plus-EnableCopyColorToRDRAM", "Async"},
        {"mupen64plus-EnableCopyDepthToRDRAM", "Software"},
        {"mupen64plus-EnableHWLighting", "False"},
        {"mupen64plus-CorrectTexrectCoords", "Off"},
        {"mupen64plus-EnableTexCoordBounds", "False"},
        {"mupen64plus-BackgroundMode", "OnePiece"},
        {"mupen64plus-EnableNativeResTexrects", "Disabled"},
        {"mupen64plus-txFilterMode", "None"},
        {"mupen64plus-txEnhancementMode", "None"},
        {"mupen64plus-txFilterIgnoreBG", "True"},
        {"mupen64plus-txHiresEnable", "False"},
        {"mupen64plus-txCacheCompression", "True"},
        {"mupen64plus-txHiresFullAlphaChannel", "False"},
        {"mupen64plus-MaxTxCacheSize", "8000"},
        {"mupen64plus-EnableLegacyBlending", "False"},
        {"mupen64plus-EnableFragmentDepthWrite", "True"},
        {"mupen64plus-EnableShadersStorage", "True"},
        {"mupen64plus-EnableTextureCache", "True"},
        {"mupen64plus-EnableEnhancedTextureStorage", "False"},
        {"mupen64plus-EnableEnhancedHighResStorage", "False"},
        {"mupen64plus-EnableHiResAltCRC", "False"},
        {"mupen64plus-EnableCopyAuxToRDRAM", "False"},
        {"mupen64plus-GLideN64IniBehaviour", "late"},
        {"mupen64plus-cpucore", "dynamic_recompiler"},
        {"mupen64plus-aspect", "4:3"},
        {"mupen64plus-EnableNativeResFactor", "0"},
        {"mupen64plus-43screensize", "640x480"},
        {"mupen64plus-astick-deadzone", "15"},
        {"mupen64plus-astick-sensitivity", "100"},
        {"mupen64plus-CountPerOp", "0"},
        {"mupen64plus-r-cbutton", "C1"},
        {"mupen64plus-l-cbutton", "C2"},
        {"mupen64plus-d-cbutton", "C3"},
        {"mupen64plus-u-cbutton", "C4"},
        {"mupen64plus-EnableOverscan", "Enabled"},
        {"mupen64plus-OverscanTop", "0"},
        {"mupen64plus-OverscanLeft", "0"},
        {"mupen64plus-OverscanRight", "0"},
        {"mupen64plus-OverscanBottom", "0"},
        {"mupen64plus-alt-map", "False"},
        {"mupen64plus-ForceDisableExtraMem", "False"},
        {"mupen64plus-IgnoreTLBExceptions", "False"},
        {"mupen64plus-pak1", "memory"},
        {"mupen64plus-pak2", "none"},
        {"mupen64plus-pak3", "none"},
        {"mupen64plus-pak4", "none"}
      }
    }
  };

  SqliteUserdataDatabase::SqliteUserdataDatabase(
    const std::filesystem::path &dbFile)
    : m_database_path(dbFile) {
    m_database = QSqlDatabase::addDatabase("QSQLITE", "userdata");
    m_database.setDatabaseName(QString::fromStdString(dbFile.string()));
    if (!m_database.open()) {
      throw std::runtime_error("Couldn't open Userdata database");
    }

    QSqlQuery createSavefileMetadata(m_database);
    createSavefileMetadata.prepare("CREATE TABLE IF NOT EXISTS savefile_metadata("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "content_id TEXT NOT NULL,"
      "slot_number INTEGER NOT NULL,"
      "savefile_md5 TEXT NOT NULL,"
      "last_modified_at INTEGER NOT NULL,"
      "created_at INTEGER NOT NULL, "
      "UNIQUE(content_id, slot_number));");

    if (!createSavefileMetadata.exec()) {
      spdlog::error("Table creation failed: {}",
                    createSavefileMetadata.lastError().text().toStdString());
    }

    QSqlQuery createPlaySessions(m_database);
    createPlaySessions.prepare("CREATE TABLE IF NOT EXISTS play_sessions("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "content_id TEXT NOT NULL,"
      "savefile_slot_number INTEGER NOT NULL,"
      "start_time INTEGER NOT NULL,"
      "end_time INTEGER NOT NULL,"
      "unpaused_duration_seconds INTEGER NOT NULL);");

    if (!createPlaySessions.exec()) {
      spdlog::error("Table creation failed: {}",
                    createPlaySessions.lastError().text().toStdString());
    }

    QSqlQuery createControllerProfiles(m_database);
    createControllerProfiles.prepare("CREATE TABLE IF NOT EXISTS controller_profiles("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "display_name TEXT NOT NULL);");

    if (!createControllerProfiles.exec()) {
      spdlog::error("Table creation failed: {}",
                    createControllerProfiles.lastError().text().toStdString());
    }

    QSqlQuery createInputMappings(m_database);
    createInputMappings.prepare("CREATE TABLE IF NOT EXISTS input_mappings("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "controller_profile_id INTEGER NOT NULL,"
      "platform_id INTEGER NOT NULL, "
      "UNIQUE(controller_profile_id, platform_id));");

    if (!createInputMappings.exec()) {
      spdlog::error("Table creation failed: {}",
                    createInputMappings.lastError().text().toStdString());
    }

    QSqlQuery createPlatformSettings(m_database);
    createPlatformSettings.prepare("CREATE TABLE IF NOT EXISTS platform_settings("
      "platform_id INTEGER NOT NULL, "
      "key TEXT NOT NULL,"
      "value TEXT NOT NULL,"
      "UNIQUE(platform_id, key));");

    if (!createPlatformSettings.exec()) {
      spdlog::error("Table creation failed: {}",
                    createPlatformSettings.lastError().text().toStdString());
    }
  }

  SqliteUserdataDatabase::~SqliteUserdataDatabase() {
    m_database.close();
    QSqlDatabase::removeDatabase(m_database.connectionName());
  }

  std::vector<SavefileMetadata>
  SqliteUserdataDatabase::getSavefileMetadataForContent(
    const std::string contentId) {
    QSqlQuery query(m_database);
    query.prepare(
      "SELECT * FROM savefile_metadata WHERE content_id = :contentId;");
    query.bindValue(":contentId", QString::fromStdString(contentId));

    if (!query.exec()) {
      spdlog::warn("Could not retrieve savefile metadata: {}",
                   query.lastError().text().toStdString());
      return {};
    }

    std::vector<SavefileMetadata> metadataList;
    while (query.next()) {
      SavefileMetadata metadata;
      metadata.id = query.value("id").toUInt();
      metadata.contentId = query.value("content_id").toString().toStdString();
      metadata.slotNumber = query.value("slot_number").toUInt();
      metadata.savefileMd5 = query.value("savefile_md5").toString().toStdString();
      metadata.lastModifiedAt = query.value("last_modified_at").toLongLong();
      metadata.createdAt = query.value("created_at").toLongLong();
      metadataList.emplace_back(metadata);
    }

    return metadataList;
  }

  bool SqliteUserdataDatabase::tableExists(const std::string tableName) {
    QSqlQuery query(m_database);
    query.prepare("SELECT 1 FROM " + QString::fromStdString(tableName) +
                  " LIMIT 1;");

    return query.exec();
  }

  std::optional<SavefileMetadata>
  SqliteUserdataDatabase::getSavefileMetadata(const std::string contentId,
                                              const int slotNumber) {
    const QString queryString =
        "SELECT * FROM savefile_metadata WHERE content_id = :contentId AND "
        "slot_number = :slotNumber LIMIT 1;";
    QSqlQuery query(m_database);
    query.prepare(queryString);
    query.bindValue(":contentId", QString::fromStdString(contentId));
    query.bindValue(":slotNumber", slotNumber);

    if (!query.exec()) {
      spdlog::error("Failed to get savefile metadata: {}",
                    query.lastError().text().toStdString());
      return std::nullopt;
    }

    if (!query.next()) {
      return std::nullopt;
    }

    SavefileMetadata metadata;
    metadata.id = query.value("id").toUInt();
    metadata.contentId = query.value("content_id").toString().toStdString();
    metadata.slotNumber = query.value("slot_number").toUInt();
    metadata.savefileMd5 = query.value("savefile_md5").toString().toStdString();
    metadata.lastModifiedAt = query.value("last_modified_at").toLongLong();
    metadata.createdAt = query.value("created_at").toLongLong();

    return metadata;
  }

  bool SqliteUserdataDatabase::updateSavefileMetadata(SavefileMetadata metadata) {
    QSqlQuery query(m_database);
    query.prepare("UPDATE savefile_metadata SET savefile_md5 = :savefileMd5, "
      "last_modified_at = :lastModifiedAt WHERE id = :id;");
    query.bindValue(":savefileMd5", QString::fromStdString(metadata.savefileMd5));
    query.bindValue(":lastModifiedAt", metadata.lastModifiedAt);
    query.bindValue(":id", metadata.id);

    if (!query.exec()) {
      spdlog::error("Update Savefile metadata failed: {}",
                    query.lastError().text().toStdString());
      return false;
    }

    return query.numRowsAffected() >= 1;
  }

  bool SqliteUserdataDatabase::createSavefileMetadata(
    SavefileMetadata &metadata) {
    if (!m_database.open()) {
      spdlog::error("Couldn't open database: {}",
                    m_database.lastError().text().toStdString());
      return false;
    }

    const QString queryString = "INSERT INTO savefile_metadata (content_id, "
        "slot_number, savefile_md5, last_modified_at, "
        "created_at) VALUES (:contentId, :slotNumber, "
        ":savefileMd5, :lastModifiedAt, :createdAt);";

    QSqlQuery query(m_database);
    query.prepare(queryString);
    query.bindValue(":contentId", QString::fromStdString(metadata.contentId));
    query.bindValue(":slotNumber", metadata.slotNumber);
    query.bindValue(":savefileMd5", QString::fromStdString(metadata.savefileMd5));
    query.bindValue(":lastModifiedAt", metadata.lastModifiedAt);
    query.bindValue(":createdAt",
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                    .count());

    if (!query.exec()) {
      query.finish();
      return false;
    }

    metadata.id = query.lastInsertId().toInt();

    query.finish();
    return true;
  }

  bool SqliteUserdataDatabase::createControllerProfile(ControllerProfile &profile) {
    return false;
  }

  std::optional<ControllerProfile> SqliteUserdataDatabase::getControllerProfile(const int id) {
    printf("Getting controller profile: %d\n", id);
    return {
      {
        id,
        "Test",
        {
          {
            0, id, 1
          }
        }
      }
    };
  }

  std::vector<ControllerProfile> SqliteUserdataDatabase::getControllerProfiles() {
    return {};
  }

  bool SqliteUserdataDatabase::createPlaySession(PlaySession &session) {
    const QString queryString = "INSERT INTO play_sessions ("
        "content_id, "
        "savefile_slot_number, "
        "start_time, "
        "end_time, "
        "unpaused_duration_seconds) "
        "VALUES (:contentId, :slotNumber, :startTime,"
        ":endTime, :unpausedDurationSeconds);";
    auto query = QSqlQuery(m_database);
    query.prepare(queryString);

    query.bindValue(":contentId", QString::fromStdString(session.contentId));
    query.bindValue(":slotNumber", session.slotNumber);
    query.bindValue(":startTime", session.startTime);
    query.bindValue(":endTime", session.endTime);
    query.bindValue(":unpausedDurationSeconds",
                    static_cast<uint16_t>(session.unpausedDurationMillis / 1000));

    if (!query.exec()) {
      spdlog::warn("Insert into play_sessions failed: {}",
                   query.lastError().text().toStdString());
      return false;
    }

    session.id = query.lastInsertId().toInt();

    return true;
  }

  std::optional<PlaySession>
  SqliteUserdataDatabase::getLatestPlaySession(const std::string contentId) {
    return std::nullopt;
    // const QString queryString = "SELECT * FROM play_sessions WHERE content_id
    // = "
    //                             ":contentId ORDER BY end_time DESC LIMIT 1;";
    // auto query = QSqlQuery(m_database);
    // query.prepare(queryString);
    //
    // query.bindValue(":contentId", QString::fromStdString(contentId));
    //
    // if (!query.exec()) {
    //   spdlog::warn("Retrieving last play_session failed: {}",
    //                query.lastError().text().toStdString());
    //   return std::nullopt;
    // }
    //
    // if (query.next()) {
    //   PlaySession session;
    //   session.id = query.value("id").toInt();
    //   session.contentId = query.value("content_id").toString().toStdString();
    //   session.slotNumber = query.value("savefile_slot_number").toUInt();
    //   session.startTime = query.value("start_time").toLongLong();
    //   session.endTime = query.value("end_time").toLongLong();
    //   session.unpausedDurationSeconds =
    //       query.value("unpaused_duration_seconds").toUInt();
    //
    //   return {session};
    // }
    //
    // return std::nullopt;
  }

  std::optional<std::string> SqliteUserdataDatabase::getPlatformSettingValue(
    int platformId, std::string key, bool defaultIfNotFound) {
    const QString queryString = "SELECT value FROM platform_settings "
        "WHERE platform_id = :platformId AND key = :key LIMIT 1;";
    auto query = QSqlQuery(m_database);
    query.prepare(queryString);

    query.bindValue(":platformId", platformId);
    query.bindValue(":key", QString::fromStdString(key));

    if (!query.exec()) {
      spdlog::warn("Get from platform_settings failed: {}",
                   query.lastError().text().toStdString());
      return std::nullopt;
    }

    if (query.next()) {
      return query.value("value").toString().toStdString();
    } else if (defaultIfNotFound) {
      return defaultPlatformValues.at(platformId).at(key);
    }

    return std::nullopt;
  }

  void SqliteUserdataDatabase::setPlatformSettingValue(const int platformId, const std::string key,
                                                       const std::string value) {
    const QString queryString = "INSERT OR REPLACE INTO platform_settings "
        "(platform_id, key, value) "
        "VALUES (:platformId, :key, :value);";
    auto query = QSqlQuery(m_database);
    query.prepare(queryString);

    query.bindValue(":platformId", platformId);
    query.bindValue(":key", QString::fromStdString(key));
    query.bindValue(":value", QString::fromStdString(value));

    if (!query.exec()) {
      spdlog::warn("Insert into platform_settings failed: {}",
                   query.lastError().text().toStdString());
    }
  }
} // namespace firelight::db
