#include <firelight/platforms/platform_service.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <rcheevos/rc_consoles.h>
#include <spdlog/spdlog.h>

namespace firelight::platforms {
namespace {
// The one mapping between Firelight platform ids and rcheevos console ids, read in both
// directions. Rows are transcribed from the per-platform hashing switch this replaces
struct PlatformRcConsole {
  int platformId;
  int rcConsoleId;
};

constexpr std::array PLATFORM_RC_CONSOLES = {
    PlatformRcConsole{PlatformService::PLATFORM_ID_GAMEBOY, RC_CONSOLE_GAMEBOY},
    PlatformRcConsole{PlatformService::PLATFORM_ID_GAMEBOY_COLOR, RC_CONSOLE_GAMEBOY_COLOR},
    PlatformRcConsole{PlatformService::PLATFORM_ID_GAMEBOY_ADVANCE, RC_CONSOLE_GAMEBOY_ADVANCE},
    PlatformRcConsole{PlatformService::PLATFORM_ID_VIRTUAL_BOY, RC_CONSOLE_VIRTUAL_BOY},
    PlatformRcConsole{PlatformService::PLATFORM_ID_NES, RC_CONSOLE_NINTENDO},
    PlatformRcConsole{PlatformService::PLATFORM_ID_SNES, RC_CONSOLE_SUPER_NINTENDO},
    PlatformRcConsole{PlatformService::PLATFORM_ID_N64, RC_CONSOLE_NINTENDO_64},
    PlatformRcConsole{PlatformService::PLATFORM_ID_NINTENDO_DS, RC_CONSOLE_NINTENDO_DS},
    PlatformRcConsole{PlatformService::PLATFORM_ID_SEGA_MASTER_SYSTEM, RC_CONSOLE_MASTER_SYSTEM},
    PlatformRcConsole{PlatformService::PLATFORM_ID_SEGA_GENESIS, RC_CONSOLE_MEGA_DRIVE},
    PlatformRcConsole{PlatformService::PLATFORM_ID_SEGA_GAMEGEAR, RC_CONSOLE_GAME_GEAR},
    PlatformRcConsole{PlatformService::PLATFORM_ID_SEGA_SATURN, RC_CONSOLE_SATURN},
    PlatformRcConsole{PlatformService::PLATFORM_ID_SEGA_32X, RC_CONSOLE_SEGA_32X},
    PlatformRcConsole{PlatformService::PLATFORM_ID_SEGA_CD, RC_CONSOLE_SEGA_CD},
    PlatformRcConsole{PlatformService::PLATFORM_ID_PS1, RC_CONSOLE_PLAYSTATION},
    PlatformRcConsole{PlatformService::PLATFORM_ID_PS2, RC_CONSOLE_PLAYSTATION_2},
    PlatformRcConsole{PlatformService::PLATFORM_ID_PLAYSTATION_PORTABLE, RC_CONSOLE_PSP},
    PlatformRcConsole{PlatformService::PLATFORM_ID_TURBOGRAFX16, RC_CONSOLE_PC_ENGINE},
    PlatformRcConsole{PlatformService::PLATFORM_ID_PC_ENGINE_CD, RC_CONSOLE_PC_ENGINE_CD},
    PlatformRcConsole{PlatformService::PLATFORM_ID_POKEMON_MINI, RC_CONSOLE_POKEMON_MINI},
    PlatformRcConsole{PlatformService::PLATFORM_ID_WONDERSWAN, RC_CONSOLE_WONDERSWAN},
    PlatformRcConsole{PlatformService::PLATFORM_ID_SG1000, RC_CONSOLE_SG1000},
    PlatformRcConsole{PlatformService::PLATFORM_ID_FAMICOM_DISK_SYSTEM, RC_CONSOLE_FAMICOM_DISK_SYSTEM},
    PlatformRcConsole{PlatformService::PLATFORM_ID_NEOGEO_POCKET, RC_CONSOLE_NEOGEO_POCKET},
    PlatformRcConsole{PlatformService::PLATFORM_ID_3DO, RC_CONSOLE_3DO},
};

// TODO
// Neither a platform nor a console may appear twice: the reverse lookup would otherwise answer
// with whichever row happened to come first
constexpr bool isBijection() {
  for (size_t i = 0; i < PLATFORM_RC_CONSOLES.size(); ++i) {
    for (size_t j = i + 1; j < PLATFORM_RC_CONSOLES.size(); ++j) {
      if (PLATFORM_RC_CONSOLES[i].platformId == PLATFORM_RC_CONSOLES[j].platformId ||
          PLATFORM_RC_CONSOLES[i].rcConsoleId == PLATFORM_RC_CONSOLES[j].rcConsoleId) {
        return false;
      }
    }
  }

  return true;
}

static_assert(isBijection());
} // namespace

PlatformService::PlatformService() {
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_GAMEBOY,
      .name = "Game Boy",
      .abbreviation = "Game Boy",
      .slug = "gb",
      .fileAssociations = {"gb", "dmg"},
      .controllerTypes = {{.id = 1,
                           .name = "Retropad",
                           .imageUrl = "qrc:/images/controllers/gb",
                           .inputs =
                               {
                                   {"A", input::GamepadInput::EastFace},
                                   {"B", input::GamepadInput::SouthFace},
                                   {"Start", input::GamepadInput::Start},
                                   {"Select", input::GamepadInput::Select},
                                   {"D-Pad Up", input::GamepadInput::DpadUp},
                                   {"D-Pad Down", input::GamepadInput::DpadDown},
                                   {"D-Pad Left", input::GamepadInput::DpadLeft},
                                   {"D-Pad Right", input::GamepadInput::DpadRight},
                               }}},
  });
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_GAMEBOY_COLOR,
      .name = "Game Boy Color",
      .abbreviation = "Game Boy Color",
      .slug = "gbc",
      .fileAssociations = {"gbc"},
      .controllerTypes = {{.id = 1,
                           .name = "Retropad",
                           .imageUrl = "qrc:/images/controllers/gbc",
                           .inputs =
                               {
                                   {"A", input::GamepadInput::EastFace},
                                   {"B", input::GamepadInput::SouthFace},
                                   {"Start", input::GamepadInput::Start},
                                   {"Select", input::GamepadInput::Select},
                                   {"D-Pad Up", input::GamepadInput::DpadUp},
                                   {"D-Pad Down", input::GamepadInput::DpadDown},
                                   {"D-Pad Left", input::GamepadInput::DpadLeft},
                                   {"D-Pad Right", input::GamepadInput::DpadRight},
                               }}},
  });
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_GAMEBOY_ADVANCE,
      .name = "Game Boy Advance",
      .abbreviation = "Game Boy Advance",
      .slug = "gba",
      .fileAssociations = {"gba"},
      .controllerTypes = {{.id = 1,
                           .name = "Retropad",
                           .imageUrl = "qrc:/images/controllers/gba",
                           .inputs = {{"A", input::GamepadInput::EastFace},
                                      {"B", input::GamepadInput::SouthFace},
                                      {"A (turbo)", input::GamepadInput::NorthFace},
                                      {"B (turbo)", input::GamepadInput::WestFace},
                                      {"L", input::GamepadInput::LeftBumper},
                                      {"R", input::GamepadInput::RightBumper},
                                      {"L (turbo)", input::GamepadInput::LeftTrigger},
                                      {"R (turbo)", input::GamepadInput::RightTrigger},
                                      {"Start", input::GamepadInput::Start},
                                      {"Select", input::GamepadInput::Select},
                                      {"D-Pad Up", input::GamepadInput::DpadUp},
                                      {"D-Pad Down", input::GamepadInput::DpadDown},
                                      {"D-Pad Left", input::GamepadInput::DpadLeft},
                                      {"D-Pad Right", input::GamepadInput::DpadRight},
                                      {"Darken solar sensor", input::GamepadInput::L3},
                                      {"Lighten solar sensor", input::GamepadInput::R3}}}},
  });
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_NES,
      .name = "NES/Famicom",
      .abbreviation = "NES",
      .slug = "nes",
      .fileAssociations = {"nes", "unf", "unif"},
      .controllerTypes = {{.id = 1,
                           .name = "Retropad",
                           .imageUrl = "qrc:/images/controllers/nes",
                           .inputs =
                               {
                                   {"A", input::GamepadInput::EastFace},
                                   {"B", input::GamepadInput::SouthFace},
                                   {"Start", input::GamepadInput::Start},
                                   {"Select", input::GamepadInput::Select},
                                   {"D-Pad Up", input::GamepadInput::DpadUp},
                                   {"D-Pad Down", input::GamepadInput::DpadDown},
                                   {"D-Pad Left", input::GamepadInput::DpadLeft},
                                   {"D-Pad Right", input::GamepadInput::DpadRight},
                               }},
                          {.id = 3,
                           .name = "Zapper",
                           .imageUrl = "qrc:/images/controllers/placeholder",
                           .deviceClass = input::GamepadInputClass::Lightgun,
                           .inputs =
                               {
                                   {"Trigger", input::GamepadInput::LightgunTrigger},
                                   {"Shoot off-screen", input::GamepadInput::LightgunReload},
                               }}},
  });
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_FAMICOM_DISK_SYSTEM,
      .name = "Famicom Disk System",
      .abbreviation = "FDS",
      .slug = "fds",
      .fileAssociations = {"fds"},
      .controllerTypes = {{.id = 1,
                           .name = "Retropad",
                           .imageUrl = "qrc:/images/controllers/nes",
                           .inputs =
                               {
                                   {"A", input::GamepadInput::EastFace},
                                   {"B", input::GamepadInput::SouthFace},
                                   {"Start", input::GamepadInput::Start},
                                   {"Select", input::GamepadInput::Select},
                                   {"D-Pad Up", input::GamepadInput::DpadUp},
                                   {"D-Pad Down", input::GamepadInput::DpadDown},
                                   {"D-Pad Left", input::GamepadInput::DpadLeft},
                                   {"D-Pad Right", input::GamepadInput::DpadRight},
                               }}},
  });
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_SNES,
      .name = "SNES/Super Famicom",
      .abbreviation = "SNES",
      .slug = "snes",
      .fileAssociations = {"sfc", "smc", "swc", "fig", "bs", "st"},
      .controllerTypes = {{.id = 1,
                           .name = "Retropad",
                           .imageUrl = "qrc:/images/controllers/snes",
                           .inputs =
                               {
                                   {"A", input::GamepadInput::EastFace},
                                   {"B", input::GamepadInput::SouthFace},
                                   {"X", input::GamepadInput::NorthFace},
                                   {"Y", input::GamepadInput::WestFace},
                                   {"L", input::GamepadInput::LeftBumper},
                                   {"R", input::GamepadInput::RightBumper},
                                   {"Start", input::GamepadInput::Start},
                                   {"Select", input::GamepadInput::Select},
                                   {"D-Pad Up", input::GamepadInput::DpadUp},
                                   {"D-Pad Down", input::GamepadInput::DpadDown},
                                   {"D-Pad Left", input::GamepadInput::DpadLeft},
                                   {"D-Pad Right", input::GamepadInput::DpadRight},
                               }},
                          {.id = 2,
                           .name = "Mouse",
                           .imageUrl = "qrc:/images/controllers/placeholder",
                           .deviceClass = input::GamepadInputClass::Mouse,
                           .inputs =
                               {
                                   {"Left Button", input::GamepadInput::MouseLeft},
                                   {"Right Button", input::GamepadInput::MouseRight},
                               }},
                          {.id = 3,
                           .name = "Super Scope",
                           .imageUrl = "qrc:/images/controllers/placeholder",
                           .deviceClass = input::GamepadInputClass::Lightgun,
                           .inputs =
                               {
                                   {"Fire", input::GamepadInput::LightgunTrigger},
                                   {"Cursor", input::GamepadInput::LightgunAuxA},
                                   {"Turbo", input::GamepadInput::LightgunAuxB},
                                   {"Pause", input::GamepadInput::LightgunStart},
                               }}},
  });
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_N64,
      .name = "Nintendo 64",
      .abbreviation = "N64",
      .slug = "n64",
      .fileAssociations = {"n64", "z64", "v64"},
      .controllerTypes = {{.id = 1,
                           .name = "Retropad",
                           .imageUrl = "qrc:/images/controllers/n64",
                           .inputs =
                               {
                                   {"A", input::GamepadInput::SouthFace},
                                   {"B", input::GamepadInput::WestFace},
                                   {"Z", input::GamepadInput::LeftTrigger},
                                   {"L", input::GamepadInput::LeftBumper},
                                   {"R", input::GamepadInput::RightBumper},
                                   {"Start", input::GamepadInput::Start},
                                   {"Select", input::GamepadInput::Select},
                                   {"C-Up", input::GamepadInput::RightStickUp},
                                   {"C-Down", input::GamepadInput::RightStickDown},
                                   {"C-Left", input::GamepadInput::RightStickLeft},
                                   {"C-Right", input::GamepadInput::RightStickRight},
                                   {"D-Pad Up", input::GamepadInput::DpadUp},
                                   {"D-Pad Down", input::GamepadInput::DpadDown},
                                   {"D-Pad Left", input::GamepadInput::DpadLeft},
                                   {"D-Pad Right", input::GamepadInput::DpadRight},
                                   {"Analog Stick Up", input::GamepadInput::LeftStickUp},
                                   {"Analog Stick Down", input::GamepadInput::LeftStickDown},
                                   {"Analog Stick Left", input::GamepadInput::LeftStickLeft},
                                   {"Analog Stick Right", input::GamepadInput::LeftStickRight},
                               }}},
  });
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_NINTENDO_DS,
      .name = "Nintendo DS",
      .abbreviation = "NDS",
      .slug = "nds",
      .fileAssociations = {"nds"},
      .discordImage = "ds",
      .controllerTypes = {{.id = 1,
                           .name = "Retropad",
                           .imageUrl = "qrc:/images/controllers/nds",
                           .inputs =
                               {
                                   {"A", input::GamepadInput::EastFace},
                                   {"B", input::GamepadInput::SouthFace},
                                   {"X", input::GamepadInput::NorthFace},
                                   {"Y", input::GamepadInput::WestFace},
                                   {"L", input::GamepadInput::LeftBumper},
                                   {"R", input::GamepadInput::RightBumper},
                                   {"Start", input::GamepadInput::Start},
                                   {"Select", input::GamepadInput::Select},
                                   {"D-Pad Up", input::GamepadInput::DpadUp},
                                   {"D-Pad Down", input::GamepadInput::DpadDown},
                                   {"D-Pad Left", input::GamepadInput::DpadLeft},
                                   {"D-Pad Right", input::GamepadInput::DpadRight},
                                   {"Move cursor up", input::GamepadInput::RightStickUp},
                                   {"Move cursor down", input::GamepadInput::RightStickDown},
                                   {"Move cursor left", input::GamepadInput::RightStickLeft},
                                   {"Move cursor right", input::GamepadInput::RightStickRight},
                                   {"Touch with cursor", input::GamepadInput::R3},
                                   {"Enable microphone (hold)", input::GamepadInput::LeftTrigger},
                                   {"Change screen layout", input::GamepadInput::RightTrigger},
                                   {"Close lid", input::GamepadInput::L3},
                               }}},
  });

  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_SG1000,
      .name = "SG-1000",
      .abbreviation = "SG-1000",
      .slug = "sg",
      .fileAssociations = {"sg"},
      .controllerTypes = {{.id = 1,
                           .name = "Retropad",
                           .imageUrl = "qrc:/images/controllers/placeholder",
                           .inputs =
                               {
                                   {"1", input::GamepadInput::SouthFace},
                                   {"2", input::GamepadInput::EastFace},
                                   {"Start", input::GamepadInput::Start},
                                   {"D-Pad Up", input::GamepadInput::DpadUp},
                                   {"D-Pad Down", input::GamepadInput::DpadDown},
                                   {"D-Pad Left", input::GamepadInput::DpadLeft},
                                   {"D-Pad Right", input::GamepadInput::DpadRight},
                               }}},
  });

  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_SEGA_MASTER_SYSTEM,
      .name = "Master System",
      .abbreviation = "Master System",
      .slug = "sms",
      .fileAssociations = {"sms", "bms"},
      .controllerTypes = {{.id = 1,
                           .name = "Retropad",
                           .imageUrl = "qrc:/images/controllers/sms",
                           .inputs =
                               {
                                   {"1", input::GamepadInput::SouthFace},
                                   {"2", input::GamepadInput::EastFace},
                                   {"Start", input::GamepadInput::Start},
                                   {"D-Pad Up", input::GamepadInput::DpadUp},
                                   {"D-Pad Down", input::GamepadInput::DpadDown},
                                   {"D-Pad Left", input::GamepadInput::DpadLeft},
                                   {"D-Pad Right", input::GamepadInput::DpadRight},
                               }},
                          {.id = 3,
                           .name = "Light Phaser",
                           .imageUrl = "qrc:/images/controllers/placeholder",
                           .deviceClass = input::GamepadInputClass::Lightgun,
                           .inputs =
                               {
                                   {"Trigger", input::GamepadInput::LightgunTrigger},
                               }}},
  });
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_SEGA_GENESIS,
      .name = "Genesis/Mega Drive",
      .abbreviation = "Genesis/MD",
      .slug = "gen",
      // smd and mdx are containers rather than dumps, and ContentHasher normalises both back to
      // the plain ROM so they carry the same identity as the .md of the same game
      .fileAssociations = {"md", "gen", "smd", "mdx"},
      .controllerTypes = {{.id = 1,
                           .name = "Retropad",
                           .imageUrl = "qrc:/images/controllers/gen-threebutton",
                           .inputs =
                               {
                                   {"A", input::GamepadInput::WestFace},
                                   {"B", input::GamepadInput::SouthFace},
                                   {"C", input::GamepadInput::EastFace},
                                   {"X", input::GamepadInput::LeftBumper},
                                   {"Y", input::GamepadInput::NorthFace},
                                   {"Z", input::GamepadInput::RightBumper},
                                   {"Start", input::GamepadInput::Start},
                                   {"Mode", input::GamepadInput::Select},
                                   {"D-Pad Up", input::GamepadInput::DpadUp},
                                   {"D-Pad Down", input::GamepadInput::DpadDown},
                                   {"D-Pad Left", input::GamepadInput::DpadLeft},
                                   {"D-Pad Right", input::GamepadInput::DpadRight},
                               }},
                          {.id = 2,
                           .name = "Mouse",
                           .imageUrl = "qrc:/images/controllers/placeholder",
                           .deviceClass = input::GamepadInputClass::Mouse,
                           .inputs =
                               {
                                   {"Left Button", input::GamepadInput::MouseLeft},
                                   {"Right Button", input::GamepadInput::MouseRight},
                                   {"Middle Button", input::GamepadInput::MouseMiddle},
                               }},
                          {.id = 3,
                           .name = "Light Gun",
                           .imageUrl = "qrc:/images/controllers/placeholder",
                           .deviceClass = input::GamepadInputClass::Lightgun,
                           .inputs =
                               {
                                   {"Trigger", input::GamepadInput::LightgunTrigger},
                                   {"B", input::GamepadInput::LightgunAuxA},
                                   {"C", input::GamepadInput::LightgunAuxB},
                                   {"Start", input::GamepadInput::LightgunStart},
                               }}},
  });
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_SEGA_GAMEGEAR,
      .name = "Game Gear",
      .abbreviation = "Game Gear",
      .slug = "gg",
      .fileAssociations = {"gg"},
      .controllerTypes = {{.id = 1,
                           .name = "Retropad",
                           .imageUrl = "qrc:/images/controllers/gg",
                           .inputs =
                               {
                                   {"1", input::GamepadInput::SouthFace},
                                   {"2", input::GamepadInput::EastFace},
                                   {"Start", input::GamepadInput::Start},
                                   {"D-Pad Up", input::GamepadInput::DpadUp},
                                   {"D-Pad Down", input::GamepadInput::DpadDown},
                                   {"D-Pad Left", input::GamepadInput::DpadLeft},
                                   {"D-Pad Right", input::GamepadInput::DpadRight},
                               }}},
  });
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_PLAYSTATION_PORTABLE,
      .name = "PlayStation Portable",
      .abbreviation = "PSP",
      .slug = "psp",
      .fileAssociations = {},
      .controllerTypes = {{.id = 1,
                           .name = "Retropad",
                           .imageUrl = "qrc:/images/controllers/psp",
                           .inputs =
                               {
                                   {"Cross", input::GamepadInput::SouthFace},
                                   {"Circle", input::GamepadInput::EastFace},
                                   {"Square", input::GamepadInput::WestFace},
                                   {"Triangle", input::GamepadInput::NorthFace},
                                   {"L", input::GamepadInput::LeftBumper},
                                   {"R", input::GamepadInput::RightBumper},
                                   {"Start", input::GamepadInput::Start},
                                   {"Select", input::GamepadInput::Select},
                                   {"D-Pad Up", input::GamepadInput::DpadUp},
                                   {"D-Pad Down", input::GamepadInput::DpadDown},
                                   {"D-Pad Left", input::GamepadInput::DpadLeft},
                                   {"D-Pad Right", input::GamepadInput::DpadRight},
                               }}},
  });

  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_TURBOGRAFX16,
      .name = "PC Engine/TurboGrafx-16",
      .abbreviation = "PCE/TG-16",
      .slug = "pce",
      .fileAssociations = {"pce", "sgx"},
      .controllerTypes = {{.id = 1,
                           .name = "Retropad",
                           .imageUrl = "qrc:/images/controllers/tgx-twobutton",
                           .inputs =
                               {
                                   {"I", input::GamepadInput::EastFace},
                                   {"II", input::GamepadInput::SouthFace},
                                   {"III (six-button mode)", input::GamepadInput::WestFace},
                                   {"IV (six-button mode)", input::GamepadInput::NorthFace},
                                   {"V (six-button mode)", input::GamepadInput::LeftBumper},
                                   {"VI (six-button mode)", input::GamepadInput::RightBumper},
                                   {"Toggle mode", input::GamepadInput::LeftTrigger},
                                   {"Run", input::GamepadInput::Start},
                                   {"Select", input::GamepadInput::Select},
                                   {"D-Pad Up", input::GamepadInput::DpadUp},
                                   {"D-Pad Down", input::GamepadInput::DpadDown},
                                   {"D-Pad Left", input::GamepadInput::DpadLeft},
                                   {"D-Pad Right", input::GamepadInput::DpadRight},
                               }}},
  });
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_PC_ENGINE_CD,
      .name = "PC Engine CD/TurboGrafx-CD",
      .abbreviation = "PCE-CD/TG-CD",
      .slug = "pcecd",
      .controllerTypes = {{.id = 1,
                           .name = "Retropad",
                           .imageUrl = "qrc:/images/controllers/tgx-twobutton",
                           .inputs =
                               {
                                   {"I", input::GamepadInput::EastFace},
                                   {"II", input::GamepadInput::SouthFace},
                                   {"III (six-button mode)", input::GamepadInput::WestFace},
                                   {"IV (six-button mode)", input::GamepadInput::NorthFace},
                                   {"V (six-button mode)", input::GamepadInput::LeftBumper},
                                   {"VI (six-button mode)", input::GamepadInput::RightBumper},
                                   {"Toggle mode", input::GamepadInput::LeftTrigger},
                                   {"Run", input::GamepadInput::Start},
                                   {"Select", input::GamepadInput::Select},
                                   {"D-Pad Up", input::GamepadInput::DpadUp},
                                   {"D-Pad Down", input::GamepadInput::DpadDown},
                                   {"D-Pad Left", input::GamepadInput::DpadLeft},
                                   {"D-Pad Right", input::GamepadInput::DpadRight},
                               }}},
  });
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_POKEMON_MINI,
      .name = "Pokémon Mini",
      .abbreviation = "Pokémon Mini",
      .slug = "pkmn",
      .fileAssociations = {"min"},
      .controllerTypes = {{.id = 1,
                           .name = "Retropad",
                           .imageUrl = "qrc:/images/controllers/pkmn-mini",
                           .inputs =
                               {
                                   {"A", input::GamepadInput::EastFace},
                                   {"B", input::GamepadInput::SouthFace},
                                   {"C", input::GamepadInput::RightBumper},
                                   {"D-Pad Up", input::GamepadInput::DpadUp},
                                   {"D-Pad Down", input::GamepadInput::DpadDown},
                                   {"D-Pad Left", input::GamepadInput::DpadLeft},
                                   {"D-Pad Right", input::GamepadInput::DpadRight},
                                   {"Shake", input::GamepadInput::LeftBumper},
                                   {"Power", input::GamepadInput::Select},
                               }}},
  });
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_WONDERSWAN,
      .name = "WonderSwan",
      .abbreviation = "WonderSwan",
      .slug = "ws",
      .fileAssociations = {"ws", "wsc", "pc2"},
      .controllerTypes = {{.id = 1,
                           .name = "Retropad",
                           .imageUrl = "qrc:/images/controllers/ws-horizontal",
                           .inputs =
                               {
                                   {"X1 (Horizontal), Y2 (Vertical)", input::GamepadInput::DpadUp},
                                   {"X2 (Horizontal), Y3 (Vertical)", input::GamepadInput::DpadRight},
                                   {"X3 (Horizontal), Y4 (Vertical)", input::GamepadInput::DpadDown},
                                   {"X4 (Horizontal), Y1 (Vertical)", input::GamepadInput::DpadLeft},
                                   {"A (Horizontal), X3 (Vertical)", input::GamepadInput::EastFace},
                                   {"B (Horizontal), X4 (Vertical)", input::GamepadInput::SouthFace},
                                   {"X1 (Horizontal)", input::GamepadInput::WestFace},
                                   {"X2 (Horizontal)", input::GamepadInput::NorthFace},
                                   {"Start", input::GamepadInput::Start},
                                   {"Switch orientation", input::GamepadInput::Select},
                               }}},
  });
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_NEOGEO_POCKET,
      .name = "NeoGeo Pocket",
      .abbreviation = "NeoGeo Pocket",
      .slug = "ngp",
      .fileAssociations = {"ngp", "ngc", "ngpc", "npc"},
      .controllerTypes = {{.id = 1,
                           .name = "Retropad",
                           .imageUrl = "qrc:/images/controllers/ngp",
                           .inputs =
                               {
                                   {"A", input::GamepadInput::SouthFace},
                                   {"B", input::GamepadInput::EastFace},
                                   {"Option", input::GamepadInput::Start},
                                   {"D-Pad Up", input::GamepadInput::DpadUp},
                                   {"D-Pad Down", input::GamepadInput::DpadDown},
                                   {"D-Pad Left", input::GamepadInput::DpadLeft},
                                   {"D-Pad Right", input::GamepadInput::DpadRight},
                               }}},
  });
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_PS1,
      .name = "PlayStation",
      .abbreviation = "PS1",
      .slug = "ps1",
      .fileAssociations = {},
      .controllerTypes = {{.id = 1,
                           .name = "Retropad",
                           .imageUrl = "qrc:/images/controllers/ngp",
                           .inputs =
                               {
                                   {"A", input::GamepadInput::SouthFace},
                                   {"B", input::GamepadInput::EastFace},
                                   {"Option", input::GamepadInput::Start},
                                   {"D-Pad Up", input::GamepadInput::DpadUp},
                                   {"D-Pad Down", input::GamepadInput::DpadDown},
                                   {"D-Pad Left", input::GamepadInput::DpadLeft},
                                   {"D-Pad Right", input::GamepadInput::DpadRight},
                               }}},
  });

  // Modeled with legacy ids but without full controller/extension data yet
  // (identity only, so names/abbreviations still render)
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_VIRTUAL_BOY,
      .name = "Virtual Boy",
      .abbreviation = "Virtual Boy",
      .slug = "vb",
      .fileAssociations = {"vb"},
      .controllerTypes = {{.id = 1,
                           .name = "Retropad",
                           .imageUrl = "qrc:/images/controllers/placeholder",
                           .inputs =
                               {
                                   {"A", input::GamepadInput::EastFace},
                                   {"B", input::GamepadInput::SouthFace},
                                   {"L", input::GamepadInput::LeftBumper},
                                   {"R", input::GamepadInput::RightBumper},
                                   {"Start", input::GamepadInput::Start},
                                   {"Select", input::GamepadInput::Select},
                                   {"Left D-Pad Up", input::GamepadInput::DpadUp},
                                   {"Left D-Pad Down", input::GamepadInput::DpadDown},
                                   {"Left D-Pad Left", input::GamepadInput::DpadLeft},
                                   {"Left D-Pad Right", input::GamepadInput::DpadRight},
                                   {"Right D-Pad Up", input::GamepadInput::RightStickUp},
                                   {"Right D-Pad Down", input::GamepadInput::RightStickDown},
                                   {"Right D-Pad Left", input::GamepadInput::RightStickLeft},
                                   {"Right D-Pad Right", input::GamepadInput::RightStickRight},
                               }}},
  });
  m_platforms.emplace_back(Platform{.id = PLATFORM_ID_SEGA_SATURN, .name = "Sega Saturn", .abbreviation = "Saturn"});
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_SEGA_32X,
      .name = "Sega 32X",
      .abbreviation = "32X",
      .slug = "32x",
      .fileAssociations = {"32x"},
      .controllerTypes = {{.id = 1,
                           .name = "Retropad",
                           .imageUrl = "qrc:/images/controllers/gen-threebutton",
                           .inputs =
                               {
                                   {"A", input::GamepadInput::WestFace},
                                   {"B", input::GamepadInput::SouthFace},
                                   {"C", input::GamepadInput::EastFace},
                                   {"X", input::GamepadInput::LeftBumper},
                                   {"Y", input::GamepadInput::NorthFace},
                                   {"Z", input::GamepadInput::RightBumper},
                                   {"Start", input::GamepadInput::Start},
                                   {"Mode", input::GamepadInput::Select},
                                   {"D-Pad Up", input::GamepadInput::DpadUp},
                                   {"D-Pad Down", input::GamepadInput::DpadDown},
                                   {"D-Pad Left", input::GamepadInput::DpadLeft},
                                   {"D-Pad Right", input::GamepadInput::DpadRight},
                               }}},
  });
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_SEGA_CD,
      .name = "Sega CD/Mega CD",
      .abbreviation = "Sega CD",
      .slug = "segacd",
      .controllerTypes = {{.id = 1,
                           .name = "Retropad",
                           .imageUrl = "qrc:/images/controllers/gen-threebutton",
                           .inputs =
                               {
                                   {"A", input::GamepadInput::WestFace},
                                   {"B", input::GamepadInput::SouthFace},
                                   {"C", input::GamepadInput::EastFace},
                                   {"X", input::GamepadInput::LeftBumper},
                                   {"Y", input::GamepadInput::NorthFace},
                                   {"Z", input::GamepadInput::RightBumper},
                                   {"Start", input::GamepadInput::Start},
                                   {"Mode", input::GamepadInput::Select},
                                   {"D-Pad Up", input::GamepadInput::DpadUp},
                                   {"D-Pad Down", input::GamepadInput::DpadDown},
                                   {"D-Pad Left", input::GamepadInput::DpadLeft},
                                   {"D-Pad Right", input::GamepadInput::DpadRight},
                               }}},
  });
  m_platforms.emplace_back(Platform{.id = PLATFORM_ID_PS2, .name = "PlayStation 2", .abbreviation = "PS2"});
  m_platforms.emplace_back(
      Platform{.id = PLATFORM_ID_3DO, .name = "3DO Interactive Multiplayer", .abbreviation = "3DO"});

  // TODO
  // A view of the table rather than a value anybody maintains by hand, so the two cannot disagree
  for (auto &platform : m_platforms) {
    platform.retroAchievementsId = static_cast<unsigned>(rcConsoleForPlatform(static_cast<int>(platform.id)));
  }

  buildExtensionIndex();
}

