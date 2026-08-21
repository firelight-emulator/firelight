#include "netplay_chat_model.hpp"

namespace firelight::gui {

NetplayChatModel::NetplayChatModel(netplay::NetplayService &service, QObject *parent)
    : QAbstractListModel(parent), m_service(service) {}

int NetplayChatModel::rowCount(const QModelIndex &parent) const {
  return parent.isValid() ? 0 : static_cast<int>(m_messages.size());
}

QVariant NetplayChatModel::data(const QModelIndex &index, const int role) const {
  if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(m_messages.size())) {
    return {};
  }
  const auto &message = m_messages[index.row()];

  switch (role) {
  case SenderName:
    return QString::fromStdString(message.senderName);
  case Text:
    return QString::fromStdString(message.text);
  case IsSelf:
    return message.senderId == m_selfId;
  case TimestampMs:
    return QVariant::fromValue(message.timestampMs);
  default:
    return {};
  }
}

QHash<int, QByteArray> NetplayChatModel::roleNames() const {
  return {{SenderName, "senderName"}, {Text, "text"}, {IsSelf, "isSelf"}, {TimestampMs, "timestampMs"}};
}

void NetplayChatModel::refresh() {
  beginResetModel();
  m_messages = m_service.session().chatMessages();
  m_selfId = m_service.session().lobby().self.id;
  endResetModel();
}

} // namespace firelight::gui
