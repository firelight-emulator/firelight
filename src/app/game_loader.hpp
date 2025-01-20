#pragma once
#include <QObject>

#include "manager_accessor.hpp"

namespace firelight {
    class GameLoader final : public QObject, public ManagerAccessor {
        Q_OBJECT

    public:
        GameLoader();

    public
    slots:
        void loadEntry(int entryId, bool waitForApproval = false);

        void approve();

        void emitResult();

    signals:
        void gameLoaded(int entryId, const QByteArray &gameData, const QByteArray &saveData, QString corePath,
                        QString contentHash,
                        unsigned int saveSlotNumber, unsigned int platformId, QString contentPath);

    private:
        QThreadPool m_threadPool;

        int m_entryId;
        QByteArray m_gameData;
        QByteArray m_saveData;
        QString m_corePath;
        QString m_contentHash;
        unsigned int m_saveSlotNumber;
        unsigned int m_platformId;
        QString m_contentPath;

        bool m_lastLoadSuccessful = false;
        bool m_holding = false;
    };
} // firelight
