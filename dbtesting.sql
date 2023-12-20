CREATE TABLE IF NOT EXISTS Users
(
    ID          INTEGER PRIMARY KEY AUTOINCREMENT,
    DisplayName TEXT NOT NULL,
    Email       TEXT NOT NULL,
    JoinedAt    DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS EnforcementTypes
(
    ID                     INTEGER PRIMARY KEY AUTOINCREMENT,
    DefaultDurationMinutes INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS Enforcements
(
    ID              INTEGER PRIMARY KEY AUTOINCREMENT,
    User            INTEGER NOT NULL,
    FOREIGN KEY (User) REFERENCES Users (ID),
    EnforcementType INTEGER NOT NULL,
    FOREIGN KEY (EnforcementType) REFERENCES EnforcementTypes (ID),
    IssuedAt        DATETIME DEFAULT CURRENT_TIMESTAMP,
    EndsAt          DATETIME
);

CREATE TABLE IF NOT EXISTS games
(
    id                   INTEGER PRIMARY KEY AUTOINCREMENT,
    name                 TEXT NOT NULL,
    extern_id_twitchigdb INTEGER DEFAULT -1
);

CREATE TABLE IF NOT EXISTS roms
(
    id                INTEGER PRIMARY KEY AUTOINCREMENT,
    filename          TEXT    NOT NULL,
    file_ext          TEXT    NOT NULL,
    game              INTEGER NOT NULL,
    platform          TEXT    NOT NULL,
    region            TEXT    NOT NULL,
    md5               TEXT    NOT NULL,
    size_bytes        INTEGER NOT NULL,
    extern_id_nointro TEXT,
    FOREIGN KEY (game) REFERENCES games (ID)
);
CREATE INDEX idx_rom_md5 ON roms (Md5);

CREATE TABLE IF NOT EXISTS RomHacks
(
    ID  INTEGER PRIMARY KEY AUTOINCREMENT,
    Rom INTEGER NOT NULL,
    FOREIGN KEY (Rom) REFERENCES roms (ID)
);

CREATE TABLE IF NOT EXISTS RomHackReleases
(
    ID        INTEGER PRIMARY KEY AUTOINCREMENT,
    CreatedAt DATETIME DEFAULT CURRENT_TIMESTAMP,
    Version   TEXT    NOT NULL,
    RomHack   INTEGER NOT NULL,
    FOREIGN KEY (RomHack) REFERENCES RomHacks (ID)
);

CREATE TABLE IF NOT EXISTS Library
(
    ID          INTEGER PRIMARY KEY AUTOINCREMENT,
    DisplayName TEXT NOT NULL,
    EntryType   TEXT NOT NULL,
    Platform    TEXT NOT NULL,
    Verified    INTEGER CHECK (Verified IN (0, 1)),
    Game        INTEGER,
    Rom         INTEGER
);

INSERT INTO CoolGuy (NAME)
VALUES ('Alex');

SELECT *
FROM CoolGuy;

