#include "platform_service.hpp"
#include "models/controller_input_descriptor.hpp"
#include "models/platform.hpp"

namespace firelight::platforms {

PlatformService::PlatformService() {
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_GAMEBOY,
      .name = "Game Boy",
      .abbreviation = "Game Boy",
      .slug = "gb",
      .fileAssociations = {"gb"},
      .controllerTypes = {{.id = 1,
                           .name = "Game Boy",
                           .imageUrl = "qrc:/images/controllers/gb",
                           .inputs = {
                               {"A", input::GamepadInput::EastFace},
                               {"B", input::GamepadInput::SouthFace},
                               {"Start", input::GamepadInput::Start},
                               {"Select", input::GamepadInput::Select},
                               {"D-Pad Up", input::GamepadInput::DpadUp},
                               {"D-Pad Down", input::GamepadInput::DpadDown},
                               {"D-Pad Left", input::GamepadInput::DpadLeft},
                               {"D-Pad Right", input::GamepadInput::DpadRight},
                           }}}});
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_GAMEBOY_COLOR,
      .name = "Game Boy Color",
      .abbreviation = "Game Boy Color",
      .slug = "gbc",
      .fileAssociations = {"gbc"},
      .controllerTypes = {{.id = 1,
                           .name = "Game Boy Color",
                           .imageUrl = "qrc:/images/controllers/gbc",
                           .inputs = {
                               {"A", input::GamepadInput::EastFace},
                               {"B", input::GamepadInput::SouthFace},
                               {"Start", input::GamepadInput::Start},
                               {"Select", input::GamepadInput::Select},
                               {"D-Pad Up", input::GamepadInput::DpadUp},
                               {"D-Pad Down", input::GamepadInput::DpadDown},
                               {"D-Pad Left", input::GamepadInput::DpadLeft},
                               {"D-Pad Right", input::GamepadInput::DpadRight},
                           }}}});
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_GAMEBOY_ADVANCE,
      .name = "Game Boy Advance",
      .abbreviation = "Game Boy Advance",
      .slug = "gba",
      .fileAssociations = {"gba"},
      .controllerTypes = {{.id = 1,
                           .name = "Game Boy Advance",
                           .imageUrl = "qrc:/images/controllers/gba",
                           .inputs = {
                               {"A", input::GamepadInput::EastFace},
                               {"B", input::GamepadInput::SouthFace},
                               {"L", input::GamepadInput::LeftBumper},
                               {"R", input::GamepadInput::RightBumper},
                               {"Start", input::GamepadInput::Start},
                               {"Select", input::GamepadInput::Select},
                               {"D-Pad Up", input::GamepadInput::DpadUp},
                               {"D-Pad Down", input::GamepadInput::DpadDown},
                               {"D-Pad Left", input::GamepadInput::DpadLeft},
                               {"D-Pad Right", input::GamepadInput::DpadRight},
                           }}}});
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_NES,
      .name = "NES/Famicom",
      .abbreviation = "NES",
      .slug = "nes",
      .fileAssociations = {"nes"},
      .controllerTypes = {{.id = 1,
                           .name = "NES Controller",
                           .imageUrl = "qrc:/images/controllers/nes",
                           .inputs = {
                               {"A", input::GamepadInput::EastFace},
                               {"B", input::GamepadInput::SouthFace},
                               {"Start", input::GamepadInput::Start},
                               {"Select", input::GamepadInput::Select},
                               {"D-Pad Up", input::GamepadInput::DpadUp},
                               {"D-Pad Down", input::GamepadInput::DpadDown},
                               {"D-Pad Left", input::GamepadInput::DpadLeft},
                               {"D-Pad Right", input::GamepadInput::DpadRight},
                           }}}});
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_SNES,
      .name = "SNES/Super Famicom",
      .abbreviation = "SNES",
      .slug = "snes",
      .fileAssociations = {"sfc", "smc"},
      .controllerTypes = {{.id = 1,
                           .name = "SNES Controller",
                           .imageUrl = "qrc:/images/controllers/snes",
                           .inputs = {
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
                           }}}});
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_N64,
      .name = "Nintendo 64",
      .abbreviation = "N64",
      .slug = "n64",
      .fileAssociations = {"n64", "z64", "v64"},
      .controllerTypes = {
          {.id = 1,
           .name = "N64 Controller",
           .imageUrl = "qrc:/images/controllers/n64",
           .inputs = {
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
           }}}});
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_NINTENDO_DS,
      .name = "Nintendo DS",
      .abbreviation = "NDS",
      .slug = "nds",
      .fileAssociations = {"nds"},
      .controllerTypes = {
          {.id = 1,
           .name = "Nintendo DS",
           .imageUrl = "qrc:/images/controllers/nds",
           .inputs = {
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
           }}}});

  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_SEGA_MASTER_SYSTEM,
      .name = "Master System",
      .abbreviation = "Master System",
      .slug = "sms",
      .fileAssociations = {"sms"},
      .controllerTypes = {{.id = 1,
                           .name = "Master System Controller",
                           .imageUrl = "qrc:/images/controllers/sms",
                           .inputs = {
                               {"1", input::GamepadInput::SouthFace},
                               {"2", input::GamepadInput::EastFace},
                               {"Start", input::GamepadInput::Start},
                               {"D-Pad Up", input::GamepadInput::DpadUp},
                               {"D-Pad Down", input::GamepadInput::DpadDown},
                               {"D-Pad Left", input::GamepadInput::DpadLeft},
                               {"D-Pad Right", input::GamepadInput::DpadRight},
                           }}}});
  m_platforms.emplace_back(
      Platform{.id = PLATFORM_ID_SEGA_GENESIS,
               .name = "Genesis/Mega Drive",
               .abbreviation = "Genesis/MD",
               .slug = "gen",
               .fileAssociations = {"md", "gen"},
               .controllerTypes = {
                   {.id = 1,
                    .name = "3-button Control Pad",
                    .imageUrl = "qrc:/images/controllers/gen-threebutton",
                    .inputs =
                        {
                            {"A", input::GamepadInput::WestFace},
                            {"B", input::GamepadInput::SouthFace},
                            {"C", input::GamepadInput::EastFace},
                            {"Start", input::GamepadInput::Start},
                            {"Mode", input::GamepadInput::Select},
                            {"D-Pad Up", input::GamepadInput::DpadUp},
                            {"D-Pad Down", input::GamepadInput::DpadDown},
                            {"D-Pad Left", input::GamepadInput::DpadLeft},
                            {"D-Pad Right", input::GamepadInput::DpadRight},
                        }},
                   {.id = 2,
                    .name = "6-button Control Pad",
                    .imageUrl = "qrc:/images/controllers/gen-sixbutton",
                    .inputs = {
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
                    }}}});
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_SEGA_GAMEGEAR,
      .name = "Game Gear",
      .abbreviation = "Game Gear",
      .slug = "gg",
      .fileAssociations = {"gg"},
      .controllerTypes = {{.id = 1,
                           .name = "Game Gear",
                           .imageUrl = "qrc:/images/controllers/gg",
                           .inputs = {
                               {"1", input::GamepadInput::SouthFace},
                               {"2", input::GamepadInput::EastFace},
                               {"Start", input::GamepadInput::Start},
                               {"D-Pad Up", input::GamepadInput::DpadUp},
                               {"D-Pad Down", input::GamepadInput::DpadDown},
                               {"D-Pad Left", input::GamepadInput::DpadLeft},
                               {"D-Pad Right", input::GamepadInput::DpadRight},
                           }}}});
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_TURBOGRAFX16,
      .name = "PC Engine/TurboGrafx-16",
      .abbreviation = "PCE/TG-16",
      .slug = "pce",
      .fileAssociations = {"pce"},
      .controllerTypes = {
          {.id = 1,
           .name = "2-button joypad",
           .imageUrl = "qrc:/images/controllers/tgx-twobutton",
           .inputs =
               {
                   {"I", input::GamepadInput::EastFace},
                   {"II", input::GamepadInput::SouthFace},
                   {"Switch to 6-button mode",
                    input::GamepadInput::LeftTrigger},
                   {"Run", input::GamepadInput::Start},
                   {"Select", input::GamepadInput::Select},
                   {"D-Pad Up", input::GamepadInput::DpadUp},
                   {"D-Pad Down", input::GamepadInput::DpadDown},
                   {"D-Pad Left", input::GamepadInput::DpadLeft},
                   {"D-Pad Right", input::GamepadInput::DpadRight},
               }},
          {.id = 2,
           .name = "6-button joypad",
           .imageUrl = "qrc:/images/controllers/tgx-sixbutton",
           .inputs = {
               {"I", input::GamepadInput::EastFace},
               {"II", input::GamepadInput::SouthFace},
               {"III", input::GamepadInput::WestFace},
               {"IV", input::GamepadInput::NorthFace},
               {"V", input::GamepadInput::LeftBumper},
               {"VI", input::GamepadInput::RightBumper},
               {"Switch to 2-button mode", input::GamepadInput::LeftTrigger},
               {"Run", input::GamepadInput::Start},
               {"Select", input::GamepadInput::Select},
               {"D-Pad Up", input::GamepadInput::DpadUp},
               {"D-Pad Down", input::GamepadInput::DpadDown},
               {"D-Pad Left", input::GamepadInput::DpadLeft},
               {"D-Pad Right", input::GamepadInput::DpadRight},
           }}}});
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_SUPERGRAFX,
      .name = "SuperGrafx",
      .abbreviation = "SuperGrafx",
      .slug = "sgx",
      .fileAssociations = {"sgx"},
      .controllerTypes = {
          {.id = 1,
           .name = "2-button joypad",
           .imageUrl = "qrc:/images/controllers/tgx-twobutton",
           .inputs =
               {
                   {"I", input::GamepadInput::EastFace},
                   {"II", input::GamepadInput::SouthFace},
                   {"Switch to 6-button mode",
                    input::GamepadInput::LeftTrigger},
                   {"Run", input::GamepadInput::Start},
                   {"Select", input::GamepadInput::Select},
                   {"D-Pad Up", input::GamepadInput::DpadUp},
                   {"D-Pad Down", input::GamepadInput::DpadDown},
                   {"D-Pad Left", input::GamepadInput::DpadLeft},
                   {"D-Pad Right", input::GamepadInput::DpadRight},
               }},
          {.id = 2,
           .name = "6-button joypad",
           .imageUrl = "qrc:/images/controllers/tgx-sixbutton",
           .inputs = {
               {"I", input::GamepadInput::EastFace},
               {"II", input::GamepadInput::SouthFace},
               {"III", input::GamepadInput::WestFace},
               {"IV", input::GamepadInput::NorthFace},
               {"V", input::GamepadInput::LeftBumper},
               {"VI", input::GamepadInput::RightBumper},
               {"Switch to 2-button mode", input::GamepadInput::LeftTrigger},
               {"Run", input::GamepadInput::Start},
               {"Select", input::GamepadInput::Select},
               {"D-Pad Up", input::GamepadInput::DpadUp},
               {"D-Pad Down", input::GamepadInput::DpadDown},
               {"D-Pad Left", input::GamepadInput::DpadLeft},
               {"D-Pad Right", input::GamepadInput::DpadRight},
           }}}});
  // m_platforms.emplace_back(
  //     Platform{.id = PLATFORM_ID_POKEMON_MINI,
  //              .name = "Pokémon Mini",
  //              .abbreviation = "Pokémon Mini",
  //              .slug = "pkmn",
  //              .fileAssociations = {"min"},
  //              .controllerTypes = {
  //                  {.id = 1,
  //                   .name = "Pokémon Mini",
  //                   .inputs = {
  //                       {"A", input::GamepadInput::EastFace},
  //                       {"B", input::GamepadInput::SouthFace},
  //                       {"C", input::GamepadInput::RightBumper},
  //                       {"D-Pad Up", input::GamepadInput::DpadUp},
  //                       {"D-Pad Down", input::GamepadInput::DpadDown},
  //                       {"D-Pad Left", input::GamepadInput::DpadLeft},
  //                       {"D-Pad Right",
  //                       input::GamepadInput::DpadRight},
  //                       {"Shake", input::GamepadInput::LeftBumper},
  //                       {"Power", input::GamepadInput::Select},
  //                   }}}});
  m_platforms.emplace_back(
      Platform{.id = PLATFORM_ID_WONDERSWAN,
               .name = "WonderSwan",
               .abbreviation = "WonderSwan",
               .slug = "ws",
               .fileAssociations = {"ws", "wsc"},
               .controllerTypes = {
                   {.id = 1,
                    .name = "Horizontal",
                    .imageUrl = "qrc:/images/controllers/ws-horizontal",
                    .inputs =
                        {
                            {"X1", input::GamepadInput::DpadUp},
                            {"X2", input::GamepadInput::DpadRight},
                            {"X3", input::GamepadInput::DpadDown},
                            {"X4", input::GamepadInput::DpadLeft},
                            {"A", input::GamepadInput::EastFace},
                            {"B", input::GamepadInput::SouthFace},
                            {"Start", input::GamepadInput::Start},
                            {"Switch to vertical", input::GamepadInput::Select},
                        }},
                   {.id = 2,
                    .name = "Vertical",
                    .imageUrl = "qrc:/images/controllers/ws-vertical",
                    .inputs = {
                        {"Y1", input::GamepadInput::DpadLeft},
                        {"Y2", input::GamepadInput::DpadUp},
                        {"Y3", input::GamepadInput::DpadRight},
                        {"Y4", input::GamepadInput::DpadDown},
                        {"X1", input::GamepadInput::WestFace},
                        {"X2", input::GamepadInput::NorthFace},
                        {"X3", input::GamepadInput::EastFace},
                        {"X4", input::GamepadInput::SouthFace},
                        {"Start", input::GamepadInput::Start},
                        {"Switch to horizontal", input::GamepadInput::Select},
                    }}}});
}
std::optional<Platform> PlatformService::getPlatform(const unsigned id) const {
  for (const auto &platform : m_platforms) {
    if (platform.id == id) {
      return platform;
    }
  }

  return {};
}
std::vector<Platform> PlatformService::listPlatforms() const {
  return m_platforms;
}

} // namespace firelight::platforms
