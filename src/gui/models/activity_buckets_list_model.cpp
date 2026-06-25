#include "activity_buckets_list_model.hpp"

#include <firelight/activity/activity_log.hpp>
#include <QDateTime>
#include <QMap>
#include <QTimeZone>
#include <library/user_library.hpp>
#include <platforms/platform_service.hpp>
#include <spdlog/spdlog.h>

namespace firelight::gui {
    ActivityBucketsListModel::ActivityBucketsListModel(QObject *parent) : QAbstractListModel(parent) {
        m_allSessions = getActivityService()->getPlaySessions();
        refreshData();
    }

    int ActivityBucketsListModel::getTotalPlaytimeSeconds() const {
        int total = 0;
        for (const auto &range: m_bucketsByTimeframe)
            total += range.totalPlaytimeSeconds;
        return total;
    }

    void ActivityBucketsListModel::setGranularity(const Granularity granularity) {
        if (m_granularity != granularity) {
            m_granularity = granularity;
            refreshData();
            emit granularityChanged();
        }
    }

    QVariant ActivityBucketsListModel::data(const QModelIndex &index, int role) const {
        if (!index.isValid() || index.row() >= m_bucketsByTimeframe.size()) {
            return {};
        }
        const auto &item = m_bucketsByTimeframe.at(index.row());
        switch (role) {
            case Label:
                return item.label;
            case PlaytimeSeconds:
                return item.totalPlaytimeSeconds;
            case NumberOfBuckets:
                return item.buckets.size();
            case YAxisValues: {
                return item.yAxisValues;
            }
            case HighestAxisValue:
                return item.highestAxisValue;
            case MaxPlaytimeSeconds:
                return item.maxPlaytimeSeconds;
            case Buckets: {
                QVariantList list;
                for (const auto &bucket: item.buckets) {
                    QVariantMap bucketMap;
                    bucketMap["label"] = bucket.label;
                    bucketMap["numberOfRecords"] = bucket.numberOfRecords;
                    bucketMap["totalPlaytimeSeconds"] = bucket.totalPlaytimeSeconds;
                    bucketMap["totalPlaytimeMinutes"] = bucket.totalPlaytimeMinutes;
                    bucketMap["highestAxisValue"] = item.highestAxisValue;

                    QVariantList gameActivityRecordsList;
                    for (const auto &record: bucket.gameActivityRecords) {
                        QVariantMap recordMap;
                        recordMap["displayName"] = record.displayName;
                        recordMap["iconSourceUrl"] = record.iconSourceUrl;
                        recordMap["platformId"] = record.platformId;
                        recordMap["platformName"] = record.platformName;
                        recordMap["secondsPlayed"] = record.secondsPlayed;
                        recordMap["percentPlayed"] = record.percentPlayed;
                        gameActivityRecordsList.append(recordMap);
                    }
                    bucketMap["gameActivityRecords"] = gameActivityRecordsList;
                    list.append(bucketMap);
                }
                return list;
            }
            case GameActivityRecords: {
                QVariantList list;
                for (const auto &record: item.gameActivityRecords) {
                    QVariantMap recordMap;
                    recordMap["displayName"] = record.displayName;
                    recordMap["iconSourceUrl"] = record.iconSourceUrl;
                    recordMap["platformId"] = record.platformId;
                    recordMap["platformName"] = record.platformName;
                    recordMap["secondsPlayed"] = record.secondsPlayed;
                    recordMap["percentPlayed"] = record.percentPlayed;
                    list.append(recordMap);
                }
                return list;
            }
            default:
                return {};
        }

        return {};
    }

