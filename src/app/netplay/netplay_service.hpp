#pragma once

#include "guest_stream_receiver.hpp"
#include "host_stream_sender.hpp"
#include "netplay_retropad_provider.hpp"

#include <firelight/event_dispatcher.hpp>
#include <firelight/library/user_library_service.hpp>
#include <firelight/netplay/session.hpp>

#include <functional>
#include <string>

namespace firelight::achievements {
class RAClient;
}

namespace firelight::netplay {

// App-side orchestrator for online play: owns the NetplaySession over the
// injected lobby backend + transport, resolves library entries into session
// game descriptors, auto-seats members into player slots, and (in later
// phases) runs the in-game stream pipelines
class NetplayService {
public:
  NetplayService(ILobbyBackend &lobbyBackend, IPeerTransport &transport, library::UserLibraryService &library,
                 std::string appVersion, achievements::RAClient *achievementClient = nullptr,
                 libretro::IRetropadProvider *localPads = nullptr,
                 std::function<std::shared_ptr<IAudioOutput>()> guestAudioFactory = {});

  // Consumer-facing session events; forwarded after this service applies its
  // own policies (slot auto-assignment). Set before hosting/joining
  void setEvents(SessionEvents events);

  void signIn(std::function<void(bool ok)> done);
  [[nodiscard]] SignInState signInState() const;
  [[nodiscard]] PlayerIdentity localIdentity() const;
  [[nodiscard]] std::string providerName() const;
  void setPlayerName(const std::string &name);

  void hostLobby(ILobbyBackend::ResultCallback done);
  void joinLobby(const std::string &joinCode, ILobbyBackend::ResultCallback done);
  void leaveLobby();

  void assignSlot(int slot, PlayerId memberId, int localPadIndex);
  void clearSlot(int slot);
  void setReady(bool ready);
  void selectGameByEntryId(int entryId);
  void sendChat(const std::string &text);

  // Ready check: host announces the selected game (phase Starting); everyone
  // sees the check and readies up. confirmLaunch moves to InGame (the host
  // actually launches); endGame returns the lobby to Idle
  bool startGame();
  void confirmLaunch();
  void endGame(const std::string &reason = "finished");

  // Library entry behind the host's selected game (-1 when unknown)
  [[nodiscard]] int selectedEntryId() const;

  [[nodiscard]] NetplaySession &session() { return m_session; }

  // Permanently installable frame/audio sink; streams only while a game is
  // live (see EmulationContext.netplayStreamSink)
  [[nodiscard]] HostStreamSender &streamSender() { return m_sender; }

  // Permanently installable port mapper (EmulationContext.retropadProvider);
  // pass-through until a hosted online game is live
  [[nodiscard]] SlotMappedRetropadProvider &retropadProvider() { return m_retropadProvider; }

  [[nodiscard]] GuestStreamReceiver &streamReceiver() { return m_receiver; }

private:
  // Host policy: members get the first free slot when they join, so a lobby
  // works without ever opening the lobby page
  void autoAssignMembers();

  void restoreHardcoreIfSuppressed();

  void handlePhase(GamePhase phase);

  ILobbyBackend &m_lobbyBackend;
  library::UserLibraryService &m_library;
  achievements::RAClient *m_achievementClient = nullptr;
  NetplaySession m_session;
  HostStreamSender m_sender{m_session};
  SlotMappedRetropadProvider m_retropadProvider;
  GuestStreamReceiver m_receiver;
  SessionEvents m_forward;
  int m_selectedEntryId = -1;
  // Remote inputs are indistinguishable from injected ones, so hardcore is
  // suppressed for the host's game and restored afterward
  bool m_hardcoreSuppressed = false;
  // The lobby's game ends when the host's emulator stops, UI or no UI
  ScopedConnection m_emulationStoppedConnection;
};

} // namespace firelight::netplay
