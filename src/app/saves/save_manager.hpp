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

#include "../../gui/models/suspend_point_list_model.hpp"
#include "suspend_point.hpp"

#include <qdir.h>
#include <qsettings.h>

namespace firelight::saves {
    class SaveManager final : public QObject {
        Q_OBJECT
        Q_PROPERTY(QString saveDirectory READ getSaveDirectory WRITE setSaveDirectory NOTIFY saveDirectoryChanged)

    public:
        SaveManager(const QString &defaultSaveDir,
                    db::ILibraryDatabase &libraryDatabase,
                    db::IUserdataDatabase &userdataDatabase,
                    gui::GameImageProvider &gameImageProvider);

        ~SaveManager() override;

        QFuture<bool> writeSaveData(const QString &contentHash, int saveSlotNumber,
                                    const Savefile &saveData);

        [[nodiscard]] std::optional<Savefile> readSaveData(const QString &contentHash, int saveSlotNumber) const;


        QFuture<bool> writeSuspendPoint(const QString &contentHash, int saveSlotNumber, int index,
                                        const SuspendPoint &suspendPoint);

        std::optional<SuspendPoint> readSuspendPoint(const QString &contentHash, int saveSlotNumber, int index);

        [[nodiscard]] QString getSaveDirectory() const;

        void setSaveDirectory(const QString &saveDirectory);

        Q_INVOKABLE QAbstractListModel *getSuspendPointListModel(const QString &contentHash, int saveSlotNumber = -1);

        Q_INVOKABLE void clearSuspendPointListModel();

    public slots:
        void handleUpdatedSuspendPoint(int index);

    signals:
        void saveDirectoryChanged(const QString &saveDirectory);

    private:
        void writeSuspendPointToDisk(const QString &contentHash, int index,
                                     const SuspendPoint &suspendPoint);

        std::optional<SuspendPoint> readSuspendPointFromDisk(const QString &contentHash, int saveSlotNumber, int index);

        void deleteSuspendPointFromDisk(const QString &contentHash, int saveSlotNumber, int index);

        QString m_currentSuspendPointListContentHash;
        int m_currentSuspendPointListSaveSlotNumber = -1;
        std::unique_ptr<emulation::SuspendPointListModel> m_suspendPointListModel;

        QSettings m_settings;

        db::ILibraryDatabase &m_libraryDatabase;
        db::IUserdataDatabase &m_userdataDatabase;
        gui::GameImageProvider &m_gameImageProvider;
        QString m_saveDirectory;
        std::unique_ptr<QThreadPool> m_ioThreadPool = nullptr;
        std::map<int, SuspendPoint> m_suspendPoints;
    };
} // namespace firelight::saves
