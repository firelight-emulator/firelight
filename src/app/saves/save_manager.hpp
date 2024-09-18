#pragma once

#include "firelight/library_entry.hpp"
#include "firelight/userdata_database.hpp"
#include "savefile.hpp"
#include <filesystem>
#include <qfuture.h>
#include <map>
#include <firelight/content_database.hpp>
#include <firelight/library_database.hpp>

#include "suspend_point.hpp"
#include "../../gui/models/suspend_point_list_model.hpp"

namespace firelight::saves {
    class SaveManager : public QObject {
        Q_OBJECT

    public:
        SaveManager(std::filesystem::path saveDir,
                    db::ILibraryDatabase &libraryDatabase,
                    db::IUserdataDatabase &userdataDatabase);

        QFuture<bool> writeSaveDataForEntry(db::LibraryEntry &entry,
                                            Savefile &saveData) const;

        std::optional<Savefile> readSaveDataForEntry(db::LibraryEntry &entry) const;

        QFuture<bool> writeSuspendPointForEntry(db::LibraryEntry &entry, int index, const SuspendPoint &suspendPoint);

        std::optional<SuspendPoint> readSuspendPointForEntry(db::LibraryEntry &entry, int index);

        Q_INVOKABLE QAbstractListModel *getSuspendPointListModel(int entryId);

        Q_INVOKABLE void clearSuspendPointListModel();

    private:
        int m_currentSuspendPointListEntryId = -1;
        std::unique_ptr<emulation::SuspendPointListModel> m_suspendPointListModel;

        db::ILibraryDatabase &m_libraryDatabase;
        db::IUserdataDatabase &m_userdataDatabase;
        std::filesystem::path m_saveDir;
        std::unique_ptr<QThreadPool> m_ioThreadPool = nullptr;
        std::map<int, SuspendPoint> m_suspendPoints;
    };
} // namespace firelight::saves
