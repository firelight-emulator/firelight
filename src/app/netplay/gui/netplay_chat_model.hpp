#pragma once

#include "../netplay_service.hpp"

#include <QAbstractListModel>

namespace firelight::gui {

class NetplayChatModel final : public QAbstractListModel {
  Q_OBJECT

public:
  enum Roles {
    SenderName = Qt::UserRole + 1,
    Text,
    IsSelf,
    TimestampMs
  };

  explicit NetplayChatModel(netplay::NetplayService &service,
                            QObject *parent = nullptr);

  [[nodiscard]] int rowCount(const QModelIndex &parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex &index,
                              int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

public slots:
  void refresh();

private:
  netplay::NetplayService &m_service;
  std::deque<netplay::ChatMessage> m_messages;
  netplay::PlayerId m_selfId = 0;
};

} // namespace firelight::gui
