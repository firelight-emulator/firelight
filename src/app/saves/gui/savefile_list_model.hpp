#pragma once

#include "../savefile_info.hpp"
#include <QAbstractListModel>

namespace firelight::saves {

class SavefileListModel : public QAbstractListModel {
  Q_OBJECT
public:
  enum Roles {
    HasData = Qt::UserRole + 1,
    FilePath,
    ContentHash,
    SlotNumber,
    SavefileMd5,
    Name,
    Description,
    LastModified
  };

  SavefileListModel();
  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index,
                              int role) const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  QHash<int, QByteArray> roleNames() const override;

  void reset(std::vector<SavefileInfo> items);

private:
  std::vector<SavefileInfo> m_items;
};

} // namespace firelight::saves
