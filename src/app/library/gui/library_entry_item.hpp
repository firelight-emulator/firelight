#pragma once

#include <QQuickItem>

#include "manager_accessor.hpp"

namespace firelight {
class LibraryEntryItem : public QQuickItem, public ManagerAccessor {
  Q_OBJECT
  Q_PROPERTY(int entryId READ getEntryId WRITE setEntryId NOTIFY entryIdChanged)
  Q_PROPERTY(QString contentHash READ getContentHash NOTIFY contentHashChanged)
  Q_PROPERTY(QString name READ getName NOTIFY nameChanged)
  Q_PROPERTY(QString icon1x1SourceUrl READ getIcon1x1SourceUrl NOTIFY
                 icon1x1SourceUrlChanged)
  Q_PROPERTY(int achievementSetId READ getAchievementSetId NOTIFY
                 achievementSetIdChanged)
  //        Q_PROPERTY(QString abbreviation READ getAbbreviation NOTIFY
  //        abbreviationChanged)

public:
  explicit LibraryEntryItem(QQuickItem *parent = nullptr);

  ~LibraryEntryItem() override = default;

  void setEntryId(int entryId);

  [[nodiscard]] int getEntryId() const;

  QString getContentHash();

  [[nodiscard]] QString getName() const;

  int getAchievementSetId() const;

  [[nodiscard]] QString getIcon1x1SourceUrl() const;

  //        [[nodiscard]] QString getAbbreviation() const;

signals:
  void entryIdChanged();

  void contentHashChanged();

  void nameChanged();

  void achievementSetIdChanged();

  void icon1x1SourceUrlChanged();

  //        void abbreviationChanged();

private:
  int m_entryId = 0;
  QString m_contentHash;
  QString m_name;
  int m_achievementSetId = 0;
  QString m_icon1x1SourceUrl;
  //        QString m_abbreviation;
};
} // namespace firelight
