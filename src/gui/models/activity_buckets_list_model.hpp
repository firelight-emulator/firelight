#pragma once
#include <QAbstractListModel>
#include <firelight/activity/play_session.hpp>

#include "service_accessor.hpp"

namespace firelight::gui {
    class ActivityBucketsListModel : public QAbstractListModel, ServiceAccessor {
        Q_OBJECT
        Q_PROPERTY(int count READ getCount NOTIFY countChanged)
        Q_PROPERTY(int totalPlaytimeSeconds READ getTotalPlaytimeSeconds NOTIFY countChanged)
        Q_PROPERTY(Granularity granularity READ getGranularity WRITE setGranularity NOTIFY granularityChanged)

    public:
        enum Granularity {
            Day = 0,
            Week,
            Month,
            Year
        };

        explicit ActivityBucketsListModel(QObject *parent = nullptr);

        [[nodiscard]] int getCount() const { return m_bucketsByTimeframe.size(); }

        [[nodiscard]] int getTotalPlaytimeSeconds() const;

        [[nodiscard]] Granularity getGranularity() const { return m_granularity; }

        void setGranularity(Granularity granularity);

        [[nodiscard]] int rowCount(const QModelIndex &parent) const override { return m_bucketsByTimeframe.size(); }

        Q_INVOKABLE QVariantMap get(int row) const;

        [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

        [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    signals:
        void countChanged();

        void granularityChanged();

    private:
        enum Roles {
            Label = Qt::UserRole + 1,
            PlaytimeSeconds,
            NumberOfBuckets,
            YAxisValues,
            HighestAxisValue,
            MaxPlaytimeSeconds,
            Buckets,
            GameActivityRecords
        };

        struct Thing {
            QString displayName;
            QString iconSourceUrl;
            int platformId;
            QString platformName;
            int secondsPlayed;
            float percentPlayed; // Percent of playtime in its bucket
        };

        struct Bucket {
            QString label;
            int numberOfRecords;
            int totalPlaytimeSeconds;
            int totalPlaytimeMinutes;
            QVector<Thing> gameActivityRecords;
        };

        struct BucketRange {
            QString label;
            int totalPlaytimeSeconds;
            int highestAxisValue;
            int maxPlaytimeSeconds;
            QVariantList yAxisValues;
            QVector<Bucket> buckets;
            QVector<Thing> gameActivityRecords; // Used to track top X most played games in the entire range
        };

        void refreshData();

        Granularity m_granularity{Day};
        QVector<BucketRange> m_bucketsByTimeframe; // For example, buckets grouped by day, week, month, year, all time
        std::vector<activity::PlaySession> m_allSessions;
        QVariantList m_yAxisValues;
    };
} // gui
// firelight
