#pragma once

#include <QObject>
#include <QVariantList>
#include <QVariantMap>

namespace firelight::library {
class VariantGroupService;
}

namespace firelight::gui {

/**
 * QML bridge for grouping entries that are the same game.
 *
 * Every method here changes group membership or the choice of which entry stands for a group;
 * reading a group's contents is the library model's job
 */
class QtVariantGroupProxy final : public QObject {
  Q_OBJECT

public:
  explicit QtVariantGroupProxy(library::VariantGroupService &service, QObject *parent = nullptr);

  /**
   * @param entryIds At least two entries, all on the same platform
   * @return The new group's id, or -1 when they cannot form one
   */
  Q_INVOKABLE int createGroup(const QVariantList &entryIds);

  Q_INVOKABLE bool addToGroup(int entryId, int groupId);

  /**
   * Takes an entry out of its group for good: automatic grouping will not put it back
   */
  Q_INVOKABLE bool removeFromGroup(int entryId);

  /**
   * Hands an entry back to automatic grouping, undoing an earlier add or remove
   */
  Q_INVOKABLE bool clearUserChoice(int entryId);

  /**
   * Stops the ordering choosing which entry stands for the group
   */
  Q_INVOKABLE bool pinPrimary(int groupId, int entryId);

  Q_INVOKABLE bool unpinPrimary(int groupId);

  Q_INVOKABLE bool setTitle(int groupId, const QString &title);

  /**
   * Whether launching the group goes straight to its primary rather than asking which variant
   */
  Q_INVOKABLE bool setAutoLaunchPrimary(int groupId, bool autoLaunch);

  /**
   * The entry to launch for a group, which is the pinned one unless it cannot be launched
   *
   * @return The entry id, or -1 when the group has nothing launchable
   */
  Q_INVOKABLE int resolvePrimary(int groupId);

private:
  library::VariantGroupService &m_service;
};

} // namespace firelight::gui
