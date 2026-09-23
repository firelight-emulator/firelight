#pragma once

#include "play_session.hpp"

#include <optional>
#include <string>
#include <vector>

namespace firelight::activity {
/**
 * Stores play session data :)
 */
class IActivityLog {
public:
  virtual ~IActivityLog() = default;

  virtual bool createPlaySession(PlaySession &session) = 0;

  virtual std::optional<PlaySession> getLatestPlaySession(std::string contentHash) = 0;

  virtual std::vector<PlaySession> getPlaySessions(std::string contentHash) = 0;

  virtual std::vector<PlaySession> getPlaySessions() = 0;

  /**
   * Re-keys every session from one content hash to another, keeping all play sessions intact
   */
  virtual bool transferSessions(const std::string &fromContentHash, const std::string &toContentHash) = 0;
};
} // namespace firelight::activity
