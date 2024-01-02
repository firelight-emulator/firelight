//
// Created by alexs on 12/22/2023.
//

#ifndef FIRELIGHT_QLIBENTRYMODELSHORT_HPP
#define FIRELIGHT_QLIBENTRYMODELSHORT_HPP

#include <qabstractitemmodel.h>

class QLibraryViewModel final : public QAbstractListModel {
  Q_OBJECT

public:
  struct Item {
    int id;
    std::string display_name;
  };

  QLibraryViewModel() = default;
  [[nodiscard]] QVariant data(const QModelIndex &index,
                              int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  bool insertRows(int row, int count, const QModelIndex &parent) override;
  bool removeRows(int row, int count, const QModelIndex &parent) override;

  void set_items(const std::vector<Item> &items);

private:
  enum Roles { DisplayName = Qt::UserRole + 1, EntryId };

  std::vector<Item> items_;
  QHash<int, QByteArray> roles_;
};

#endif // FIRELIGHT_QLIBENTRYMODELSHORT_HPP
