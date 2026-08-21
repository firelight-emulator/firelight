#pragma once

#include <firelight/saves/savefile.hpp>
#include <firelight/saves/savefile_info.hpp>
#include <firelight/saves/suspend_point.hpp>

#include <future>
#include <optional>
#include <string>
#include <vector>

namespace firelight::saves {

// The save/suspend-point persistence contract. A plain domain interface: it
// carries no Qt notification concerns (the QML save-directory binding lives in
// QtSaveManagerProxy, and suspend-point changes are announced through the
// EventDispatcher — see save_events.hpp). Content hashes and the save directory
// are std::string; SuspendPoint's screenshot (QImage) is an accepted boundary
// type shared with the media/screenshot code
class ISaveManager {
public:
  virtual ~ISaveManager() = default;

  [[nodiscard]] virtual std::vector<SavefileInfo> getSaveFileInfoList(const std::string &contentHash) const = 0;

  virtual std::future<bool> writeSaveData(const std::string &contentHash, int saveSlotNumber,
                                          const Savefile &saveData) = 0;

  [[nodiscard]] virtual std::optional<Savefile> readSaveData(const std::string &contentHash,
                                                             int saveSlotNumber) const = 0;

  virtual void writeSuspendPoint(const std::string &contentHash, int saveSlotNumber, int index,
                                 const SuspendPoint &suspendPoint) = 0;

  virtual std::optional<SuspendPoint> readSuspendPoint(const std::string &contentHash, int saveSlotNumber,
                                                       int index) = 0;

  virtual void deleteSuspendPoint(const std::string &contentHash, int saveSlotNumber, int index) = 0;

  /**
   * Moves everything saved under one content hash to another, for when two library entries turn
   * out to be one game. Slots already present at the destination are left alone rather than
   * overwritten, because losing a save is worse than leaving a duplicate behind
   *
   * @return True when there was something to move and it moved
   */
  virtual bool transferSaves(const std::string &fromContentHash, const std::string &toContentHash) = 0;

  [[nodiscard]] virtual std::string getSaveDirectory() const = 0;

  // The one directory handed to cores for their own file writes. libretro gives
  // a core a single save directory, so it is kept apart from the per-game tree
  // this manager owns
  [[nodiscard]] virtual std::string getSharedCoreSaveDirectory() const = 0;

  virtual void setSaveDirectory(const std::string &saveDirectory) = 0;
};
} // namespace firelight::saves
