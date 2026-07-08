#pragma once

#include "../app/netplay/netplay_service.hpp"

#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariantList>

namespace firelight::gui {

// QML bridge for online play. Provider-agnostic on purpose: the UI renders
// "Sign in with <providerName>" from data and never names Discord. Session
// events can arrive on network threads; everything is marshaled to the GUI
// thread before signals fire.
class QtNetworkServiceProxy final : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString signInState READ signInState NOTIFY signInStateChanged)
  Q_PROPERTY(QString providerName READ providerName CONSTANT)
  Q_PROPERTY(QString playerName READ playerName NOTIFY playerNameChanged)
  Q_PROPERTY(bool inLobby READ inLobby NOTIFY lobbyStateChanged)
  Q_PROPERTY(bool isHost READ isHost NOTIFY lobbyStateChanged)
  Q_PROPERTY(int memberCount READ memberCount NOTIFY lobbyStateChanged)
  Q_PROPERTY(QString joinCode READ joinCode NOTIFY lobbyStateChanged)
  Q_PROPERTY(QString sessionPhase READ sessionPhase NOTIFY phaseChanged)
  Q_PROPERTY(QString gameName READ gameName NOTIFY gameChanged)
  Q_PROPERTY(QString gameArtUrl READ gameArtUrl NOTIFY gameChanged)
  Q_PROPERTY(int gamePlatformId READ gamePlatformId NOTIFY gameChanged)
  Q_PROPERTY(bool hasGame READ hasGame NOTIFY gameChanged)
  Q_PROPERTY(int readySlotCount READ readySlotCount NOTIFY slotsChanged)
  Q_PROPERTY(int occupiedSlotCount READ occupiedSlotCount NOTIFY slotsChanged)
  Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

public:
  explicit QtNetworkServiceProxy(netplay::NetplayService &service,
                                 QObject *parent = nullptr);

  [[nodiscard]] QString signInState() const;
  [[nodiscard]] QString providerName() const;
  [[nodiscard]] QString playerName() const;
  [[nodiscard]] bool inLobby() const;
  [[nodiscard]] bool isHost() const;
  [[nodiscard]] QString joinCode() const;
  [[nodiscard]] QString sessionPhase() const;
  [[nodiscard]] QString gameName() const;
  [[nodiscard]] QString gameArtUrl() const;
  [[nodiscard]] int gamePlatformId() const;
  [[nodiscard]] bool hasGame() const;
  [[nodiscard]] int memberCount() const;
  [[nodiscard]] int readySlotCount() const;
  [[nodiscard]] int occupiedSlotCount() const;
  [[nodiscard]] bool busy() const { return m_busy; }

  Q_INVOKABLE void setPlayerName(const QString &name);
  Q_INVOKABLE void signIn();
  Q_INVOKABLE void hostLobby();
  Q_INVOKABLE void joinLobby(const QString &joinCode);
  Q_INVOKABLE void leaveLobby();
  Q_INVOKABLE void assignSlot(int slot, const QString &memberId,
                              int localPadIndex);
  Q_INVOKABLE void clearSlot(int slot);
  Q_INVOKABLE void setReady(bool ready);
  Q_INVOKABLE void selectGame(int entryId);
  Q_INVOKABLE void sendChat(const QString &text);
  // Ready check: startSession announces the game (host only), confirmLaunch
  // marks the game live once the host actually launches, endSession returns
  // the lobby to idle.
  Q_INVOKABLE void startSession();
  Q_INVOKABLE void confirmLaunch();
  Q_INVOKABLE void endSession();
  Q_INVOKABLE [[nodiscard]] int selectedGameEntryId() const;
  Q_INVOKABLE void copyJoinCode();
  // The stream view registers itself to receive decoded frames. Detach only
  // releases if `item` is still the attached view — a page being torn down
  // must not detach its replacement.
  Q_INVOKABLE void attachStreamItem(QObject *item);
  Q_INVOKABLE void detachStreamItem(QObject *item);
  // Lobby members for the host's slot-assignment picker:
  // [{memberId, displayName}].
  Q_INVOKABLE [[nodiscard]] QVariantList lobbyMembers() const;

signals:
  void signInStateChanged();
  void playerNameChanged();
  void lobbyStateChanged();
  void slotsChanged();
  void phaseChanged();
  void gameChanged();
  void chatChanged();
  void busyChanged();
  void errorOccurred(const QString &message);

private:
  void setBusy(bool busy);
  // Queues a signal emission onto the GUI thread.
  template <typename Fn> void onGuiThread(Fn &&fn) {
    QMetaObject::invokeMethod(this, std::forward<Fn>(fn),
                              Qt::QueuedConnection);
  }

  netplay::NetplayService &m_service;
  bool m_busy = false;
  QPointer<QObject> m_attachedStreamItem;
};

} // namespace firelight::gui
