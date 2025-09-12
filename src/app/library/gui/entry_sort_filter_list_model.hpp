#pragma once
#include "../entry.hpp"
#include "../user_library.hpp"
#include <QAbstractListModel>
#include <QSortFilterProxyModel>

namespace firelight::library {
class EntrySortFilterListModel : public QSortFilterProxyModel {
  Q_OBJECT
  Q_PROPERTY(
      int folderId READ getFolderId WRITE setFolderId NOTIFY folderIdChanged)
  Q_PROPERTY(QString sortMethod READ getSortMethod WRITE setSortMethod NOTIFY
                 sortMethodChanged)

public:
  explicit EntrySortFilterListModel(QObject *parent = nullptr);

  int getFolderId() const;
  void setFolderId(int folderId);

  QString getSortMethod() const;
  void setSortMethod(const QString &method);

signals:
  void folderIdChanged();
  void sortMethodChanged();

protected:
  bool filterAcceptsRow(int source_row,
                        const QModelIndex &source_parent) const override;

  bool lessThan(const QModelIndex &source_left,
                const QModelIndex &source_right) const override;

private:
  int m_folderId = -1;
  QString m_sortMethod;
};
} // namespace firelight::library
