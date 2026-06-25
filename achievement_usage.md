| File | Line | Usage |
|------|------|-------|
| `src/main.cpp` | 3 | `#include "app/achievements/gui/AchievementSetItem.hpp"` |
| `src/main.cpp` | 19 | `#include <firelight/achievement_service.hpp>` |
| `src/main.cpp` | 21 | `#include <sqlite_achievement_repository.hpp>` |
| `src/main.cpp` | 31 | `#include <rcheevos/ra_client.hpp>` |
| `src/main.cpp` | 59 | `#include "gui/qt_achievement_service_proxy.hpp"` |
| `src/main.cpp` | 229 | `rcheevos3.db` path string |
| `src/main.cpp` | 233 | `RetroAchievementsOfflineClient offlineRaClient(...)` |
| `src/main.cpp` | 235 | `achievements::RAClient raClient(...)` |
| `src/main.cpp` | 294–295 | `qmlRegisterType<RetroAchievementsGameItem>(...)` |
| `src/gui/qt_achievement_service_proxy.cpp` | 1 | `#include "qt_achievement_service_proxy.hpp"` |
| `src/gui/qt_achievement_service_proxy.cpp` | 3 | `#include <firelight/achievement_service.hpp>` |
| `src/app/manager_accessor.hpp` | 13 | `#include "../../libs/firelight/achievements/src/rcheevos/ra_client.hpp"` |
| `src/app/manager_accessor.hpp` | 29 | `setAchievementManager(achievements::RAClient*)` |
| `src/app/manager_accessor.hpp` | 55 | `static achievements::RAClient* getAchievementManager()` |
| `src/app/manager_accessor.hpp` | 77 | `static achievements::RAClient* m_achievementManager` |
| `src/app/manager_accessor.cpp` | 9 | `achievements::RAClient* ManagerAccessor::m_achievementManager` |
| `src/app/manager_accessor.cpp` | 35 | `achievements::RAClient* t_achievementManager` (param) |
| `src/app/manager_accessor.cpp` | 82 | `achievements::RAClient* ManagerAccessor::getAchievementManager()` |
| `src/app/emulation/emulator_instance.cpp` | 102–103 | `getAchievementManager()->loadGame(m_platformId, contentHash)` |
| `src/app/emulation/emulator_instance.cpp` | 128 | `getAchievementManager()->doFrame(m_core.get())` |
| `src/app/emulation/emulator_instance.cpp` | 132 | `getAchievementManager()->reset()` |
| `src/app/emulation/emulator_instance.cpp` | 169–170 | `getAchievementManager()->loggedIn() && hardcoreModeActive()` — gates rewind |
| `src/app/achievements/gui/AchievementSetItem.hpp` | 2 | `#include "achievement_list_model.hpp"` |
| `src/app/achievements/gui/AchievementSetItem.hpp` | 3 | `#include "achievement_list_sort_filter_model.hpp"` |
| `src/app/achievements/gui/AchievementSetItem.hpp` | 4 | `#include <firelight/achievement_set.hpp>` |
| `src/app/achievements/gui/AchievementSetItem.hpp` | 10 | `class AchievementSetItem` declaration |
| `src/app/achievements/gui/AchievementSetItem.hpp` | 27 | `AchievementSetItem(const AchievementSet&, ...)` |
| `src/app/achievements/gui/AchievementSetItem.hpp` | 44 | `AchievementListSortFilterModel* getAchievements()` |
| `src/app/achievements/gui/AchievementSetItem.hpp` | 62–63 | `m_sortFilterModel`, `m_achievementListModel` members |
| `src/app/achievements/gui/AchievementSetItem.cpp` | 3 | `#include <firelight/achievement_service.hpp>` |
| `src/app/achievements/gui/AchievementSetItem.cpp` | 9 | `AchievementSetItem::AchievementSetItem(const AchievementSet&, ...)` |
| `src/app/achievements/gui/AchievementSetItem.cpp` | 25 | `QVector<AchievementListModel::Item> items` |
| `src/app/achievements/gui/AchievementSetItem.cpp` | 33 | `items.emplace_back(AchievementListModel::Item{...})` |
| `src/app/achievements/gui/AchievementSetItem.cpp` | 45 | `make_unique<AchievementListModel>(...)` |
| `src/app/achievements/gui/AchievementSetItem.cpp` | 48 | `make_unique<AchievementListSortFilterModel>(...)` |
| `src/app/achievements/gui/AchievementSetItem.cpp` | 56–57 | `AchievementListSortFilterModel* AchievementSetItem::getAchievements()` |
| `src/app/achievements/gui/achievement_list_model.hpp` | 3 | `#include <firelight/achievement.hpp>` |
| `src/app/achievements/gui/achievement_list_model.hpp` | 9 | `class AchievementListModel` declaration |
| `src/app/achievements/gui/achievement_list_model.cpp` | 1 | `#include "achievement_list_model.hpp"` |
| `src/app/achievements/gui/achievement_list_model.cpp` | 7 | `AchievementListModel::AchievementListModel(...)` |
| `src/app/achievements/gui/achievement_list_model.cpp` | 13–87 | `roleNames()`, `rowCount()`, `data()`, `size()`, `setHardcore()` |
| `src/app/achievements/gui/achievement_list_sort_filter_model.hpp` | 6 | `class AchievementListSortFilterModel` declaration |
| `src/app/achievements/gui/achievement_list_sort_filter_model.cpp` | 3 | `#include "achievement_list_model.hpp"` |
| `src/app/achievements/gui/achievement_list_sort_filter_model.cpp` | 6–149 | `AchievementListSortFilterModel` methods using `AchievementListModel::Roles` |
| `src/app/achievements/gui/retro_achievements_game_item.hpp` | 2 | `#include "AchievementSetItem.hpp"` |
| `src/app/achievements/gui/retro_achievements_game_item.hpp` | 9 | `class RetroAchievementsGameItem` declaration |
| `src/app/achievements/gui/retro_achievements_game_item.hpp` | 20 | `Q_PROPERTY(...achievementSets...)` |
| `src/app/achievements/gui/retro_achievements_game_item.hpp` | 42 | `QList<AchievementSetItem*> getAchievementSets()` |
| `src/app/achievements/gui/retro_achievements_game_item.cpp` | 3 | `#include <firelight/achievement_service.hpp>` |
| `src/app/achievements/gui/retro_achievements_game_item.cpp` | 9–55 | `RetroAchievementsGameItem` methods |
| `src/app/library/library_scanner2.cpp` | 7 | `#include <rcheevos/rc_hash.h>` |
| `src/app/library/rom_file.cpp` | 8 | `#include <rcheevos/rc_hash.h>` |
| `src/app/library/gui/library_entry_item.hpp` | 18 | `Q_PROPERTY(int achievementSetId ...)` |
| `src/app/library/gui/library_entry_item.hpp` | 38 | `int getAchievementSetId()` |
| `src/app/library/gui/library_entry_item.cpp` | 49 | `LibraryEntryItem::getAchievementSetId()` |
| `src/app/saves/save_manager.cpp` | 300–308 | Write `rcheevos.state` to disk |
| `src/app/saves/save_manager.cpp` | 377–383 | Read `rcheevos.state` from disk |
| `src/app/saves/save_manager.cpp` | 399 | `.retroachievementsState = rcheevosData` |
| `src/app/saves/save_manager.cpp` | 438–440 | Delete `rcheevos.state` file |
| `src/app/db/sqlite_content_database.hpp` | 28 | `getRetroAchievementsIdForGame(int id)` |
| `src/app/db/sqlite_content_database.cpp` | 138 | `SqliteContentDatabase::getRetroAchievementsIdForGame(int id)` |