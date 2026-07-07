#pragma once

#include <firelight/media/game_capture.hpp>

#include <optional>
#include <string>
#include <vector>

namespace firelight::media {

// The gallery's index of captured screenshots/clips.
class IGameCaptureRepository {
public:
  virtual ~IGameCaptureRepository() = default;

  // Inserts `capture` (a no-op if its filePath is already indexed) and sets its
  // id. Returns whether a valid id is now set.
  virtual bool add(GameCapture &capture) = 0;

  virtual std::vector<GameCapture> listAll() = 0;
  virtual std::vector<GameCapture>
  listForGame(const std::string &contentHash) = 0;
  virtual std::optional<GameCapture> getById(int id) = 0;

  virtual bool setFavorite(int id, bool favorite) = 0;
  virtual bool remove(int id) = 0;

  // Whether a capture with this file path is already indexed (used by reconcile).
  virtual bool existsForPath(const std::string &filePath) = 0;
};

} // namespace firelight::media
