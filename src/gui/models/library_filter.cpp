// TODO: NEEDS REVIEW
#include "library_filter.hpp"

namespace firelight::gui {
namespace {

// TODO
// Reads an optional criterion as the enum QML sees
LibraryFilter::Tristate toTristate(const std::optional<bool> &value) {
  if (!value.has_value()) {
    return LibraryFilter::Unset;
  }

  return *value ? LibraryFilter::Yes : LibraryFilter::No;
}

// TODO
// The other direction. Unset clears the criterion rather than storing a third value
std::optional<bool> fromTristate(const LibraryFilter::Tristate value) {
  if (value == LibraryFilter::Unset) {
    return std::nullopt;
  }

  return value == LibraryFilter::Yes;
}

QVariantList toVariantList(const std::vector<int> &values) {
  QVariantList list;
  list.reserve(static_cast<qsizetype>(values.size()));

  for (const auto value : values) {
    list.append(value);
  }

  return list;
}

std::vector<int> toIntVector(const QVariantList &values) {
  std::vector<int> result;
  result.reserve(static_cast<size_t>(values.size()));

  for (const auto &value : values) {
    result.push_back(value.toInt());
  }

  return result;
}

} // namespace

LibraryFilter::LibraryFilter(QObject *parent) : QObject(parent) {}

const library::SmartFolderCriteria &LibraryFilter::getCriteria() const { return m_criteria; }

void LibraryFilter::setCriteria(const library::SmartFolderCriteria &criteria) {
  if (m_criteria == criteria) {
    return;
  }

  m_criteria = criteria;

  emit contentDirectoryIdsChanged();
  emit pathContainsChanged();
  emit platformIdsChanged();
  emit nameContainsChanged();
  emit favoriteChanged();
  emit playableChanged();
  emit unplayedChanged();
  emit genresChanged();
  emit developerChanged();
  emit publisherChanged();
  emit yearMinChanged();
  emit yearMaxChanged();
  emit playedAfterMillisChanged();
  emit minMinutesPlayedChanged();
  emit playedWithinDaysChanged();
  emit changed();
}

QVariantList LibraryFilter::getContentDirectoryIds() const { return toVariantList(m_criteria.contentDirectoryIds); }

void LibraryFilter::setContentDirectoryIds(const QVariantList &ids) {
  auto values = toIntVector(ids);

  if (m_criteria.contentDirectoryIds == values) {
    return;
  }

  m_criteria.contentDirectoryIds = std::move(values);
  emit contentDirectoryIdsChanged();
  emit changed();
}

QString LibraryFilter::getPathContains() const { return QString::fromStdString(m_criteria.pathContains); }

void LibraryFilter::setPathContains(const QString &value) {
  if (getPathContains() == value) {
    return;
  }

  m_criteria.pathContains = value.toStdString();
  emit pathContainsChanged();
  emit changed();
}

QVariantList LibraryFilter::getPlatformIds() const { return toVariantList(m_criteria.platformIds); }

void LibraryFilter::setPlatformIds(const QVariantList &ids) {
  auto values = toIntVector(ids);

  if (m_criteria.platformIds == values) {
    return;
  }

  m_criteria.platformIds = std::move(values);
  emit platformIdsChanged();
  emit changed();
}

QString LibraryFilter::getNameContains() const { return QString::fromStdString(m_criteria.nameContains); }

void LibraryFilter::setNameContains(const QString &value) {
  if (getNameContains() == value) {
    return;
  }

  m_criteria.nameContains = value.toStdString();
  emit nameContainsChanged();
  emit changed();
}

LibraryFilter::Tristate LibraryFilter::getFavorite() const { return toTristate(m_criteria.favorite); }

void LibraryFilter::setFavorite(const Tristate value) {
  if (getFavorite() == value) {
    return;
  }

  m_criteria.favorite = fromTristate(value);
  emit favoriteChanged();
  emit changed();
}

LibraryFilter::Tristate LibraryFilter::getPlayable() const { return toTristate(m_criteria.playable); }

void LibraryFilter::setPlayable(const Tristate value) {
  if (getPlayable() == value) {
    return;
  }

  m_criteria.playable = fromTristate(value);
  emit playableChanged();
  emit changed();
}

LibraryFilter::Tristate LibraryFilter::getUnplayed() const { return toTristate(m_criteria.unplayed); }

void LibraryFilter::setUnplayed(const Tristate value) {
  if (getUnplayed() == value) {
    return;
  }

  m_criteria.unplayed = fromTristate(value);
  emit unplayedChanged();
  emit changed();
}

QStringList LibraryFilter::getGenres() const {
  QStringList genres;
  genres.reserve(static_cast<qsizetype>(m_criteria.genres.size()));

  for (const auto &genre : m_criteria.genres) {
    genres.append(QString::fromStdString(genre));
  }

  return genres;
}

void LibraryFilter::setGenres(const QStringList &genres) {
  std::vector<std::string> values;
  values.reserve(static_cast<size_t>(genres.size()));

  for (const auto &genre : genres) {
    values.push_back(genre.toStdString());
  }

  if (m_criteria.genres == values) {
    return;
  }

  m_criteria.genres = std::move(values);
  emit genresChanged();
  emit changed();
}

QString LibraryFilter::getDeveloper() const { return QString::fromStdString(m_criteria.developer); }

void LibraryFilter::setDeveloper(const QString &value) {
  if (getDeveloper() == value) {
    return;
  }

  m_criteria.developer = value.toStdString();
  emit developerChanged();
  emit changed();
}

QString LibraryFilter::getPublisher() const { return QString::fromStdString(m_criteria.publisher); }

void LibraryFilter::setPublisher(const QString &value) {
  if (getPublisher() == value) {
    return;
  }

  m_criteria.publisher = value.toStdString();
  emit publisherChanged();
  emit changed();
}

int LibraryFilter::getYearMin() const { return m_criteria.yearMin.value_or(NO_YEAR); }

void LibraryFilter::setYearMin(const int year) {
  if (getYearMin() == year) {
    return;
  }

  m_criteria.yearMin = year == NO_YEAR ? std::nullopt : std::optional(year);
  emit yearMinChanged();
  emit changed();
}

int LibraryFilter::getYearMax() const { return m_criteria.yearMax.value_or(NO_YEAR); }

void LibraryFilter::setYearMax(const int year) {
  if (getYearMax() == year) {
    return;
  }

  m_criteria.yearMax = year == NO_YEAR ? std::nullopt : std::optional(year);
  emit yearMaxChanged();
  emit changed();
}

qint64 LibraryFilter::getPlayedAfterMillis() const {
  return static_cast<qint64>(m_criteria.playedAfterMillis.value_or(NO_TIMESTAMP));
}

void LibraryFilter::setPlayedAfterMillis(const qint64 millis) {
  if (getPlayedAfterMillis() == millis) {
    return;
  }

  m_criteria.playedAfterMillis = millis == NO_TIMESTAMP ? std::nullopt : std::optional(static_cast<int64_t>(millis));
  emit playedAfterMillisChanged();
  emit changed();
}

int LibraryFilter::getMinMinutesPlayed() const {
  if (!m_criteria.minSecondsPlayed.has_value()) {
    return NO_MINUTES;
  }

  return static_cast<int>(*m_criteria.minSecondsPlayed / 60);
}

void LibraryFilter::setMinMinutesPlayed(const int minutes) {
  if (getMinMinutesPlayed() == minutes) {
    return;
  }

  m_criteria.minSecondsPlayed = minutes < 0 ? std::nullopt : std::optional(static_cast<int64_t>(minutes) * 60);
  emit minMinutesPlayedChanged();
  emit changed();
}

int LibraryFilter::getPlayedWithinDays() const { return m_criteria.playedWithinDays.value_or(NO_DAYS); }

void LibraryFilter::setPlayedWithinDays(const int days) {
  if (getPlayedWithinDays() == days) {
    return;
  }

  m_criteria.playedWithinDays = days == NO_DAYS ? std::nullopt : std::optional(days);
  emit playedWithinDaysChanged();
  emit changed();
}

bool LibraryFilter::isEmpty() const { return m_criteria.isEmpty(); }

QString LibraryFilter::getJson() const { return QString::fromStdString(m_criteria.toJson()); }

void LibraryFilter::setJson(const QString &json) {
  setCriteria(library::SmartFolderCriteria::parse(json.toStdString()));
}

void LibraryFilter::clear() { setCriteria({}); }

void LibraryFilter::copyFrom(LibraryFilter *other) {
  if (other == nullptr) {
    return;
  }

  setCriteria(other->getCriteria());
}

} // namespace firelight::gui
