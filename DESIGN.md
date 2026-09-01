<!-- TODO: NEEDS REVIEW -->
# Design decisions

Load-bearing facts about how Firelight is put together, and the reasoning that fixes them in place.
Things here are not preferences — each one has something downstream that breaks if it changes.

A fact belongs here when it took real work to establish, when the code alone does not make it
obvious, or when someone reading the code would reasonably assume the opposite. Where a decision is
still open, it says so.

---

## Identity

### A content hash is the key, and it must stay derivable from the file alone

The governing requirement: *someone should be able to reinstall Firelight, not have the content
database for some reason, and have their saves and everything just work.*

`rc_hash` computes a content hash from the file's bytes — no database, no network. That is the only
identifier with this property, which is why it is the key.

User data is hash-keyed across **five separate databases**: `settings.db/game_settings`,
`cheats.db/cheats`, `activity.db/play_sessions`, `userdata.db/savefile_metadata`, and
`rcheevos3.db/game_hashes`. They reattach after a reinstall because the hash is recomputed from the
files. None of them could follow a database id, and none should — a US and a JP dump cannot share a
`.srm` even when they are the same game.

**Consequence:** entries are not keyed on a game id, and rekeying them on one has been evaluated and
rejected. The id is obtained *by hashing the file and looking the hash up*, so it is the output, not
the input; content the database does not recognise would have no key at all.

### Many files, one dump, one entry

```
content_files       physical files. MANY share one content_hash —
                    .z64 / .v64 / .n64, headered or not, zipped or not, ten copies in ten folders
content_hash        a DUMP: the logical content, format-independent
entries             one per dump. a game
run_configurations  a way to launch. an entry has one or more
```

`rc_hash` normalises format and container away, so a `.cue` and a `.chd` of one disc hash
identically, as do `.z64`/`.v64`/`.n64` of one cartridge.

**A unique content hash means a unique game.** Two dumps that hash differently are different games
and are never merged in data. Two copies of Super Mario World that hash differently are two entries.

### There is no game id, and there will not be one

A RetroAchievements id spans regions *and* discs by construction — FF7 USA and FF7 JP are one RA
game — so it can never decide whether a US disc 1 and a JP disc 2 belong together.

The metadata database answers **per dump, keyed on content hash**: disc number, disc count, title,
region. That keeps everything derivable from the file with no id space to keep stable.

All of it is gone: `Entry::gameId`, `ContentFile::m_gameId`, the `content_files.game_id` column,
`GameIdentity::gameId`, `areSameGame`'s id branch and `getCandidateEntryIds`' `game_id` join.

There is a consequence worth stating plainly, because it looks like a regression and is not one:
two releases of one game that are *named differently* — Biohazard 2 and Resident Evil 2 — no longer
group as variants on their own. An id was the only thing that could have joined them, and it could
never be trusted to, because the same id also spans their discs. What joins them instead is a common
title, which the metadata database supplies per dump once it has run.

---

## Entries and ways in

### An entry is a facade for one or more run configurations

The `RunConfiguration` is the real launchable unit. An entry is the user-facing presentation of the
ways to launch one game.

This is what an entry *is*, not a rule about visibility. **Visibility is a separate concern:** an
entry whose files are all gone stays visible as a badged, unplayable tombstone rather than vanishing.

### A disc set is a way to launch

So it lives at the run-configuration layer rather than floating beside it. Every disc owns a set; a
lone disc is a **set of one**, so there is no special case between one disc and several. A cartridge
has no set at all — it launches through its own file, as the next section explains. A disc inside a
set has no way in of its own; you launch the entry and choose a disc.

### Cartridges get one configuration per file, discs get one per set

This split is forced by the launch artifact, not chosen:

- **Disc content** launches through `<appdata>/playlists/<hash>.m3u`, a pure function of the hash.
  Two configurations sharing a hash would render to the *same* file and fight over it. So disc
  content gets one configuration per **set**.
- **Cartridge content** launches through its own file with no artifact, so two ways in sharing a hash
  never collide. One configuration per **file** — `Mario.z64` and `Mario.v64` are two ways in.

The playlist path must be a pure function of the hash because **cores with their own memory-card
handling name the card after the path they are handed** (`game_loader.cpp` hands them one shared
directory). A launch path that changes shape between runs silently orphans a memory card.

### A patched result is its own entry

A patch produces content with its own hash — `patch_files` already stores `patched_content_hash` —
and a different hash is a different game. Since the patched bytes never exist on disk, its way in
names the *original* file plus the patch. Saves and achievements key on the patched hash, which is
right: a randomiser is not the same game as what it was built from.

---

## Discs

### An `.m3u` is a redirect, not content

`rc_hash_generate_from_playlist` (`libs/rcheevos/src/rhash/hash.c:765`) reads the playlist's first
line that is neither blank nor a `#` comment, resolves it against the playlist's own directory, and
hashes **that file**. The playlist's own bytes never enter the hash.