std::optional<Platform> PlatformService::getPlatform(const unsigned id) const {
  for (const auto &platform : m_platforms) {
    if (platform.id == id) {
      return platform;
    }
  }
  return {};
}

std::vector<Platform> PlatformService::listPlatforms() const { return m_platforms; }

void PlatformService::buildExtensionIndex() {
  for (const auto &platform : m_platforms) {
    for (const auto &association : platform.fileAssociations) {
      const auto [it, inserted] = m_platformIdByExtension.emplace(association, static_cast<int>(platform.id));

      // TODO
      // Two platforms claiming one extension has no right answer, and a scan over the list
      // would silently hand back whichever was constructed first
      if (!inserted) {
        spdlog::warn("Extension {} is claimed by platforms {} and {}; keeping {}", association, it->second, platform.id,
                     it->second);
      }
    }
  }
}

int PlatformService::platformIdForExtension(const std::string &extension) const {
  std::string lower = extension;
  std::transform(lower.begin(), lower.end(), lower.begin(), [](const unsigned char c) { return std::tolower(c); });

  const auto it = m_platformIdByExtension.find(lower);
  return it == m_platformIdByExtension.end() ? PLATFORM_ID_UNKNOWN : it->second;
}

int PlatformService::platformIdForRcConsole(const int rcConsoleId) const {
  for (const auto &[platformId, consoleId] : PLATFORM_RC_CONSOLES) {
    if (consoleId == rcConsoleId) {
      return platformId;
    }
  }

  return PLATFORM_ID_UNKNOWN;
}

int PlatformService::rcConsoleForPlatform(const int platformId) {
  for (const auto &[id, consoleId] : PLATFORM_RC_CONSOLES) {
    if (id == platformId) {
      return consoleId;
    }
  }

  return RC_CONSOLE_UNKNOWN;
}
} // namespace firelight::platforms
