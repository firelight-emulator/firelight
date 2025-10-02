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
      .emulationSettings = {
          {.label = "Screen gap",
           .category = "Nintendo DS",
           .key = "melonds_screen_gap",
           .description = "Sets the number of pixels between the two screens.",
           .defaultValue = "0",
           .type = settings::OPTIONS,
           .options = {{.label = "0px", .value = "0"},
                       {.label = "1px", .value = "1"},
                       {.label = "2px", .value = "2"},
                       {.label = "3px", .value = "3"},
                       {.label = "4px", .value = "4"},
                       {.label = "5px", .value = "5"},
                       {.label = "6px", .value = "6"},
                       {.label = "7px", .value = "7"},
                       {.label = "8px", .value = "8"},
                       {.label = "9px", .value = "9"},
                       {.label = "10px", .value = "10"},
                       {.label = "11px", .value = "11"},
                       {.label = "12px", .value = "12"},
                       {.label = "13px", .value = "13"},
                       {.label = "14px", .value = "14"},
                       {.label = "15px", .value = "15"},
                       {.label = "16px", .value = "16"},
                       {.label = "17px", .value = "17"},
                       {.label = "18px", .value = "18"},
                       {.label = "19px", .value = "19"},
                       {.label = "20px", .value = "20"},
                       {.label = "21px", .value = "21"},
                       {.label = "22px", .value = "22"},
                       {.label = "23px", .value = "23"},
                       {.label = "24px", .value = "24"},
                       {.label = "25px", .value = "25"},
                       {.label = "26px", .value = "26"},
                       {.label = "27px", .value = "27"},
                       {.label = "28px", .value = "28"},
                       {.label = "29px", .value = "29"},
                       {.label = "30px", .value = "30"},
                       {.label = "31px", .value = "31"},
                       {.label = "32px", .value = "32"},
                       {.label = "33px", .value = "33"},
                       {.label = "34px", .value = "34"},
                       {.label = "35px", .value = "35"},
                       {.label = "36px", .value = "36"},
                       {.label = "37px", .value = "37"},
                       {.label = "38px", .value = "38"},
                       {.label = "39px", .value = "39"},
                       {.label = "40px", .value = "40"},
                       {.label = "41px", .value = "41"},
                       {.label = "42px", .value = "42"},
                       {.label = "43px", .value = "43"},
                       {.label = "44px", .value = "44"},
                       {.label = "45px", .value = "45"},
                       {.label = "46px", .value = "46"},
                       {.label = "47px", .value = "47"},
                       {.label = "48px", .value = "48"},
                       {.label = "49px", .value = "49"},
                       {.label = "50px", .value = "50"},
                       {.label = "51px", .value = "51"},
                       {.label = "52px", .value = "52"},
                       {.label = "53px", .value = "53"},
                       {.label = "54px", .value = "54"},
                       {.label = "55px", .value = "55"},
                       {.label = "56px", .value = "56"},
                       {.label = "57px", .value = "57"},
                       {.label = "58px", .value = "58"},
                       {.label = "59px", .value = "59"},
                       {.label = "60px", .value = "60"},
                       {.label = "61px", .value = "61"},
                       {.label = "62px", .value = "62"},
                       {.label = "63px", .value = "63"},
                       {.label = "64px", .value = "64"},
                       {.label = "65px", .value = "65"},
                       {.label = "66px", .value = "66"},
                       {.label = "67px", .value = "67"},
                       {.label = "68px", .value = "68"},
                       {.label = "69px", .value = "69"},
                       {.label = "70px", .value = "70"},
                       {.label = "71px", .value = "71"},
                       {.label = "72px", .value = "72"},
                       {.label = "73px", .value = "73"},
                       {.label = "74px", .value = "74"},
                       {.label = "75px", .value = "75"},
                       {.label = "76px", .value = "76"},
                       {.label = "77px", .value = "77"},
                       {.label = "78px", .value = "78"},
                       {.label = "79px", .value = "79"},
                       {.label = "80px", .value = "80"},
                       {.label = "81px", .value = "81"},
                       {.label = "82px", .value = "82"},
                       {.label = "83px", .value = "83"},
                       {.label = "84px", .value = "84"},
                       {.label = "85px", .value = "85"},
                       {.label = "86px", .value = "86"},
                       {.label = "87px", .value = "87"},
                       {.label = "88px", .value = "88"},
                       {.label = "89px", .value = "89"},
                       {.label = "90px", .value = "90"},
                       {.label = "91px", .value = "91"},
                       {.label = "92px", .value = "92"},
                       {.label = "93px", .value = "93"},
                       {.label = "94px", .value = "94"},
                       {.label = "95px", .value = "95"},
                       {.label = "96px", .value = "96"},
                       {.label = "97px", .value = "97"},
                       {.label = "98px", .value = "98"},
                       {.label = "99px", .value = "99"}}}}});

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
