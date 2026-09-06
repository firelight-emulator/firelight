#pragma once

#include <firelight/library/smart_folder.hpp>

#include <QObject>
#include <QStringList>
#include <QVariantList>

namespace firelight::gui {

/**
 * A set of library filter criteria, editable from QML.
 */
class LibraryFilter : public QObject {
  Q_OBJECT

  Q_PROPERTY(QVariantList contentDirectoryIds READ getContentDirectoryIds WRITE setContentDirectoryIds NOTIFY
                 contentDirectoryIdsChanged)
  Q_PROPERTY(QString pathContains READ getPathContains WRITE setPathContains NOTIFY pathContainsChanged)

  Q_PROPERTY(QVariantList platformIds READ getPlatformIds WRITE setPlatformIds NOTIFY platformIdsChanged)
  Q_PROPERTY(QString nameContains READ getNameContains WRITE setNameContains NOTIFY nameContainsChanged)
  Q_PROPERTY(Tristate favorite READ getFavorite WRITE setFavorite NOTIFY favoriteChanged)
  Q_PROPERTY(Tristate playable READ getPlayable WRITE setPlayable NOTIFY playableChanged)
  Q_PROPERTY(Tristate unplayed READ getUnplayed WRITE setUnplayed NOTIFY unplayedChanged)
  Q_PROPERTY(QStringList genres READ getGenres WRITE setGenres NOTIFY genresChanged)
  Q_PROPERTY(QString developer READ getDeveloper WRITE setDeveloper NOTIFY developerChanged)
  Q_PROPERTY(QString publisher READ getPublisher WRITE setPublisher NOTIFY publisherChanged)
  Q_PROPERTY(int yearMin READ getYearMin WRITE setYearMin NOTIFY yearMinChanged)
  Q_PROPERTY(int yearMax READ getYearMax WRITE setYearMax NOTIFY yearMaxChanged)
  Q_PROPERTY(
      qint64 playedAfterMillis READ getPlayedAfterMillis WRITE setPlayedAfterMillis NOTIFY playedAfterMillisChanged)
  Q_PROPERTY(int minMinutesPlayed READ getMinMinutesPlayed WRITE setMinMinutesPlayed NOTIFY minMinutesPlayedChanged)
  Q_PROPERTY(int playedWithinDays READ getPlayedWithinDays WRITE setPlayedWithinDays NOTIFY playedWithinDaysChanged)

  Q_PROPERTY(bool empty READ isEmpty NOTIFY changed)
  Q_PROPERTY(QString json READ getJson NOTIFY changed)
  Q_PROPERTY(QString savedJson READ getSavedJson WRITE setSavedJson NOTIFY savedJsonChanged)
  Q_PROPERTY(bool dirty READ isDirty NOTIFY dirtyChanged)

public:
  enum Tristate { Unset = -1, No = 0, Yes = 1 };
  Q_ENUM(Tristate)

  static constexpr int NO_YEAR = 0;
  static constexpr int NO_DAYS = 0;
  static constexpr qint64 NO_TIMESTAMP = 0;
  static constexpr int NO_MINUTES = -1;

  explicit LibraryFilter(QObject *parent = nullptr);

  [[nodiscard]] const library::SmartFolderCriteria &getCriteria() const;
  void setCriteria(const library::SmartFolderCriteria &criteria);

  [[nodiscard]] QVariantList getContentDirectoryIds() const;
  void setContentDirectoryIds(const QVariantList &ids);

  [[nodiscard]] QString getPathContains() const;
  void setPathContains(const QString &value);

  [[nodiscard]] QVariantList getPlatformIds() const;
  void setPlatformIds(const QVariantList &ids);

  [[nodiscard]] QString getNameContains() const;
  void setNameContains(const QString &value);

  [[nodiscard]] Tristate getFavorite() const;
  void setFavorite(Tristate value);

  [[nodiscard]] Tristate getPlayable() const;
  void setPlayable(Tristate value);

  [[nodiscard]] Tristate getUnplayed() const;
  void setUnplayed(Tristate value);

  [[nodiscard]] QStringList getGenres() const;
  void setGenres(const QStringList &genres);

  [[nodiscard]] QString getDeveloper() const;
  void setDeveloper(const QString &value);

  [[nodiscard]] QString getPublisher() const;
  void setPublisher(const QString &value);

  [[nodiscard]] int getYearMin() const;
  void setYearMin(int year);

  [[nodiscard]] int getYearMax() const;
  void setYearMax(int year);

  [[nodiscard]] qint64 getPlayedAfterMillis() const;
  void setPlayedAfterMillis(qint64 millis);

  // Minutes at the surface and seconds underneath, so the one place that conversion happens is here
  [[nodiscard]] int getMinMinutesPlayed() const;
  void setMinMinutesPlayed(int minutes);

  [[nodiscard]] int getPlayedWithinDays() const;
  void setPlayedWithinDays(int days);

  /**
   * @return Whether nothing is set, so the filter matches the whole library
   */
  [[nodiscard]] bool isEmpty() const;

  /**
   * @return The criteria as the JSON a smart folder stores
   */
  [[nodiscard]] QString getJson() const;

  /**
   * @return The criteria last saved, as JSON
   */
  [[nodiscard]] QString getSavedJson() const;

  void setSavedJson(const QString &savedJson);

  /**
   * @return Whether the criteria differ from the ones last saved. False when nothing was saved
   */
  [[nodiscard]] bool isDirty() const;

  /**
   * Replaces every criterion with what the JSON carries. Criteria this object has no property for
   * are preserved rather than dropped, which is what makes editing a saved folder lossless
   */
  Q_INVOKABLE void setJson(const QString &json);

  /**
   * Forgets every criterion
   */
  Q_INVOKABLE void clear();

  /**
   * Takes another filter's criteria wholesale, for saving the current view as a folder
   */
  Q_INVOKABLE void copyFrom(const LibraryFilter *other);

signals:
  void contentDirectoryIdsChanged();
  void pathContainsChanged();
  void platformIdsChanged();
  void nameContainsChanged();
  void favoriteChanged();
  void playableChanged();
  void unplayedChanged();
  void genresChanged();
  void developerChanged();
  void publisherChanged();
  void yearMinChanged();
  void yearMaxChanged();
  void playedAfterMillisChanged();
  void minMinutesPlayedChanged();
  void playedWithinDaysChanged();

  void changed();

  void savedJsonChanged();

  void dirtyChanged();

private:
  library::SmartFolderCriteria m_criteria;
  QString m_savedJson;
};

} // namespace firelight::gui
