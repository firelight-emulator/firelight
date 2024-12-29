#include "sqlite_controller_repository.hpp"

#include <QSqlQuery>
#include <QSqlError>
#include <QThread>
#include <utility>
#include <spdlog/spdlog.h>

constexpr auto DATABASE_PREFIX = "controllers_";

namespace firelight::input {
    SqliteControllerRepository::SqliteControllerRepository(QString dbFilePath) : m_dbFilePath(std::move(dbFilePath)) {
        const auto db = getDatabase();

        QSqlQuery createProfiles(db);
        createProfiles.prepare("CREATE TABLE IF NOT EXISTS profilesv1("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "controller_name TEXT NOT NULL,"
            "vendor_id INTEGER NOT NULL,"
            "product_id INTEGER NOT NULL,"
            "product_version_id INTEGER NOT NULL,"
            "created_at INTEGER NOT NULL);");

        if (!createProfiles.exec()) {
            spdlog::error("Table creation failed: {}", createProfiles.lastError().text().toStdString());
        }

        QSqlQuery createMappings(db);
        createMappings.prepare("CREATE TABLE IF NOT EXISTS mappingsv1("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "profile_id INTEGER NOT NULL,"
            "platform_id INTEGER NOT NULL,"
            "mapping_data TEXT,"
            "created_at INTEGER NOT NULL);");

        if (!createMappings.exec()) {
            spdlog::error("Table creation failed: {}", createMappings.lastError().text().toStdString());
        }
    }

    std::shared_ptr<ControllerProfile> SqliteControllerRepository::getControllerProfile(const int profileId) const {
        // if (m_profiles.contains(profileId)) {
        //     return std::make_shared<ControllerProfile>(m_profiles.at(profileId));
        // }

        return {};
    }

    std::shared_ptr<ControllerProfile> SqliteControllerRepository::getControllerProfile(const std::string name,
        const int vendorId, const int productId, const int productVersion) {
        for (const auto &profile: m_profiles) {
            if (profile->getVendorId() == vendorId && profile->getProductId() == productId &&
                profile->getProductVersion() == productVersion) {
                return profile;
            }
        }

        QSqlQuery query(getDatabase());
        query.prepare(
            "SELECT * FROM profilesv1 WHERE vendor_id = :vendorId AND product_id = :productId AND product_version_id = :productVersion");
        query.bindValue(":vendorId", vendorId);
        query.bindValue(":productId", productId);
        query.bindValue(":productVersion", productVersion);

        if (!query.exec() || !query.next()) {
            QSqlQuery insertQuery(getDatabase());
            insertQuery.prepare(
                "INSERT INTO profilesv1 (controller_name, vendor_id, product_id, product_version_id, created_at) VALUES (:controllerName, :vendorId, :productId, :productVersion, :createdAt)");
            insertQuery.bindValue(":controllerName", QString::fromStdString(name));
            insertQuery.bindValue(":vendorId", vendorId);
            insertQuery.bindValue(":productId", productId);
            insertQuery.bindValue(":productVersion", productVersion);
            insertQuery.bindValue(":createdAt",
                                  std::chrono::duration_cast<std::chrono::milliseconds>(
                                      std::chrono::system_clock::now().time_since_epoch())
                                  .count());

            if (!insertQuery.exec()) {
                spdlog::error("Insert failed: {}", insertQuery.lastError().text().toStdString());
                return {};
            }

            query.exec();
            query.next();
        }

        auto profile = std::make_shared<ControllerProfile>(
            query.value("id").toInt(),
            query.value("vendor_id").toInt(),
            query.value("product_id").toInt(),
            query.value("product_version_id").toInt()
        );
        m_profiles.emplace_back(profile);
        return profile;
    }

    std::shared_ptr<InputMapping> SqliteControllerRepository::getInputMapping(const int mappingId) const {
        for (const auto &mapping: m_inputMappings) {
            if (mapping->getId() == mappingId) {
                return mapping;
            }
        }

        return nullptr;
    }

    std::shared_ptr<InputMapping>
    SqliteControllerRepository::getInputMapping(const int profileId, const int platformId) {
        for (const auto &mapping: m_inputMappings) {
            if (mapping->getControllerProfileId() == profileId && mapping->getPlatformId() ==
                platformId) {
                return mapping;
            }
        }

        QSqlQuery query(getDatabase());
        query.prepare(
            "SELECT * FROM mappingsv1 WHERE profile_id = :profileId AND platform_id = :platformId");
        query.bindValue(":profileId", profileId);
        query.bindValue(":platformId", platformId);

        if (!query.exec() || !query.next()) {
            QSqlQuery insertQuery(getDatabase());
            insertQuery.prepare(
                "INSERT INTO mappingsv1 (profile_id, platform_id, created_at) VALUES (:profileId, :platformId, :createdAt)");
            insertQuery.bindValue(":profileId", profileId);
            insertQuery.bindValue(":platformId", platformId);
            insertQuery.bindValue(":createdAt",
                                  std::chrono::duration_cast<std::chrono::milliseconds>(
                                      std::chrono::system_clock::now().time_since_epoch())
                                  .count());

            if (!insertQuery.exec()) {
                spdlog::error("Insert failed: {}", insertQuery.lastError().text().toStdString());
                return {};
            }

            query.exec();
            query.next();
        }

        auto mapping = std::make_shared<InputMapping>([this](InputMapping &m) {
            QSqlQuery updateQuery(getDatabase());
            updateQuery.prepare("UPDATE mappingsv1 SET mapping_data = :mappingData WHERE id = :id");
            updateQuery.bindValue(":mappingData", QString::fromStdString(m.serialize()));
            updateQuery.bindValue(":id", m.getId());

            if (!updateQuery.exec()) {
                spdlog::error("Update failed: {}", updateQuery.lastError().text().toStdString());
            }
        });
        mapping->setId(query.value("id").toInt());
        mapping->setControllerProfileId(profileId);
        mapping->setPlatformId(platformId);

        const auto data = query.value("mapping_data");
        if (!data.isNull()) {
            mapping->deserialize(data.toString().toStdString());
        }

        m_inputMappings.emplace_back(mapping);
        return mapping;
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
} // input
