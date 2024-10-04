#pragma once

#include "firelight/library_entry.hpp"
#include "firelight/userdata_database.hpp"
#include "savefile.hpp"
#include <filesystem>
#include <qfuture.h>
#include <map>
#include <firelight/content_database.hpp>
#include <firelight/library_database.hpp>
#include "../../gui/game_image_provider.hpp"

#include "suspend_point.hpp"
#include "../../gui/models/suspend_point_list_model.hpp"

namespace firelight::saves {
    class SaveManager : public QObject {
        Q_OBJECT

    public:
        SaveManager(std::filesystem::path saveDir,
                    db::ILibraryDatabase &libraryDatabase,
                    db::IUserdataDatabase &userdataDatabase,
                    gui::GameImageProvider &gameImageProvider);

        QFuture<bool> writeSaveData(const QString &contentHash, int saveSlotNumber,
                                    const Savefile &saveData);

        std::optional<Savefile> readSaveData(const QString &contentHash, int saveSlotNumber) const;


        QFuture<bool> writeSuspendPoint(const QString &contentHash, int saveSlotNumber, int index,
                                        const SuspendPoint &suspendPoint);

        std::optional<SuspendPoint> readSuspendPoint(const QString &contentHash, int saveSlotNumber, int index);

        Q_INVOKABLE QAbstractListModel *getSuspendPointListModel(const QString &contentHash, int saveSlotNumber = -1);

        Q_INVOKABLE void clearSuspendPointListModel();

    public slots:
        void handleUpdatedSuspendPoint(int index);

    private:
        void writeSuspendPointToDisk(const QString &contentHash, int index,
                                     const SuspendPoint &suspendPoint);

        std::optional<SuspendPoint> readSuspendPointFromDisk(const QString &contentHash, int saveSlotNumber, int index);

        void deleteSuspendPointFromDisk(const QString &contentHash, int saveSlotNumber, int index);

        QString m_currentSuspendPointListContentHash;
        int m_currentSuspendPointListSaveSlotNumber = -1;
        std::unique_ptr<emulation::SuspendPointListModel> m_suspendPointListModel;

        db::ILibraryDatabase &m_libraryDatabase;
        db::IUserdataDatabase &m_userdataDatabase;
        gui::GameImageProvider &m_gameImageProvider;
        std::filesystem::path m_saveDir;
        std::unique_ptr<QThreadPool> m_ioThreadPool = nullptr;
        std::map<int, SuspendPoint> m_suspendPoints;
    };
} // namespace firelight::saves
