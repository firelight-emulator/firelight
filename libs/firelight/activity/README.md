# Firelight Activity Module

## Usage

---

You pretty much just need to create an instance of IActivityLog (only one implementation exists in this module: SqliteActivityLog) 
and start using it. Fairly straightforward :) You'll need to create and manage new PlaySession objects and put them into the
log when they're done. It doesn't currently support any sort of "live" tracking of play sessions, but that could be added in the future if needed.

The classes here don't really do any calculations or validations, so callers need to manage them correctly.

## Architecture

---

The activity module is a single interface (IActivityLog) with one SQLite-backed implementation that persists and retrieves PlaySession records.

## Data Structures

---

### Activity Log
The entrypoint into the module. Lets you create and retrieve play sessions.

### Play Session
Represents the start and end time for a user's game session. Also keeps track of the number of seconds during which the 
game was NOT paused.