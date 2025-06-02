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

public:
  explicit EntrySortFilterListModel(QObject *parent = nullptr);

  int getFolderId() const;
  void setFolderId(int folderId);

signals:
  void folderIdChanged();

protected:
  bool filterAcceptsRow(int source_row,
                        const QModelIndex &source_parent) const override;

private:
  int m_folderId = -1;
};
} // namespace firelight::library
