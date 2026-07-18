#pragma once
#include <QSortFilterProxyModel>
#include <qtmetamacros.h>

namespace firelight::library {
class UserLibraryService;
}

namespace firelight::platforms {
class PlatformService;
}

namespace firelight::gui {
class SearchResultsListModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
  SearchResultsListModel(firelight::library::UserLibraryService &library,
                         firelight::platforms::PlatformService &platformService, QObject *parent = nullptr)
      : QAbstractListModel(parent), m_library(&library), m_platformService(&platformService) {
    refreshItems();
  }

  void refreshItems();

  Q_INVOKABLE void setFilterString(const QString &pattern);

  Q_PROPERTY(int maxResultsPerCategory READ maxResultsPerCategory WRITE setMaxResultsPerCategory NOTIFY
                 maxResultsPerCategoryChanged)

  [[nodiscard]] int maxResultsPerCategory() const { return m_maxResultsPerCategory; }

  void setMaxResultsPerCategory(int max);

  [[nodiscard]] int count() const;

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;

  [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

signals:
  void maxResultsPerCategoryChanged();

  void countChanged();

private:
  enum Roles {
    DisplayName = Qt::UserRole + 1,
    Category,
    IconSourceUrl,
    ItemType,
    PlatformName,
    PlatformShortName,
    PlatformIconSourceUrl,
    MatchScore
  };

  struct Item {
    QString displayName;
    QString category;
    QString iconSourceUrl;
    QString itemType;
    QString platformName;
    QString platformShortName;
    QString platformIconSourceUrl;
    int matchScore;
  };

  firelight::library::UserLibraryService *m_library;
  firelight::platforms::PlatformService *m_platformService;

  QVector<Item> m_allItems;
  QVector<Item> m_visibleItems;

  QString m_currentPattern;
  int m_maxResultsPerCategory = 5; // 0 = unlimited

  void trimToLimit();

  static int calculateFuzzyScore(const QString &text, const QString &pattern) {
    const QString t = text.toLower();
    const QString p = pattern.toLower();

    int score = 0;
    int tIdx = 0;
    int pIdx = 0;
    int consecutiveMatches = 0;

    while (tIdx < t.length() && pIdx < p.length()) {
      if (t.at(tIdx) == p.at(pIdx)) {
        score += 10; // Base match points
        if (consecutiveMatches > 0) {
          score += 5; // Consecutive streak bonus
        }

        // Boundary Bonus: Starts of words, camelCase capitals, or snake_case underscores
        if (tIdx == 0 || text.at(tIdx).isUpper() || text.at(tIdx - 1) == '_' || text.at(tIdx - 1) == ' ') {
          score += 20;
        }

        pIdx++;
        consecutiveMatches++;
      } else {
        consecutiveMatches = 0;
        score -= 1; // Penalty for loose, wide character gaps
      }
      tIdx++;
    }

    return (pIdx == p.length()) ? score : -1;
  }
};
} // namespace firelight::gui
