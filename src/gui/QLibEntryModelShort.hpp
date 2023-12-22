//
// Created by alexs on 12/22/2023.
//

#ifndef FIRELIGHT_QLIBENTRYMODELSHORT_HPP
#define FIRELIGHT_QLIBENTRYMODELSHORT_HPP

#include "src/app/library/entry.hpp"

#include <QAbstractListModel>
class QLibEntryModelShort : public QAbstractListModel {
  Q_OBJECT

  enum Roles { DisplayName = Qt::UserRole + 1, EntryId };

public:
  int rowCount(const QModelIndex &parent) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  void setEntries(const std::vector<FL::Library::Entry> &entries);

private:
  std::vector<FL::Library::Entry> m_entries;
};

#endif // FIRELIGHT_QLIBENTRYMODELSHORT_HPP
