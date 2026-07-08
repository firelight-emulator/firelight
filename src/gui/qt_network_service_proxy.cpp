#include "qt_network_service_proxy.hpp"

#include "../app/netplay/gui/netplay_stream_item.hpp"

#include <QClipboard>
#include <QGuiApplication>
#include <QPointer>

namespace firelight::gui {

namespace {
QString signInStateName(const netplay::SignInState state) {
  switch (state) {
  case netplay::SignInState::SigningIn:
    return "signing-in";
  case netplay::SignInState::Ready:
    return "ready";
  case netplay::SignInState::Failed:
    return "failed";
  case netplay::SignInState::SignedOut:
  default:
    return "signed-out";
  }
}

QString phaseName(const netplay::GamePhase phase) {
  switch (phase) {
  case netplay::GamePhase::Starting:
    return "starting";
  case netplay::GamePhase::InGame:
    return "in-game";
  case netplay::GamePhase::Idle:
  default:
    return "idle";
  }
}
} // namespace

QtNetworkServiceProxy::QtNetworkServiceProxy(netplay::NetplayService &service,
                                             QObject *parent)
    : QObject(parent), m_service(service) {
  netplay::SessionEvents events;
  events.lobbyChanged = [this] {
    onGuiThread([this] { emit lobbyStateChanged(); });
  };
  events.slotsChanged = [this] {
    onGuiThread([this] { emit slotsChanged(); });
  };
  events.gameChanged = [this] {
    onGuiThread([this] { emit gameChanged(); });
  };
  events.phaseChanged = [this](netplay::GamePhase) {
    onGuiThread([this] { emit phaseChanged(); });
  };
  events.chatMessageAdded = [this](const netplay::ChatMessage &) {
    onGuiThread([this] { emit chatChanged(); });
  };
  events.peerReady = [this](netplay::PlayerId, netplay::IPeerLink &) {
    onGuiThread([this] {
      emit lobbyStateChanged();
      emit slotsChanged();
    });
  };
  events.peerLost = [this](netplay::PlayerId) {
    onGuiThread([this] { emit lobbyStateChanged(); });
  };
  events.lobbyEnded = [this](const std::string &reason) {
    onGuiThread([this, reason] {
      emit lobbyStateChanged();
      emit slotsChanged();
      emit phaseChanged();
      emit gameChanged();
      emit chatChanged();
      if (reason == "lobby-closed" || reason == "host-left") {
        emit errorOccurred("The host closed the lobby");
      }
    });
  };
  m_service.setEvents(std::move(events));
}

QString QtNetworkServiceProxy::signInState() const {
  return signInStateName(m_service.signInState());
}

QString QtNetworkServiceProxy::providerName() const {
  return QString::fromStdString(m_service.providerName());
}

QString QtNetworkServiceProxy::playerName() const {
  return QString::fromStdString(m_service.localIdentity().displayName);
}

bool QtNetworkServiceProxy::inLobby() const {
  return m_service.session().inLobby();
}

bool QtNetworkServiceProxy::isHost() const {
  return m_service.session().isHost();
}

QString QtNetworkServiceProxy::joinCode() const {
  return QString::fromStdString(m_service.session().joinCode());
}

QString QtNetworkServiceProxy::sessionPhase() const {
  return phaseName(m_service.session().phase());
}

QString QtNetworkServiceProxy::gameName() const {
  const auto game = m_service.session().game();
  return game ? QString::fromStdString(game->gameName) : QString();
}

QString QtNetworkServiceProxy::gameArtUrl() const {
  const auto game = m_service.session().game();
  return game ? QString::fromStdString(game->artUrl) : QString();
}

int QtNetworkServiceProxy::gamePlatformId() const {
  const auto game = m_service.session().game();
  return game ? game->platformId : 0;
}

bool QtNetworkServiceProxy::hasGame() const {
  return m_service.session().game().has_value();
}

int QtNetworkServiceProxy::memberCount() const {
  return static_cast<int>(m_service.session().lobby().members.size());
}

int QtNetworkServiceProxy::readySlotCount() const {
  int count = 0;
  for (const auto &slot : m_service.session().slotTable().all()) {
    if (slot && slot->ready) {
      count++;
    }
  }
  return count;
}

int QtNetworkServiceProxy::occupiedSlotCount() const {
  return m_service.session().slotTable().occupiedCount();
}

void QtNetworkServiceProxy::setPlayerName(const QString &name) {
  m_service.setPlayerName(name.trimmed().toStdString());
  emit playerNameChanged();
}

void QtNetworkServiceProxy::signIn() {
  setBusy(true);
  emit signInStateChanged();
  m_service.signIn([this](bool) {
    onGuiThread([this] {
      setBusy(false);
      emit signInStateChanged();
      emit playerNameChanged();
    });
  });
}

void QtNetworkServiceProxy::hostLobby() {
  setBusy(true);
  m_service.hostLobby([this](const bool ok, const std::string &error) {
    onGuiThread([this, ok, error] {
      setBusy(false);
      emit lobbyStateChanged();
      if (!ok) {
        emit errorOccurred(QString::fromStdString(error));
      }
    });
  });
}

void QtNetworkServiceProxy::joinLobby(const QString &joinCode) {
  setBusy(true);
  m_service.joinLobby(
      joinCode.trimmed().toStdString(),
      [this](const bool ok, const std::string &error) {
        onGuiThread([this, ok, error] {
          setBusy(false);
          emit lobbyStateChanged();
          emit slotsChanged();
          emit gameChanged();
          emit phaseChanged();
          if (!ok) {
            emit errorOccurred(QString::fromStdString(error));
          }
        });
      });
}

void QtNetworkServiceProxy::leaveLobby() {
  m_service.leaveLobby();
  emit lobbyStateChanged();
  emit slotsChanged();
  emit phaseChanged();
  emit gameChanged();
  emit chatChanged();
}

void QtNetworkServiceProxy::assignSlot(const int slot, const QString &memberId,
                                       const int localPadIndex) {
  bool ok = false;
  const auto id = memberId.toULongLong(&ok);
  if (ok) {
    m_service.assignSlot(slot, id, localPadIndex);
  }
}

void QtNetworkServiceProxy::clearSlot(const int slot) {
  m_service.clearSlot(slot);
}

void QtNetworkServiceProxy::setReady(const bool ready) {
  m_service.setReady(ready);
}

void QtNetworkServiceProxy::selectGame(const int entryId) {
  m_service.selectGameByEntryId(entryId);
}

void QtNetworkServiceProxy::sendChat(const QString &text) {
  const auto trimmed = text.trimmed();
  if (!trimmed.isEmpty()) {
    m_service.sendChat(trimmed.toStdString());
  }
}

void QtNetworkServiceProxy::startSession() {
  if (!m_service.startGame()) {
    emit errorOccurred("Pick a game first");
  }
}

void QtNetworkServiceProxy::confirmLaunch() { m_service.confirmLaunch(); }

void QtNetworkServiceProxy::endSession() { m_service.endGame(); }

int QtNetworkServiceProxy::selectedGameEntryId() const {
  return m_service.selectedEntryId();
}

void QtNetworkServiceProxy::attachStreamItem(QObject *item) {
  auto *streamItem = qobject_cast<NetplayStreamItem *>(item);
  if (!streamItem) {
    return;
  }
  m_attachedStreamItem = streamItem;
  // QPointer: the item is QML-owned and may be destroyed while frames are in
  // flight; presentFrame marshals internally, so a stale call is a no-op.
  QPointer<NetplayStreamItem> guard(streamItem);
  m_service.streamReceiver().setFrameSink(
      [guard](firelight::media::StreamVideoFrame frame) {
        if (auto *target = guard.data()) {
          target->presentFrame(std::move(frame));
        }
      });
}

void QtNetworkServiceProxy::detachStreamItem(QObject *item) {
  if (item != m_attachedStreamItem) {
    return; // a torn-down page must not detach its replacement
  }
  m_attachedStreamItem = nullptr;
  m_service.streamReceiver().setFrameSink(nullptr);
}

void QtNetworkServiceProxy::copyJoinCode() {
  if (auto *clipboard = QGuiApplication::clipboard()) {
    clipboard->setText(joinCode());
  }
}

QVariantList QtNetworkServiceProxy::lobbyMembers() const {
  QVariantList members;
  for (const auto &member : m_service.session().lobby().members) {
    members.append(QVariantMap{
        {"memberId", QString::number(member.id)},
        {"displayName", QString::fromStdString(member.displayName)},
    });
  }
  return members;
}

void QtNetworkServiceProxy::setBusy(const bool busy) {
  if (m_busy != busy) {
    m_busy = busy;
    emit busyChanged();
  }
}

} // namespace firelight::gui
