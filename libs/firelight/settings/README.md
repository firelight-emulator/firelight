# Settings module

This module pretty much just reads JSON documents that have the settings defined and provides a way to access them

For Firelight, the settings are in data/settings, but that's not defined here. Just FYI

The settings are broken down like this:

### Settings
- An individual setting. Can be set as a global app setting, an emulation setting, or a per-core setting

### Groups
- A named list of settings ("groups" json key)

### Pages
- A named list of groups ("pages" json key)