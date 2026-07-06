#pragma once

#include <firelight/metadata/media_asset_repository.hpp>

#include <memory>
#include <mutex>
#include <string>

namespace SQLite {
    class Database;
}

namespace firelight::metadata {
    /**
     * Sqlite implementation of the IMediaAssetRepository interface.
     */
    class SqliteMediaAssetRepository final : public IMediaAssetRepository {
    public:
        explicit SqliteMediaAssetRepository(std::string databaseFile);

        ~SqliteMediaAssetRepository() override;

        std::vector<MediaAsset>
        listForContent(const std::string &contentHash) override;

        std::vector<MediaAsset>
        listForContentAndType(const std::string &contentHash, MediaType type) override;

        std::optional<MediaAsset> selectedFor(const std::string &contentHash,
                                              MediaType type) override;

        bool add(MediaAsset &asset) override;

        bool setSelected(int assetId) override;

        bool remove(int assetId) override;

    private:
        std::string m_databaseFile;
        std::unique_ptr<SQLite::Database> m_db;

        mutable std::recursive_mutex m_mutex; // Protect access to the database connection and operations
    };
} // namespace firelight::metadata
