#include <firelight/platforms/platform_service.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <rcheevos/rc_consoles.h>

namespace firelight::platforms {
namespace {
// RA-supported consoles Firelight does not fully model (no controller/extension
// data). Each becomes an identity-only Platform (id = 1000 + rcheevos console
// id) so its name still renders in the library. Names mirror the previous
// PlatformMetadata::getPlatformName mapping
struct RaCoverageConsole {
  int rcConsoleId;
  const char *name;
};

constexpr std::array RA_COVERAGE_CONSOLES = {
    RaCoverageConsole{RC_CONSOLE_ATARI_LYNX, "Atari Lynx"},
    RaCoverageConsole{RC_CONSOLE_GAMECUBE, "Nintendo GameCube"},
    RaCoverageConsole{RC_CONSOLE_ATARI_JAGUAR, "Atari Jaguar"},
    RaCoverageConsole{RC_CONSOLE_WII, "Nintendo Wii"},
    RaCoverageConsole{RC_CONSOLE_WII_U, "Nintendo Wii U"},
    RaCoverageConsole{RC_CONSOLE_XBOX, "Xbox"},
    RaCoverageConsole{RC_CONSOLE_MAGNAVOX_ODYSSEY2, "Magnavox Odyssey 2"},
    RaCoverageConsole{RC_CONSOLE_ATARI_2600, "Atari 2600"},
    RaCoverageConsole{RC_CONSOLE_MS_DOS, "MS-DOS"},
    RaCoverageConsole{RC_CONSOLE_ARCADE, "Arcade"},
    RaCoverageConsole{RC_CONSOLE_MSX, "MSX"},
    RaCoverageConsole{RC_CONSOLE_COMMODORE_64, "Commodore 64"},
    RaCoverageConsole{RC_CONSOLE_ZX81, "ZX81"},
    RaCoverageConsole{RC_CONSOLE_ORIC, "Oric"},
    RaCoverageConsole{RC_CONSOLE_VIC20, "Commodore VIC-20"},
    RaCoverageConsole{RC_CONSOLE_AMIGA, "Amiga"},
    RaCoverageConsole{RC_CONSOLE_ATARI_ST, "Atari ST"},
    RaCoverageConsole{RC_CONSOLE_AMSTRAD_PC, "Amstrad CPC"},
    RaCoverageConsole{RC_CONSOLE_APPLE_II, "Apple II"},
    RaCoverageConsole{RC_CONSOLE_DREAMCAST, "Sega Dreamcast"},
    RaCoverageConsole{RC_CONSOLE_CDI, "Philips CD-i"},
    RaCoverageConsole{RC_CONSOLE_3DO, "3DO Interactive Multiplayer"},
    RaCoverageConsole{RC_CONSOLE_COLECOVISION, "ColecoVision"},
    RaCoverageConsole{RC_CONSOLE_INTELLIVISION, "Intellivision"},
    RaCoverageConsole{RC_CONSOLE_VECTREX, "Vectrex"},
    RaCoverageConsole{RC_CONSOLE_PC8800, "PC-8800"},
    RaCoverageConsole{RC_CONSOLE_PC9800, "PC-9800"},
    RaCoverageConsole{RC_CONSOLE_PCFX, "PC-FX"},
    RaCoverageConsole{RC_CONSOLE_ATARI_5200, "Atari 5200"},
    RaCoverageConsole{RC_CONSOLE_ATARI_7800, "Atari 7800"},
    RaCoverageConsole{RC_CONSOLE_X68K, "Sharp X68000"},
    RaCoverageConsole{RC_CONSOLE_CASSETTEVISION, "Cassette Vision"},
    RaCoverageConsole{RC_CONSOLE_SUPER_CASSETTEVISION, "Super Cassette Vision"},
    RaCoverageConsole{RC_CONSOLE_NEO_GEO_CD, "Neo Geo CD"},
    RaCoverageConsole{RC_CONSOLE_FAIRCHILD_CHANNEL_F, "Fairchild Channel F"},
    RaCoverageConsole{RC_CONSOLE_FM_TOWNS, "FM Towns"},
    RaCoverageConsole{RC_CONSOLE_ZX_SPECTRUM, "ZX Spectrum"},
    RaCoverageConsole{RC_CONSOLE_GAME_AND_WATCH, "Game & Watch"},
    RaCoverageConsole{RC_CONSOLE_NOKIA_NGAGE, "Nokia N-Gage"},
    RaCoverageConsole{RC_CONSOLE_NINTENDO_3DS, "Nintendo 3DS"},
    RaCoverageConsole{RC_CONSOLE_SUPERVISION, "Watara Supervision"},
    RaCoverageConsole{RC_CONSOLE_SHARPX1, "Sharp X1"},
    RaCoverageConsole{RC_CONSOLE_TIC80, "TIC-80"},
    RaCoverageConsole{RC_CONSOLE_THOMSONTO8, "Thomson TO8"},
    RaCoverageConsole{RC_CONSOLE_PC6000, "PC-6000"},
    RaCoverageConsole{RC_CONSOLE_PICO, "Sega Pico"},
    RaCoverageConsole{RC_CONSOLE_MEGADUCK, "Mega Duck"},
    RaCoverageConsole{RC_CONSOLE_ZEEBO, "Zeebo"},
    RaCoverageConsole{RC_CONSOLE_ARDUBOY, "Arduboy"},
    RaCoverageConsole{RC_CONSOLE_WASM4, "WASM-4"},
    RaCoverageConsole{RC_CONSOLE_ARCADIA_2001, "Arcadia 2001"},
    RaCoverageConsole{RC_CONSOLE_INTERTON_VC_4000, "Interton VC 4000"},
    RaCoverageConsole{RC_CONSOLE_ELEKTOR_TV_GAMES_COMPUTER, "Elektor TV Games Computer"},
    RaCoverageConsole{RC_CONSOLE_PC_ENGINE_CD, "PC Engine CD/TurboGrafx-CD"},
    RaCoverageConsole{RC_CONSOLE_ATARI_JAGUAR_CD, "Atari Jaguar CD"},
    RaCoverageConsole{RC_CONSOLE_NINTENDO_DSI, "Nintendo DSi"},
    RaCoverageConsole{RC_CONSOLE_TI83, "TI-83"},
    RaCoverageConsole{RC_CONSOLE_UZEBOX, "Uzebox"},
    RaCoverageConsole{RC_CONSOLE_FAMICOM_DISK_SYSTEM, "Famicom Disk System"},
};
} // namespace

PlatformService::PlatformService() {
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_GAMEBOY,
      .name = "Game Boy",
      .abbreviation = "Game Boy",
      .slug = "gb",
      .fileAssociations = {"gb"},
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
      .fileAssociations = {"nes"},
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
      .id = PLATFORM_ID_SNES,
      .name = "SNES/Super Famicom",
      .abbreviation = "SNES",
      .slug = "snes",
      .fileAssociations = {"sfc", "smc"},
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
      .fileAssociations = {"sms"},
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
      .fileAssociations = {"md", "gen"},
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
      .fileAssociations = {"iso", "cso", "pbp"},
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
      .fileAssociations = {"pce"},
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
      .id = PLATFORM_ID_SUPERGRAFX,
      .name = "SuperGrafx",
      .abbreviation = "SuperGrafx",
      .slug = "sgx",
      .fileAssociations = {"sgx"},
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
      .fileAssociations = {"ws", "wsc"},
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
      .fileAssociations = {"ngp", "ngc"},
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
      .fileAssociations = {"iso", "cue"},
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
  m_platforms.emplace_back(Platform{.id = PLATFORM_ID_VIRTUAL_BOY,
                                    .name = "Virtual Boy",
                                    .abbreviation = "Virtual Boy",
                                    .retroAchievementsId = RC_CONSOLE_VIRTUAL_BOY});
  m_platforms.emplace_back(Platform{.id = PLATFORM_ID_SEGA_SATURN,
                                    .name = "Sega Saturn",
                                    .abbreviation = "Saturn",
                                    .retroAchievementsId = RC_CONSOLE_SATURN});
  m_platforms.emplace_back(Platform{.id = PLATFORM_ID_SEGA_32X,
                                    .name = "Sega 32X",
                                    .abbreviation = "32X",
                                    .retroAchievementsId = RC_CONSOLE_SEGA_32X});
  m_platforms.emplace_back(Platform{.id = PLATFORM_ID_SEGA_CD,
                                    .name = "Sega CD/Mega CD",
                                    .abbreviation = "Sega CD",
                                    .retroAchievementsId = RC_CONSOLE_SEGA_CD});
  m_platforms.emplace_back(Platform{.id = PLATFORM_ID_PS2,
                                    .name = "PlayStation 2",
                                    .abbreviation = "PS2",
                                    .retroAchievementsId = RC_CONSOLE_PLAYSTATION_2});

  // RA-coverage consoles Firelight doesn't fully model: id = 1000 + rc id
  for (const auto &console : RA_COVERAGE_CONSOLES) {
    m_platforms.emplace_back(Platform{.id = static_cast<unsigned>(1000 + console.rcConsoleId),
                                      .name = console.name,
                                      .abbreviation = console.name,
                                      .retroAchievementsId = static_cast<unsigned>(console.rcConsoleId)});
  }
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

int PlatformService::platformIdForExtension(const std::string &extension) const {
  std::string lower = extension;
  std::transform(lower.begin(), lower.end(), lower.begin(), [](const unsigned char c) { return std::tolower(c); });
  for (const auto &platform : m_platforms) {
    for (const auto &association : platform.fileAssociations) {
      if (association == lower) {
        return static_cast<int>(platform.id);
      }
    }
  }
  return PLATFORM_ID_UNKNOWN;
}

int PlatformService::platformIdForRcConsole(const int rcConsoleId) const {
  // Consoles Firelight modeled with legacy ids keep them; every other
  // RA-supported console maps to its provisional id (1000 + rc id)
  switch (rcConsoleId) {
  case RC_CONSOLE_GAMEBOY:
    return PLATFORM_ID_GAMEBOY;
  case RC_CONSOLE_GAMEBOY_COLOR:
    return PLATFORM_ID_GAMEBOY_COLOR;
  case RC_CONSOLE_GAMEBOY_ADVANCE:
    return PLATFORM_ID_GAMEBOY_ADVANCE;
  case RC_CONSOLE_VIRTUAL_BOY:
    return PLATFORM_ID_VIRTUAL_BOY;
  case RC_CONSOLE_NINTENDO:
    return PLATFORM_ID_NES;
  case RC_CONSOLE_SUPER_NINTENDO:
    return PLATFORM_ID_SNES;
  case RC_CONSOLE_NINTENDO_64:
    return PLATFORM_ID_N64;
  case RC_CONSOLE_NINTENDO_DS:
    return PLATFORM_ID_NINTENDO_DS;
  case RC_CONSOLE_MASTER_SYSTEM:
    return PLATFORM_ID_SEGA_MASTER_SYSTEM;
  case RC_CONSOLE_MEGA_DRIVE:
    return PLATFORM_ID_SEGA_GENESIS;
  case RC_CONSOLE_GAME_GEAR:
    return PLATFORM_ID_SEGA_GAMEGEAR;
  case RC_CONSOLE_SATURN:
    return PLATFORM_ID_SEGA_SATURN;
  case RC_CONSOLE_SEGA_32X:
    return PLATFORM_ID_SEGA_32X;
  case RC_CONSOLE_SEGA_CD:
    return PLATFORM_ID_SEGA_CD;
  case RC_CONSOLE_PLAYSTATION:
    return PLATFORM_ID_PS1;
  case RC_CONSOLE_PLAYSTATION_2:
    return PLATFORM_ID_PS2;
  case RC_CONSOLE_PSP:
    return PLATFORM_ID_PLAYSTATION_PORTABLE;
  case RC_CONSOLE_PC_ENGINE:
    return PLATFORM_ID_TURBOGRAFX16;
  case RC_CONSOLE_POKEMON_MINI:
    return PLATFORM_ID_POKEMON_MINI;
  case RC_CONSOLE_WONDERSWAN:
    return PLATFORM_ID_WONDERSWAN;
  case RC_CONSOLE_SG1000:
    return PLATFORM_ID_SG1000;
  case RC_CONSOLE_NEOGEO_POCKET:
    return PLATFORM_ID_NEOGEO_POCKET;
  case RC_CONSOLE_UNKNOWN:
    return PLATFORM_ID_UNKNOWN;
  default:
    return 1000 + rcConsoleId;
  }
}
} // namespace firelight::platforms