    QVariantMap ActivityBucketsListModel::get(int row) const {
        if (row < 0 || row >= m_bucketsByTimeframe.size())
            return {};
        QVariantMap result;
        result["label"] = data(index(row, 0), Label);
        result["playtimeSeconds"] = data(index(row, 0), PlaytimeSeconds);
        result["numberOfBuckets"] = data(index(row, 0), NumberOfBuckets);
        result["yAxisValues"] = data(index(row, 0), YAxisValues);
        result["highestAxisValue"] = data(index(row, 0), HighestAxisValue);
        result["maxPlaytimeSeconds"] = data(index(row, 0), MaxPlaytimeSeconds);
        result["buckets"] = data(index(row, 0), Buckets);
        result["gameActivityRecords"] = data(index(row, 0), GameActivityRecords);
        return result;
    }

    QHash<int, QByteArray> ActivityBucketsListModel::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[Label] = "label";
        roles[PlaytimeSeconds] = "playtimeSeconds";
        roles[NumberOfBuckets] = "numberOfBuckets";
        roles[HighestAxisValue] = "highestAxisValue";
        roles[MaxPlaytimeSeconds] = "maxPlaytimeSeconds";
        roles[Buckets] = "buckets";
        roles[GameActivityRecords] = "gameActivityRecords";
        return roles;
    }

    void ActivityBucketsListModel::refreshData() {
        beginResetModel();
        m_bucketsByTimeframe.clear();

        QLocale locale = QLocale::system();

        if (m_granularity == Day && !m_allSessions.empty()) {
            uint64_t earliestMs = m_allSessions[0].startTime;
            for (const auto &session: m_allSessions)
                earliestMs = std::min(earliestMs, session.startTime);

            const QDate firstDay = QDateTime::fromMSecsSinceEpoch(
                static_cast<qint64>(earliestMs), QTimeZone::systemTimeZone()).date();
            const QDate today = QDate::currentDate();

            for (QDate day = firstDay; day <= today; day = day.addDays(1)) {
                const auto dayStartMs = static_cast<uint64_t>(
                    QDateTime(day, QTime(0, 0, 0), QTimeZone::systemTimeZone()).toMSecsSinceEpoch());

                BucketRange range;
                auto dayOfWeekString = locale.dayName(day.dayOfWeek(), QLocale::ShortFormat);
                auto monthString = locale.monthName(day.month(), QLocale::ShortFormat);

                range.label = QString("%1 %2 %3 (%4)").arg(QString::number(day.day()), monthString,
                                                           QString::number(day.year()), dayOfWeekString);
                qint64 dayTotalMs = 0;
                qint64 maxHourMs = 0;

                QMap<QString, qint64> totalBucketPlaytimeByGame;

                for (int hour = 0; hour < 24; ++hour) {
                    const uint64_t hourStartMs = dayStartMs + static_cast<uint64_t>(hour) * 3600000;
                    const uint64_t hourEndMs = hourStartMs + 3600000;

                    QMap<QString, qint64> playtimeByGame;
                    for (const auto &session: m_allSessions) {
                        if (session.endTime <= hourStartMs || session.startTime >= hourEndMs)
                            continue;

                        const uint64_t clippedStart = std::max(session.startTime, hourStartMs);
                        const uint64_t clippedEnd = std::min(session.endTime, hourEndMs);
                        playtimeByGame[QString::fromStdString(session.contentHash)] +=
                                static_cast<qint64>(clippedEnd - clippedStart);
                        totalBucketPlaytimeByGame[QString::fromStdString(session.contentHash)] += static_cast<qint64>(
                            clippedEnd - clippedStart);
                    }

                    qint64 hourTotalMs = 0;
                    for (const auto ms: playtimeByGame) {
                        hourTotalMs += ms;
                    }

                    dayTotalMs += hourTotalMs;
                    maxHourMs = std::max(maxHourMs, hourTotalMs);

                    Bucket bucket;
                    bucket.label = hour < 10 ? QString("0%1:00").arg(hour) : QString("%1:00").arg(hour);
                    bucket.totalPlaytimeSeconds = static_cast<int>(hourTotalMs / 1000);


                    for (auto it = playtimeByGame.constBegin(); it != playtimeByGame.constEnd(); ++it) {
                        const auto entry = getLibraryService()->getEntryWithContentHash(it.key());
                        const auto displayName = entry ? entry->displayName : it.key();
                        const auto platform = entry
                                                  ? getPlatformService()->getPlatform(entry->platformId)
                                                  : std::nullopt;

                        Thing thing;
                        thing.displayName = displayName;
                        thing.iconSourceUrl = entry ? entry->icon1x1SourceUrl : "";
                        thing.platformId = entry ? entry->platformId : -1;
                        thing.platformName = platform ? QString::fromStdString(platform->name) : "Unknown";
                        thing.secondsPlayed = static_cast<int>(it.value() / 1000);
                        thing.percentPlayed = hourTotalMs > 0
                                                  ? static_cast<float>(it.value()) / static_cast<float>(hourTotalMs)
                                                  : 0.0f;

                        bucket.gameActivityRecords.append(thing);
                    }

                    std::ranges::sort(bucket.gameActivityRecords,
                                      [](const Thing &a, const Thing &b) {
                                          return a.secondsPlayed > b.secondsPlayed;
                                      });

                    bucket.numberOfRecords = bucket.gameActivityRecords.size();
                    range.buckets.append(bucket);
                }

                for (auto it = totalBucketPlaytimeByGame.constBegin(); it != totalBucketPlaytimeByGame.constEnd(); ++
                     it) {
                    const auto entry = getLibraryService()->getEntryWithContentHash(it.key());
                    const auto displayName = entry ? entry->displayName : it.key();
                    const auto platform = entry
                                              ? getPlatformService()->getPlatform(entry->platformId)
                                              : std::nullopt;

                    Thing thing;
                    thing.displayName = displayName;
                    thing.iconSourceUrl = entry ? entry->icon1x1SourceUrl : "";
                    thing.platformId = entry ? entry->platformId : -1;
                    thing.platformName = platform ? QString::fromStdString(platform->name) : "Unknown";
                    thing.secondsPlayed = static_cast<int>(it.value() / 1000);
                    thing.percentPlayed = dayTotalMs > 0
                                              ? static_cast<float>(it.value()) / static_cast<float>(dayTotalMs)
                                              : 0.0f;

                    range.gameActivityRecords.append(thing);
                }

                std::ranges::sort(range.gameActivityRecords,
                                  [](const Thing &a, const Thing &b) {
                                      return a.secondsPlayed > b.secondsPlayed;
                                  });

                range.totalPlaytimeSeconds = static_cast<int>(dayTotalMs / 1000);
                range.highestAxisValue = maxHourMs;

                // Get upper range to the nearest hour for better graph scaling
                const auto maxHoursRounded = (maxHourMs + 3599999) / 3600000;
                range.maxPlaytimeSeconds = static_cast<int>(maxHoursRounded * 3600);

                // if only one hour, do minutes instead in 10 minute increments
                if (maxHoursRounded <= 1) {
                    for (int minute = 1; minute <= maxHoursRounded * 6; ++minute) {
                        range.yAxisValues.append(QString::fromStdString("0:" + std::to_string(minute * 10)));
                    }
                } else {
                    for (int hour = 1; hour <= maxHoursRounded; ++hour) {
                        range.yAxisValues.append(QString::fromStdString(std::to_string(hour) + ":00"));
                    }
                }

                // Round up max hour ms to nearest hour for better graph scaling
                // const auto maxHoursRounded = (maxHourMs + 3599999) / 3600000 * 3600000;
                // for (int hour = 0; hour < maxHoursRounded; ++hour) {
                //     spdlog::info("Adding y-axis value {} for day {}, maxHourMs: {}, maxHoursRounded: {}",
                //                  hour, day.toString().toStdString(), maxHourMs, maxHoursRounded);
                //     range.yAxisValues.append(hour);
                // }

                m_bucketsByTimeframe.append(range);
            }
        }

        endResetModel();
        emit countChanged();
    }
}
