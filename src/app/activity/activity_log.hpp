#pragma once

#include "play_session.hpp"
#include <optional>
#include <string>
#include <vector>

namespace firelight::activity {
class IActivityLog {
protected:
  virtual ~IActivityLog() = default;

public:
  virtual bool createPlaySession(PlaySession &session) = 0;

  virtual std::optional<PlaySession>
  getLatestPlaySession(std::string contentHash) = 0;

  virtual std::vector<PlaySession> getPlaySessions(std::string contentHash) = 0;

  virtual std::vector<PlaySession> getPlaySessions() = 0;
};
} // namespace firelight::activity
