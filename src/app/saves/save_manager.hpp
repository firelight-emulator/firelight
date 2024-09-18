#pragma once

#include "firelight/library_entry.hpp"
#include "firelight/userdata_database.hpp"
#include "savefile.hpp"
#include <filesystem>
#include <qfuture.h>

#include "suspend_point.hpp"
#include "../../gui/models/rewind_model.hpp"

namespace firelight::saves {
    class SaveManager : public QObject {
    public:
        SaveManager(std::filesystem::path saveDir,
                    db::IUserdataDatabase &userdataDatabase);

        QFuture<bool> writeSaveDataForEntry(db::LibraryEntry &entry,
                                            Savefile &saveData) const;

        std::optional<Savefile> readSaveDataForEntry(db::LibraryEntry &entry) const;

        QFuture<bool> writeSuspendPointForEntry(db::LibraryEntry &entry, int index, SuspendPoint &suspendPoint) const;

        std::optional<SuspendPoint> readSuspendPointForEntry(db::LibraryEntry &entry, int index) const;

    private:
        db::IUserdataDatabase &m_userdataDatabase;
        std::filesystem::path m_saveDir;
        std::unique_ptr<QThreadPool> m_ioThreadPool = nullptr;
    };
} // namespace firelight::saves
