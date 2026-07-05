#pragma once

#include <firelight/saves/savefile.hpp>
#include <firelight/saves/savefile_info.hpp>
#include <firelight/saves/suspend_point.hpp>

#include <QString>
#include <future>
#include <optional>
#include <vector>

namespace firelight::saves {

// The save/suspend-point persistence contract. A plain domain interface: it
// carries no Qt notification concerns (the QML save-directory binding lives in
// QtSaveManagerProxy, and suspend-point changes are announced through the
// EventDispatcher — see save_events.hpp). QString/QImage(SuspendPoint) are kept
// as value/boundary types.
class ISaveManager {
public:
  virtual ~ISaveManager() = default;

  [[nodiscard]] virtual std::vector<SavefileInfo>
  getSaveFileInfoList(const QString &contentHash) const = 0;

  virtual std::future<bool> writeSaveData(const QString &contentHash,
                                          int saveSlotNumber,
                                          const Savefile &saveData) = 0;

  [[nodiscard]] virtual std::optional<Savefile>
  readSaveData(const QString &contentHash, int saveSlotNumber) const = 0;

  virtual void writeSuspendPoint(const QString &contentHash, int saveSlotNumber,
                                 int index,
                                 const SuspendPoint &suspendPoint) = 0;

  virtual std::optional<SuspendPoint>
  readSuspendPoint(const QString &contentHash, int saveSlotNumber,
                   int index) = 0;

  virtual void deleteSuspendPoint(const QString &contentHash,
                                  int saveSlotNumber, int index) = 0;

  [[nodiscard]] virtual QString getSaveDirectory() const = 0;
  virtual void setSaveDirectory(const QString &saveDirectory) = 0;
};
} // namespace firelight::saves