That comment skipping (`hash.c:716`) is what lets a generated playlist open with a marker line
without moving the set's identity.

Three consequences:

1. A playlist hashes identically to its first listed disc, and reordering it changes its hash.
2. **A playlist whose first disc is absent produces no hash at all** — `rc_hash` fails outright. It
   does not fall back to the second line.
3. Only the first 1023 bytes are read, so a playlist front-loaded with comments past 1KB fails.

### A set is identified by its lowest-numbered member that has a hash

Not simply "lowest-numbered member". A member that exists only as a line in someone's playlist has no
hash and can never anchor a set — see above. So a set is identified by the lowest disc actually on
disk, and binding a lower one later moves the identity.

A member whose file has gone *missing* keeps its hash and keeps anchoring, which is what stops an
unplugged drive re-keying anyone's saves.

### Every recognised file gets a row, and its role says what it is

`content_files` records everything the walk recognises — dumps, the raw tracks a sheet speaks for,
and playlists — with `role` saying which. Before this, a track or a playlist was simply not handed
back, so what the walk decided about a file could only be inferred from its absence.

Only a `Dump` stands for a game. Only a dump is hashed, announces itself, and can become an entry;
the listings that mean "what can be launched" filter on the role, and the ones that mean "what is on
disk" do not.

The playlist case is the reason this matters rather than being bookkeeping: a playlist hashes
identically to its first listed disc, so a row that does not say what it is hands that disc's entry a
second way in and its own path as somewhere the game's content lives.

### Membership is not presence

A disc whose file goes missing stays a member. The member list and the present-file list are two
different questions and every consumer needs both.

### An entry carries no disc number, and no set id

Which disc a dump is belongs to the file. An entry stands for a game rather than for one of its
discs, so it has no disc number of its own; `identityOf(const ContentFile &)` is where the disc axis
is read, and placement is its only caller.

Which set an entry is in is derived rather than stored: an entry belongs to the set its way in names
(`run_configurations.disc_set_id`), and additionally to any set holding a dump of its hash — the
second is how a disc rejoining a set is found before the set has taken it over. Nothing writes an
entry-to-set column, because two copies of one fact drift.

### One entry per content hash, enforced in code

`entries.content_hash` carries a plain index rather than a unique one, so the schema alone does not
say this. `createEntry` does: it refuses a hash an entry already holds. That refusal is what makes an
entry a facade *over* its ways in rather than one of them.

### A set's membership lives in `disc_set_discs` and nowhere else

`content_files` used to mirror it in a `disc_set_id` column, kept in step by hand. Two copies of one
fact drift, so the column is gone and the membership row is the only answer to which set a disc is
in — including for the events announcing a file going missing or coming back.

### A playlist of ours says so on its first line

Generated playlists sit in `<appdata>/playlists`, named for the set's content hash, and open with
`# Generated by Firelight`. That line is what makes overwriting or deleting one ours to do:
`retirePlaylist` and `materializePlaylist` both read the file first and leave alone anything that
does not carry it. The path alone is not enough, because it is a path anybody could write to.

### Membership names a file, not a dump

A `.cue` and a `.chd` of one disc share a hash, and the renderer needs to see **both** to pick the
openable one — it sorts by `(disc_number, in_archive, file_path)` and de-dups by hash, keeping the
first. So disc numbers are not unique within a set.

### Nothing automatic ever evicts a disc

Placement walks a ladder once and records which rung decided: the user's own choice → a user-supplied
`.m3u` → the metadata database → filename heuristics → its own set of one. A claim speaks only about
the discs it names. Only a person removes a disc from a set.

Where more than one candidate set matches, placement picks deterministically and marks the row
uncertain, so the guess is visible rather than silent.

### Scan order must never change any outcome

Identity is a pure function of membership, so the order files are discovered in cannot reach it.

---

## Open questions

### Does a user renumbering a disc move the entry's identity?

If a renumber makes a different disc the lowest, the identity would follow — taking saves, cheats,
settings, playtime and achievements across five databases with it. A number spinner causing silent
data movement is the sharp edge.

**The content hash must stay invisible to users**, which rules out the obvious mitigation: a dialog
explaining that saves live under a hash is exactly the leak that rule forbids.

One option is already closed: identity cannot be pinned from a playlist ahead of the disc existing,
because a playlist yields no hash of its own.

---

## Other

### Variant groups are a display concern

Several genuinely different games shown as one tile. Grouping is **stored** — a user's choices about
it have to persist — but it must never merge or delete entries. Distinct from disc sets, which are a
*data* fact: one game, several files, correctly collapsed to one row.

### Two grouping mechanisms, deliberately separate

Their predicates are opposites. Discs of one release need compatible regions and *distinct* disc
numbers; variants need *equal* disc numbers across regions. One engine whose two policies contradict
each other on the axis that matters would buy less than it costs.
