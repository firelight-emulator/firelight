# Firelight modules

The goal of each module is to be a self-contained static library that can be used by the app. As much as possible, none
of them should depend on Qt or each other, though some dependencies are unavoidable in the current design...


## Modules

---

| Module | What it does                                                                               |
|---|--------------------------------------------------------------------------------------------|
| [`platforms`](platforms/README.md) | Contains hard-coded info about the platforms Firelight supports                            |                                                        |
| [`settings`](settings/README.md) | Models the settings structure for the app                                                  |
| [`activity`](activity/README.md) | Supports creation of play session records                                                  |
| [`audio`](audio/README.md) | Audio-related goodies like resampling, dynamic rate control, etc for the app and emulation |
| [`metadata`](metadata/README.md) | Classes and such to identify and look up info about games                                  |
| [`media`](media/README.md) | Manages a user's screenshots and video clips                                               |
| [`netplay`](netplay/README.md) | WIP                                                                                        |
| [`saves`](saves/README.md) | Responsible for managing save and suspend point data and metadata                          |
| [`cheats`](cheats/README.md) | Handles parsing and applying cheats for games, supports a few formats                      |
| [`mods`](mods/README.md) | WIP                                                                                        |
| [`library`](library/README.md) | Manages a user's library including scanning, ingesting, modifying, etc                     |
| [`achievements`](achievements/README.md) | Provides the RetroAchievements support including offline support                           |
| [`input`](input/README.md) | Manages input devices via SDL                                                              |
| [`discord`](discord/README.md) | For rich presence                                                                          |
