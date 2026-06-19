#include "search_results_list_model.hpp"

#include <library/user_library.hpp>
#include <platforms/platform_service.hpp>

#include "util/fuzzy_string_matcher.hpp"

namespace firelight::gui {
    static int categoryOrder(const QString &category) {
        if (category == "Games") return 0;
        if (category == "Folders") return 1;
        return 2;
    }

    // Removes items beyond m_maxResultsPerCategory within each category.
    // Must be called after m_visibleItems is sorted by category.
    void SearchResultsListModel::trimToLimit() {
        if (m_maxResultsPerCategory <= 0) return;
        QString current;
        int count = 0;
        for (int i = 0; i < m_visibleItems.size();) {
            const QString &cat = m_visibleItems[i].category;
            if (cat != current) {
                current = cat;
                count = 1;
                ++i;
            } else if (++count > m_maxResultsPerCategory) {
                m_visibleItems.removeAt(i);
            } else {
                ++i;
            }
        }
    }

    void SearchResultsListModel::setMaxResultsPerCategory(int max) {
        if (m_maxResultsPerCategory == max) return;
        m_maxResultsPerCategory = max;
        emit maxResultsPerCategoryChanged();
        setFilterString(m_currentPattern);
    }

    int SearchResultsListModel::count() const {
        return m_visibleItems.size();
    }

    void SearchResultsListModel::refreshItems() {
        beginResetModel();
        m_allItems.clear();

        for (const auto &entry: getLibraryService()->getEntries(0, 0)) {
            if (entry.hidden) continue;

            Item item;
            item.displayName = entry.displayName;
            item.category = "Games";
            item.iconSourceUrl = entry.icon1x1SourceUrl;
            item.itemType = "LibraryEntry";
            item.platformName = QString::fromStdString(getPlatformService()->getPlatform(entry.platformId)->name);
            item.platformShortName = QString::fromStdString(getPlatformService()->getPlatform(entry.platformId)->slug);
            item.platformIconSourceUrl = QString::fromStdString(
                getPlatformService()->getPlatform(entry.platformId)->slug);
            m_allItems.append(item);
        }

        for (const auto &folder: getLibraryService()->listFolders({})) {
            Item item;
            item.displayName = QString::fromStdString(folder.displayName);
            item.category = "Folders";
            item.iconSourceUrl = QString::fromStdString(folder.iconSourceUrl);
            item.itemType = "LibraryFolder";
            m_allItems.append(item);
        }

        // Seed visible items: group by category, alphabetical within each group
        m_visibleItems.clear();
        m_visibleItems = m_allItems;
        std::ranges::sort(m_visibleItems, [](const Item &a, const Item &b) {
            const int ca = categoryOrder(a.category), cb = categoryOrder(b.category);
            if (ca != cb) return ca < cb;
            return a.displayName < b.displayName;
        });
        trimToLimit();
        endResetModel();
    }

    void SearchResultsListModel::setFilterString(const QString &pattern) {
        m_currentPattern = pattern;
        beginResetModel();
        m_visibleItems.clear();

        if (pattern.isEmpty()) {
            m_visibleItems = m_allItems;
            std::ranges::sort(m_visibleItems, [](const Item &a, const Item &b) {
                const int ca = categoryOrder(a.category), cb = categoryOrder(b.category);
                if (ca != cb) return ca < cb;
                return a.displayName < b.displayName;
            });
        } else {
            for (Item item: m_allItems) {
                const int score = calculateFuzzyScore(item.displayName, pattern);
                if (score >= 0) {
                    item.matchScore = score;
                    m_visibleItems.append(item);
                }
            }
            std::ranges::sort(m_visibleItems, [](const Item &a, const Item &b) {
                const int ca = categoryOrder(a.category), cb = categoryOrder(b.category);
                if (ca != cb) return ca < cb;
                if (a.matchScore != b.matchScore) return a.matchScore > b.matchScore;
                return a.displayName < b.displayName;
            });
        }

        trimToLimit();
        emit countChanged();
        endResetModel();
    }

    int SearchResultsListModel::rowCount(const QModelIndex &parent) const {
        return m_visibleItems.count();
    }

    QVariant SearchResultsListModel::data(const QModelIndex &index, int role) const {
        if (!index.isValid() || index.row() >= m_visibleItems.count()) {
            return {};
        }

        const auto &item = m_visibleItems.at(index.row());

        switch (role) {
            case DisplayName: return item.displayName;
            case Category: return item.category;
            case IconSourceUrl: return item.iconSourceUrl;
            case ItemType: return item.itemType;
            case PlatformName: return item.platformName;
            case PlatformShortName: return item.platformShortName;
            case PlatformIconSourceUrl: return item.platformIconSourceUrl;
            case MatchScore: return item.matchScore;
            default: break;
        }

        return {};
    }

    QHash<int, QByteArray> SearchResultsListModel::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[DisplayName] = "displayName";
        roles[Category] = "category";
        roles[IconSourceUrl] = "iconSourceUrl";
        roles[ItemType] = "itemType";
        roles[PlatformName] = "platformName";
        roles[PlatformShortName] = "platformShortName";
        roles[PlatformIconSourceUrl] = "platformIconSourceUrl";
        roles[MatchScore] = "matchScore";
        return roles;
    }
}
