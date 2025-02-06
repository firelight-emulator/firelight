#pragma once

#include "play_session.hpp"
#include <optional>


namespace firelight::activity {
    class IActivityLog {
    protected:
        virtual ~IActivityLog() = default;

    public:
        virtual bool createPlaySession(PlaySession &session) = 0;

        virtual std::optional<PlaySession> getLatestPlaySession(std::string contentHash) = 0;
    };
}
