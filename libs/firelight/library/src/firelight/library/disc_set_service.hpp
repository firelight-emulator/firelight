#pragma once

#include <firelight/event_dispatcher.hpp>
#include <firelight/library/disc_set.hpp>
#include <firelight/library/disc_set_playlist.hpp>
#include <firelight/library/user_library_repository.hpp>

#include <atomic>
#include <optional>
#include <string>
#include <vector>

namespace firelight::library {

/**
 * Folds the discs of one game into a single library entry.
 *
 * A disc is a content file, not an entry, so a three-disc game is one row. Discs arrive as
 * separate entries because each has its own content hash; this finds them and absorbs the
 * extras into the one holding the lowest disc, which is the only one left able to launch.
 *
 * Runs off EntryUpdatedEvent rather than at ingest, because the title a set is matched on is
 * written by metadata population after the entry already exists
 */
class DiscSetService {
public:
  DiscSetService(IUserLibraryRepository &library, std::string appDataDirectory);

  ~DiscSetService() = default;

  /**
   * Where playlists are written from now on. Existing ones are left where they are, since
   * moving a file somebody's other frontend is pointed at is not ours to do
   */
  void setPlaylistLocation(PlaylistLocation location);

  /**
   * Finds the discs of the same game as this entry and folds them together.
   *
   * @return True when anything moved
   */
  bool autoGroupDiscs(int entryId);

  /**
   * Takes a disc out of its set and gives it its own entry back. Records that a person decided
   * it, so automatic grouping will not undo them
   */
  bool detachDisc(int contentFileId);

  /**
   * Hands an entry back to automatic grouping, undoing an earlier detach
   */
  bool clearUserChoice(int entryId);

  /**
   * Rewrites the playlist of any set whose one has gone or fallen out of date.
   *
   * A set launches through a file we generate, so a game whose playlist is deleted, moved or
   * left behind by a games folder that moved would otherwise disappear from the library while
   * every one of its discs is still sitting there
   */
  void reconcilePlaylists();

private:
  /**
   * Whether these discs and the ones already in their sets number off without repeating.
   *
   * A repeat means more than one release is in play, and which disc pairs with which is a
   * guess. Guessing wrong deletes somebody's entry, so nothing happens instead
   */
  bool areDiscNumbersUnambiguous(const std::vector<Entry> &members);

  /** The set these discs belong in, creating one when none of them is in a set yet */
  std::optional<int> resolveSet(const std::vector<Entry> &members, const Entry &survivor);

  /**
   * Folds one entry into another: its discs join the set, their ways in are removed, and the
   * entry itself goes. Announces the move so saves and playtime can follow
   */
  bool absorb(const Entry &absorbed, const Entry &survivor, int setId);

  /** Gives a disc a way in of its own, which is what puts it back in the library as an entry */
  bool restoreOwnEntry(const ContentFile &file, const std::string &title, bool isUserChoice);

  /** Removes a set once it is down to one disc, since one disc is not a set */
  bool dissolveIfUndersized(int setId);

  /**
   * Writes the set's playlist and makes it the way in, so the core is handed one path for the
   * whole game and writes one memory card.
   *
   * A playlist somebody wrote themselves is adopted rather than overwritten, and a folder that
   * cannot be written to is left alone
   */
  bool writePlaylist(int setId, const Entry &survivor);

  /** Takes a retired set's playlist out of the library, and off disk when we wrote it */
  void retirePlaylist(const DiscSet &set);

  /** The entry a set launches through */
  std::optional<Entry> survivorOf(int setId);

  IUserLibraryRepository &m_library;

  std::string m_appDataDirectory;

  std::atomic<PlaylistLocation> m_playlistLocation{PlaylistLocation::BesideDiscs};

  // Set while a write of this service's own is in flight, so the event that write publishes
  // does not start another round
  std::atomic_bool m_applying{false};

  ScopedConnection m_entryUpdatedConnection;

  ScopedConnection m_runConfigurationDeletedConnection;
};

} // namespace firelight::library
