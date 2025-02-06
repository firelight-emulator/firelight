#pragma once

#include "firelight/activity/activity_log.hpp"

#include <QSqlDatabase>

namespace firelight::activity {
    class SqliteActivityLog final : public IActivityLog {
    public:
      explicit SqliteActivityLog(QString dbPath);

        bool createPlaySession(PlaySession &session) override;

        std::optional<PlaySession> getLatestPlaySession(std::string contentHash) override;

        [[nodiscard]] QSqlDatabase getDatabase() const;
    private:
        QString databasePath;
    };
} // activity
// firelight
