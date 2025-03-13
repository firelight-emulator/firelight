#include "SqliteModRepository.h"

#include "../platform_metadata.hpp"

namespace firelight::mods {
SqliteModRepository::SqliteModRepository() {
  m_mods[1] = ModInfo{
    .name = "Mario Kart 64: Amped Up",
    .version = "1.0",
    .author = "Litronom",
    .targetGameName = "Mario Kart 64",
    .targetContentHash = "",
    .platformId = PlatformMetadata::PLATFORM_ID_N64,
    .tagline = "",
    .description = "",
    .clearLogoUrl = "qrc:images/mods/mk64ampedup/clearlogo",
    .mediaUrls = {"qrc:images/mods/mk64ampedup/pic1", "qrc:images/mods/mk64ampedup/pic2", "qrc:images/mods/mk64ampedup/pic3", "qrc:images/mods/mk64ampedup/pic4", "qrc:images/mods/mk64ampedup/pic5", "qrc:images/mods/mk64ampedup/pic6"}
  };
  m_mods[2] = ModInfo{
    .name = "Pokémon Radical Red",
    .version = "1.0",
    .author = "soupercell",
    .targetGameName = "Pokémon FireRed Version",
    .targetContentHash = "",
    .platformId = PlatformMetadata::PLATFORM_ID_GAMEBOY_ADVANCE,
    .tagline = "",
    .description = "",
    .clearLogoUrl = "qrc:images/mods/radicalred/clearlogo",
    .mediaUrls = {"qrc:images/mods/radicalred/pic1", "qrc:images/mods/radicalred/pic2", "qrc:images/mods/radicalred/pic3", "qrc:images/mods/radicalred/pic4", "qrc:images/mods/radicalred/pic5", "qrc:images/mods/radicalred/pic6", "qrc:images/mods/radicalred/pic7", "qrc:images/mods/radicalred/pic8"}
  };
  m_mods[3] = ModInfo{
    .name = "Tajna and the Mana Seeds",
    .version = "1.0",
    .author = "Tob",
    .targetGameName = "Super Mario World",
    .targetContentHash = "",
    .platformId = PlatformMetadata::PLATFORM_ID_SNES,
    .tagline = "Take control of Tajna in this complete overhaul hack for Super Mario World!",
    .description = "Fight, Jump and Spin yourself through these unique stages! Each of the 6 stages comes with their own collectibles and platforming challenges. This hack takes inspiration from old and new games and romhacks to introduce its mechanics in a simple and intuitive way.",
    .clearLogoUrl = "qrc:images/mods/tajna/clearlogo",
    .mediaUrls = {"qrc:images/mods/tajna/pic1", "qrc:images/mods/tajna/pic2", "qrc:images/mods/tajna/pic3", "qrc:images/mods/tajna/pic4", "qrc:images/mods/tajna/pic5", "qrc:images/mods/tajna/pic6"}
  };
  m_mods[4] = ModInfo{
    .name = "Ultimate Goomboss Challenge",
    .version = "1.0",
    .author = "Enneagram",
    .targetGameName = "Paper Mario",
    .targetContentHash = "",
    .platformId = PlatformMetadata::PLATFORM_ID_N64,
    .tagline = "Saving the world is hungry work.",
    .description = "With the Star Rod recovered, a satisfied Mario and Goombario head out for a bite to eat. But who's this in their favourite picnic spot? And more importantly, can you come up with a strategy to defeat... THE ULTIMATE GOOMBOSS CHALLENGE? Made as a love letter to RPG boss battles, and a determination to make the most epic possible final product using only Goombas.",
    .clearLogoUrl = "qrc:images/mods/goomboss/clearlogo",
    .mediaUrls = {"qrc:images/mods/goomboss/pic1", "qrc:images/mods/goomboss/pic2", "qrc:images/mods/goomboss/pic3", "qrc:images/mods/goomboss/pic4","qrc:images/mods/goomboss/pic5"}
  };
  m_mods[5] = ModInfo{
    .name = "Super Mario 64: Beyond the Cursed Mirror",
    .version = "1.0",
    .author = "rovertronic",
    .targetGameName = "Super Mario 64",
    .targetContentHash = "20b854b239203baf6c961b850a4a51a2",
    .platformId = PlatformMetadata::PLATFORM_ID_N64,
    .tagline = "Greetings Superstar, did you know that your nemesis hid in the decrepit castle all this time? It appears he fosters his strength in a hidden compartment behind one cursed mirror. I implore you to defeat him, once and for all – for the sake of the kingdom. -Yours truly, The Showrunner",
    .description = "SM64: Beyond the Cursed Mirror is a major Super Mario 64 ROM hack created by Rovert, which contains 15 custom-made courses and 120 stars waiting to be collected. With a unique in-depth story, original characters, and new mechanics, this SM64 ROM hack will provide a traditional yet unique experience.",
    .clearLogoUrl = "qrc:images/mods/cursedmirror/clearlogo",
    .mediaUrls = {"qrc:images/mods/cursedmirror/pic1", "qrc:images/mods/cursedmirror/pic2", "qrc:images/mods/cursedmirror/pic3", "qrc:images/mods/cursedmirror/pic4", "qrc:images/mods/cursedmirror/pic5", "qrc:images/mods/cursedmirror/pic6"}
  };
}

std::optional<ModInfo> SqliteModRepository::getModInfo(const int modId) {

  if (!m_mods.contains(modId)) {
    return {};
  }

  return {m_mods[modId]};
}
} // mods
// firelight