This document describes the directories and files that the application creates and manages on each user's machine.

All paths are relative to a directory ${APP_DIR} TODO

---
## `/system`

#### `/system/.cores`

This directory contains the emulator libretro cores used for the app's game emulation. They are identified by the filenames, so they must not change.
#### `/system/content.db`

This is the content database file that is shipped with the app. It contains the records for Games, ROMs, Rombacks, Platforms, etc. TODO: make page on content db

---
## `/userdata`

#### `/userdata/library.db`

#### `/userdata`
#### `/userdata/${rom_md5}`

#### `/userdata/${rom_md5}/savefiles`

#### `/userdata/${rom_md5}/savestates`

---
## `/roms`

This is the default directory for user's rom files. The app will automatically use this a source to scan for roms for the library, and it is not possible to remove this directory from that scan.

The directory structure under `/roms` is not relevant to the app and can be organized by the user however they like.