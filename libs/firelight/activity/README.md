# Firelight Activity Module
This is pretty much just one data type (Play Session) and the interface used to persist them.

## Architecture

---

```mermaid
classDiagram
direction TB

class IActivityLog {
  <<interface>>
  +createPlaySession(PlaySession) bool
  +getLatestPlaySession(string hash) optional~PlaySession~
  +getPlaySessions(string hash) vector~PlaySession~
  +getPlaySessions() vector~PlaySession~
}

class SqliteActivityLog {
  +SqliteActivityLog(QString dbPath)
  +createPlaySession(PlaySession) bool
  +getLatestPlaySession(string hash) optional~PlaySession~
  +getPlaySessions() vector~PlaySession~
  +getDatabase() QSqlDatabase
  -QString databasePath
}

class PlaySession {
  +int id
  +string contentHash
  +uint slotNumber
  +uint64 startTime
  +uint64 endTime
  +uint64 unpausedDurationMillis
}

class QSqlDatabase {
  <<Qt6::Sql>>
}

IActivityLog <|-- SqliteActivityLog
IActivityLog ..> PlaySession : accepts / returns
SqliteActivityLog ..> PlaySession : creates, writes id
SqliteActivityLog --> QSqlDatabase : per-thread conn

%% Omitted: PlaySession.contentId (declared but never persisted); external consumers EmulatorItemRenderer (writer) and EntryListModel (reader) live outside this module and reach it via ServiceAccessor.
```

The activity module is a single interface (IActivityLog) with one SQLite-backed implementation that persists and retrieves PlaySession records over per-thread QSqlDatabase connections.

## Data Structures

---

### Activity Log
The entrypoint into the module. Lets you create and retrieve play sessions.

### Play Session
Represents the start and end time for a user's game session. Also keeps track of the number of seconds during which the 
game was NOT paused.