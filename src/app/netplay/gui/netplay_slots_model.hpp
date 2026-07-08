#pragma once

#include "../netplay_service.hpp"

#include <QAbstractListModel>

namespace firelight::gui {

// Always MAX_SLOTS rows — empty slots are rows with occupied=false, so the
// lobby's slot grid is a stable 8-cell layout.
class NetplaySlotsModel final : public QAbstractListModel {
  Q_OBJECT

public:
  enum Roles {
    SlotNumber = Qt::UserRole + 1,
    Occupied,
    MemberId,
    DisplayName,
    IsSelf,
    IsHostMember,
    Ready,
    LocalPadIndex
  };

  explicit NetplaySlotsModel(netplay::NetplayService &service,
                             QObject *parent = nullptr);

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index,
                              int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

public slots:
  void refresh();

private:
  netplay::NetplayService &m_service;
  netplay::SlotTable m_slots;
  netplay::PlayerId m_selfId = 0;
  netplay::PlayerId m_hostId = 0;
};

} // namespace firelight::gui
