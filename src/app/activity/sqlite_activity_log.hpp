#pragma once

#include "firelight/activity_log.hpp"


namespace firelight::activity {
    class SqliteActivityLog : public IActivityLog {
    public:
        bool createPlaySession(db::PlaySession &session) override;

        std::optional<db::PlaySession> getLatestPlaySession(const std::string &contentHash) override;
    };
} // activity
// firelight
