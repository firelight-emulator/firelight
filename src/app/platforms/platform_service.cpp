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
      .controllerTypes =
          {{.id = 1,
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
      .emulationSettings =
          {{.label = "Simulate LCD ghosting effects",
            .category = "Game Boy",
            .key = "gambatte_mix_frames",
            .description =
                "Simple: 50/50 mix of current and previous frames\nLCD "
                "Ghosting: Mimics natural LCD response times by combining "
                "multiple buffered frames",
            .defaultValue = "disabled",
            .type = settings::OPTIONS,
            .options = {{.label = "None", .value = "disabled"},
                        {.label = "Simple (Accurate)", .value = "mix"},
                        {.label = "Simple (Fast)", .value = "mix_fast"},
                        {.label = "LCD Ghosting (Accurate)",
                         .value = "lcd_ghosting"},
                        {.label = "LCD Ghosting (Fast)",
                         .value = "lcd_ghosting_fast"}}},
           {.label = "Allow opposing D-Pad directions",
            .category = "Game Boy",
            .key = "gambatte_up_down_allowed",
            .description = "Allows pressing up and down or left and right at "
                           "the same time. Can cause glitches in some games.",
            .defaultValue = "disabled",
            .type = settings::BOOLEAN,
            .trueStringValue = "enabled",
            .falseStringValue = "disabled"}},
  });
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_GAMEBOY_COLOR,
      .name = "Game Boy Color",
      .abbreviation = "Game Boy Color",
      .slug = "gbc",
      .fileAssociations = {"gbc"},
      .controllerTypes =
          {{.id = 1,
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
      .emulationSettings = {
          {.label = "Simulate LCD ghosting effects",
           .category = "Game Boy Color",
           .key = "gambatte_mix_frames",
           .description =
               "Simple: 50/50 mix of current and previous frames\nLCD "
               "Ghosting: Mimics natural LCD response times by combining "
               "multiple buffered frames",
           .defaultValue = "disabled",
           .type = settings::OPTIONS,
           .options = {{.label = "None", .value = "disabled"},
                       {.label = "Simple (Accurate)", .value = "mix"},
                       {.label = "Simple (Fast)", .value = "mix_fast"},
                       {.label = "LCD Ghosting (Accurate)",
                        .value = "lcd_ghosting"},
                       {.label = "LCD Ghosting (Fast)",
                        .value = "lcd_ghosting_fast"}}},
          {.label = "Allow opposing D-Pad directions",
           .category = "Game Boy Color",
           .key = "gambatte_up_down_allowed",
           .description = "Allows pressing up and down or left and right at "
                          "the same time. Can cause glitches in some games.",
           .defaultValue = "disabled",
           .type = settings::BOOLEAN,
           .trueStringValue = "enabled",
           .falseStringValue = "disabled"},
          {
              .label = "Color correction",
              .category = "Color correction",
              .key = "gambatte_gbc_color_correction",
              .defaultValue = "GBC only",
              .description = "Adjusts output colors to match the display of "
                             "real Game Boy Color hardware.",
              .type = settings::BOOLEAN,
              .trueStringValue = "GBC only",
              .falseStringValue = "disabled",
          },
          {.label = "Color correction mode",
           .category = "Color correction",
           .key = "gambatte_gbc_color_correction_mode",
           .defaultValue = "accurate",
           .description =
               "You can change this from Accurate to Fast if you are "
               "experiencing performance issues.\n\n"
               "Accurate: Produces colors identical to original hardware\n"
               "Fast: Darkens colors and reduces saturation",
           .type = settings::OPTIONS,
           .options = {{.label = "Accurate", .value = "accurate"},
                       {.label = "Fast", .value = "fast"}}},
          {.label = "Frontlight position",
           .category = "Color correction",
           .key = "gambatte_gbc_frontlight_position",
           .defaultValue = "central",
           .description =
               "Simulates the physical response of the Game Boy Color LCD "
               "panel when illuminated from different angles.\n\n"
               "Central: Standard color reproduction\n"
               "Above screen: Increases brightness"
               "Below screen: Decreases brightness",
           .type = settings::OPTIONS,
           .options = {{.label = "Central", .value = "central"},
                       {.label = "Above screen", .value = "above screen"},
                       {.label = "Below screen", .value = "below screen"}}}}});
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_GAMEBOY_ADVANCE,
      .name = "Game Boy Advance",
      .abbreviation = "Game Boy Advance",
      .slug = "gba",
      .fileAssociations = {"gba"},
      .controllerTypes =
          {{.id = 1,
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
      .emulationSettings = {}});
  m_platforms.emplace_back(
      Platform{.id = PLATFORM_ID_NES,
               .name = "NES/Famicom",
               .abbreviation = "NES",
               .slug = "nes",
               .fileAssociations = {"nes"},
               .controllerTypes =
                   {{.id = 1,
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
               .emulationSettings = {}});
  m_platforms.emplace_back(
      Platform{.id = PLATFORM_ID_SNES,
               .name = "SNES/Super Famicom",
               .abbreviation = "SNES",
               .slug = "snes",
               .fileAssociations = {"sfc", "smc"},
               .controllerTypes =
                   {{.id = 1,
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
                         }}},
               .emulationSettings = {}});
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_N64,
      .name = "Nintendo 64",
      .abbreviation = "N64",
      .slug = "n64",
      .fileAssociations = {"n64", "z64", "v64"},
      .controllerTypes =
          {{.id = 1,
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
      .emulationSettings = {}});
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_NINTENDO_DS,
      .name = "Nintendo DS",
      .abbreviation = "NDS",
      .slug = "nds",
      .fileAssociations = {"nds"},
      .controllerTypes =
          {{.id = 1,
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
                    {"Enable microphone (hold)",
                     input::GamepadInput::LeftTrigger},
                    {"Change screen layout", input::GamepadInput::RightTrigger},
                    {"Close lid", input::GamepadInput::L3},
                }}},
      .emulationSettings = {}});

  m_platforms.emplace_back(
      Platform{.id = PLATFORM_ID_SG1000,
               .name = "SG-1000",
               .abbreviation = "SG-1000",
               .slug = "sg",
               .fileAssociations = {"sg"},
               .controllerTypes =
                   {{.id = 1,
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
               .emulationSettings = {}});

  m_platforms.emplace_back(
      Platform{.id = PLATFORM_ID_SEGA_MASTER_SYSTEM,
               .name = "Master System",
               .abbreviation = "Master System",
               .slug = "sms",
               .fileAssociations = {"sms"},
               .controllerTypes =
                   {{.id = 1,
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
                         }}},
               .emulationSettings = {}});
  m_platforms.emplace_back(
      Platform{.id = PLATFORM_ID_SEGA_GENESIS,
               .name = "Genesis/Mega Drive",
               .abbreviation = "Genesis/MD",
               .slug = "gen",
               .fileAssociations = {"md", "gen"},
               .controllerTypes =
                   {{.id = 1,
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
               .emulationSettings = {}});
  m_platforms.emplace_back(
      Platform{.id = PLATFORM_ID_SEGA_GAMEGEAR,
               .name = "Game Gear",
               .abbreviation = "Game Gear",
               .slug = "gg",
               .fileAssociations = {"gg"},
               .controllerTypes =
                   {{.id = 1,
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
               .emulationSettings = {}});
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_TURBOGRAFX16,
      .name = "PC Engine/TurboGrafx-16",
      .abbreviation = "PCE/TG-16",
      .slug = "pce",
      .fileAssociations = {"pce"},
      .controllerTypes =
          {{.id = 1,
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
      .emulationSettings = {}});
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_SUPERGRAFX,
      .name = "SuperGrafx",
      .abbreviation = "SuperGrafx",
      .slug = "sgx",
      .fileAssociations = {"sgx"},
      .controllerTypes =
          {{.id = 1,
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
      .emulationSettings = {}});
  m_platforms.emplace_back(
      Platform{.id = PLATFORM_ID_POKEMON_MINI,
               .name = "Pokémon Mini",
               .abbreviation = "Pokémon Mini",
               .slug = "pkmn",
               .fileAssociations = {"min"},
               .controllerTypes =
                   {{.id = 1,
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
               .emulationSettings = {}});
  m_platforms.emplace_back(Platform{
      .id = PLATFORM_ID_WONDERSWAN,
      .name = "WonderSwan",
      .abbreviation = "WonderSwan",
      .slug = "ws",
      .fileAssociations = {"ws", "wsc"},
      .controllerTypes =
          {{.id = 1,
            .name = "Retropad",
            .imageUrl = "qrc:/images/controllers/ws-horizontal",
            .inputs =
                {
                    {"X1 (Horizontal), Y2 (Vertical)",
                     input::GamepadInput::DpadUp},
                    {"X2 (Horizontal), Y3 (Vertical)",
                     input::GamepadInput::DpadRight},
                    {"X3 (Horizontal), Y4 (Vertical)",
                     input::GamepadInput::DpadDown},
                    {"X4 (Horizontal), Y1 (Vertical)",
                     input::GamepadInput::DpadLeft},
                    {"A (Horizontal), X3 (Vertical)",
                     input::GamepadInput::EastFace},
                    {"B (Horizontal), X4 (Vertical)",
                     input::GamepadInput::SouthFace},
                    {"X1 (Horizontal)", input::GamepadInput::WestFace},
                    {"X2 (Horizontal)", input::GamepadInput::NorthFace},
                    {"Start", input::GamepadInput::Start},
                    {"Switch orientation", input::GamepadInput::Select},
                }}},
      .emulationSettings = {}});
  m_platforms.emplace_back(
      Platform{.id = PLATFORM_ID_NEOGEO_POCKET,
               .name = "NeoGeo Pocket",
               .abbreviation = "NeoGeo Pocket",
               .slug = "ngp",
               .fileAssociations = {"ngp", "ngc"},
               .controllerTypes =
                   {{.id = 1,
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
               .emulationSettings = {}});
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
