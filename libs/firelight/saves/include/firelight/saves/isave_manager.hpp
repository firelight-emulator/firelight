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
// type shared with the media/screenshot code.
class ISaveManager {
public:
  virtual ~ISaveManager() = default;

  [[nodiscard]] virtual std::vector<SavefileInfo>
  getSaveFileInfoList(const std::string &contentHash) const = 0;

  virtual std::future<bool> writeSaveData(const std::string &contentHash,
                                          int saveSlotNumber,
                                          const Savefile &saveData) = 0;

  [[nodiscard]] virtual std::optional<Savefile>
  readSaveData(const std::string &contentHash, int saveSlotNumber) const = 0;

  virtual void writeSuspendPoint(const std::string &contentHash,
                                 int saveSlotNumber, int index,
                                 const SuspendPoint &suspendPoint) = 0;

  virtual std::optional<SuspendPoint>
  readSuspendPoint(const std::string &contentHash, int saveSlotNumber,
                   int index) = 0;

  virtual void deleteSuspendPoint(const std::string &contentHash,
                                  int saveSlotNumber, int index) = 0;

  // Copies SRAM saves — and only SRAM, never suspend points — from one content
  // hash's save directory to another, per slot, writing a destination slot only
  // when it has no save yet. Used to carry a mod's progress forward across a
  // save-compatible version update, where the patched content hash (and thus the
  // save directory) changes. Suspend points are deliberately not migrated: they
  // are tied to exact ROM/core state and cannot survive a patch change.
  // Returns the number of slots copied.
  virtual int copySavefilesForward(const std::string &fromContentHash,
                                   const std::string &toContentHash) = 0;

  [[nodiscard]] virtual std::string getSaveDirectory() const = 0;
  virtual void setSaveDirectory(const std::string &saveDirectory) = 0;
};
} // namespace firelight::saves
