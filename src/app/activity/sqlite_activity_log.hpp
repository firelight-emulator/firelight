#pragma once

#include "firelight/activity/activity_log.hpp"


namespace firelight::activity {
    class SqliteActivityLog final : public IActivityLog {
    public:
        bool createPlaySession(db::PlaySession &session) override;

        std::optional<db::PlaySession> getLatestPlaySession(std::string contentHash) override;
    };
} // activity
// firelight
