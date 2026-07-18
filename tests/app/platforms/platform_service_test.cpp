#include <firelight/input/gamepad_input.hpp>
#include <firelight/platforms/platform_service.hpp>
#include <gtest/gtest.h>
#include <rcheevos/rc_consoles.h>

class PlatformServiceTest : public testing::Test {
protected:
  static void
  assertPlatformsEqual(const firelight::platforms::Platform &first,
                       const firelight::platforms::Platform &second) {
    EXPECT_EQ(first.id, second.id);
    EXPECT_EQ(first.name, second.name);
    EXPECT_EQ(first.abbreviation, second.abbreviation);
    EXPECT_EQ(first.slug, second.slug);
    EXPECT_EQ(first.fileAssociations, second.fileAssociations);
    EXPECT_EQ(first.controllerTypes.size(), second.controllerTypes.size());

    for (size_t i = 0; i < first.controllerTypes.size(); ++i) {
      const auto &expectedController = second.controllerTypes[i];
      const auto &actualController = first.controllerTypes[i];

      EXPECT_EQ(actualController.id, expectedController.id);
      EXPECT_EQ(actualController.name, expectedController.name);
      EXPECT_EQ(actualController.imageUrl, expectedController.imageUrl);
      EXPECT_EQ(actualController.inputs.size(),
                expectedController.inputs.size());

      for (size_t j = 0; j < actualController.inputs.size(); ++j) {
        const auto &expectedInput = expectedController.inputs[j];
        const auto &actualInput = actualController.inputs[j];

        EXPECT_EQ(actualInput.name, expectedInput.name);
        EXPECT_EQ(actualInput.virtualInput, expectedInput.virtualInput);
      }
    }
  }
};

TEST_F(PlatformServiceTest, AllPlatformsPresent) {
  using PS = firelight::platforms::PlatformService;
  const PS service;
  // Fully-modeled platforms (with controller data) plus the identity-only
  // RetroAchievements-coverage consoles are all registered
  EXPECT_GT(service.listPlatforms().size(), 60u);
  for (const int id : {PS::PLATFORM_ID_GAMEBOY, PS::PLATFORM_ID_N64,
                       PS::PLATFORM_ID_SNES, PS::PLATFORM_ID_PS1,
                       PS::PLATFORM_ID_VIRTUAL_BOY}) {
    EXPECT_TRUE(service.getPlatform(id).has_value())
        << "missing platform id " << id;
  }
}

TEST_F(PlatformServiceTest, MouseAndLightGunDisplayEntriesArePresent) {
  using firelight::input::GamepadInputClass;
  const firelight::platforms::PlatformService service;

  int mouse = 0, lightgun = 0, joypad = 0;
  for (const auto &platform : service.listPlatforms()) {
    for (const auto &type : platform.controllerTypes) {
      switch (type.deviceClass) {
      case GamepadInputClass::Mouse:
        ++mouse;
        break;
      case GamepadInputClass::Lightgun:
        ++lightgun;
        break;
      case GamepadInputClass::Joypad:
        ++joypad;
        break;
      }
    }
  }
  // Every platform still has its standard controller, plus the WS2 mouse/light
  // gun display entries on the platforms that support them
  EXPECT_GE(joypad, 17);
  EXPECT_GT(mouse, 0);
  EXPECT_GT(lightgun, 0);
}

TEST_F(PlatformServiceTest, PlatformIdForExtensionMapsKnownExtensions) {
  using PS = firelight::platforms::PlatformService;
  const PS service;

  EXPECT_EQ(service.platformIdForExtension("gb"), PS::PLATFORM_ID_GAMEBOY);
  EXPECT_EQ(service.platformIdForExtension("sfc"), PS::PLATFORM_ID_SNES);
  EXPECT_EQ(service.platformIdForExtension("smc"), PS::PLATFORM_ID_SNES);
  EXPECT_EQ(service.platformIdForExtension("n64"), PS::PLATFORM_ID_N64);
  EXPECT_EQ(service.platformIdForExtension("z64"), PS::PLATFORM_ID_N64);

  // Lookup is case-insensitive
  EXPECT_EQ(service.platformIdForExtension("GB"), PS::PLATFORM_ID_GAMEBOY);
  EXPECT_EQ(service.platformIdForExtension("N64"), PS::PLATFORM_ID_N64);

  // Unknown extensions fall through to PLATFORM_ID_UNKNOWN
  EXPECT_EQ(service.platformIdForExtension("xyz"), PS::PLATFORM_ID_UNKNOWN);
  EXPECT_EQ(service.platformIdForExtension(""), PS::PLATFORM_ID_UNKNOWN);
}

TEST_F(PlatformServiceTest, PlatformIdForRcConsoleMapsConsoles) {
  using PS = firelight::platforms::PlatformService;
  const PS service;

  // Consoles modeled with legacy platform ids map to them directly
  EXPECT_EQ(service.platformIdForRcConsole(RC_CONSOLE_NINTENDO),
            PS::PLATFORM_ID_NES);
  EXPECT_EQ(service.platformIdForRcConsole(RC_CONSOLE_SUPER_NINTENDO),
            PS::PLATFORM_ID_SNES);
  EXPECT_EQ(service.platformIdForRcConsole(RC_CONSOLE_NINTENDO_64),
            PS::PLATFORM_ID_N64);
  EXPECT_EQ(service.platformIdForRcConsole(RC_CONSOLE_SATURN),
            PS::PLATFORM_ID_SEGA_SATURN);
  EXPECT_EQ(service.platformIdForRcConsole(RC_CONSOLE_PLAYSTATION_2),
            PS::PLATFORM_ID_PS2);

  // Unknown maps to PLATFORM_ID_UNKNOWN
  EXPECT_EQ(service.platformIdForRcConsole(RC_CONSOLE_UNKNOWN),
            PS::PLATFORM_ID_UNKNOWN);

  // Consoles Firelight doesn't fully model map to the provisional id
  // (1000 + rc console id) that identifies them by name only
  EXPECT_EQ(service.platformIdForRcConsole(RC_CONSOLE_ATARI_LYNX),
            1000 + RC_CONSOLE_ATARI_LYNX);
  EXPECT_TRUE(
      service.getPlatform(1000 + RC_CONSOLE_ATARI_LYNX).has_value());
}

// RA-coverage consoles Firelight doesn't fully model still resolve to an
// identity-only platform whose name renders in the library
TEST_F(PlatformServiceTest, RaCoverageConsoleHasIdentityPlatform) {
  const firelight::platforms::PlatformService service;

  const auto lynx = service.getPlatform(1000 + RC_CONSOLE_ATARI_LYNX);
  ASSERT_TRUE(lynx.has_value());
  EXPECT_EQ(lynx->name, "Atari Lynx");
  EXPECT_EQ(lynx->retroAchievementsId,
            static_cast<unsigned>(RC_CONSOLE_ATARI_LYNX));
  // Identity-only: no controller/extension data
  EXPECT_TRUE(lynx->controllerTypes.empty());
  EXPECT_TRUE(lynx->fileAssociations.empty());
}

// The Discord large-image key defaults to the slug, except where a platform sets
// an explicit override (Nintendo DS -> "ds")
TEST_F(PlatformServiceTest, DiscordImageFallsBackToSlug) {
  using PS = firelight::platforms::PlatformService;
  const PS service;

  const auto gb = service.getPlatform(PS::PLATFORM_ID_GAMEBOY);
  ASSERT_TRUE(gb.has_value());
  EXPECT_TRUE(gb->discordImage.empty());
  EXPECT_EQ(gb->discordImageOrSlug(), gb->slug); // "gb"

  const auto nds = service.getPlatform(PS::PLATFORM_ID_NINTENDO_DS);
  ASSERT_TRUE(nds.has_value());
  EXPECT_EQ(nds->discordImage, "ds");
  EXPECT_EQ(nds->discordImageOrSlug(), "ds");
}

// Platform survives a JSON round-trip on its serialized fields (the model keeps
// to_json/from_json so platform data could later be externalized to a file)
TEST_F(PlatformServiceTest, PlatformJsonRoundTrip) {
  firelight::platforms::Platform original;
  original.id = 42;
  original.name = "Test Console";
  original.abbreviation = "TC";
  original.slug = "tc";
  original.retroAchievementsId = 7;
  original.fileAssociations = {"tc", "tcx"};
  original.discordImage = "tc-logo";

  const nlohmann::json j = original;
  const auto restored = j.get<firelight::platforms::Platform>();

  EXPECT_EQ(restored.id, original.id);
  EXPECT_EQ(restored.name, original.name);
  EXPECT_EQ(restored.abbreviation, original.abbreviation);
  EXPECT_EQ(restored.slug, original.slug);
  EXPECT_EQ(restored.retroAchievementsId, original.retroAchievementsId);
  EXPECT_EQ(restored.fileAssociations, original.fileAssociations);
  EXPECT_EQ(restored.discordImage, original.discordImage);
}

// TEST_F(PlatformServiceTest, PlatformGameboyIsCorrect) {
//   const auto expectedNes = firelight::platforms::Platform{
//       .id = firelight::platforms::PlatformService::PLATFORM_ID_GAMEBOY,
//       .name = "Game Boy",
//       .abbreviation = "Game Boy",
//       .slug = "gb",
//       .fileAssociations = {"gb"},
//       .controllerTypes = {
//           {.id = 1,
//            .name = "Retropad",
//            .imageUrl = "qrc:/images/controllers/gb",
//            .inputs = {
//                {"A", firelight::input::GamepadInput::EastFace},
//                {"B", firelight::input::GamepadInput::SouthFace},
//                {"Start", firelight::input::GamepadInput::Start},
//                {"Select", firelight::input::GamepadInput::Select},
//                {"D-Pad Up", firelight::input::GamepadInput::DpadUp},
//                {"D-Pad Down", firelight::input::GamepadInput::DpadDown},
//                {"D-Pad Left", firelight::input::GamepadInput::DpadLeft},
//                {"D-Pad Right", firelight::input::GamepadInput::DpadRight},
//            }}}};
//
//   const auto &service = firelight::platforms::PlatformService::getInstance();
//
//   const auto nes = service.getPlatform(
//       firelight::platforms::PlatformService::PLATFORM_ID_GAMEBOY);
//   EXPECT_TRUE(nes.has_value());
//
//   assertPlatformsEqual(nes.value(), expectedNes);
// }
//
// TEST_F(PlatformServiceTest, PlatformGameboyColorIsCorrect) {
//   const auto expected = firelight::platforms::Platform{
//       .id = firelight::platforms::PlatformService::PLATFORM_ID_GAMEBOY_COLOR,
//       .name = "Game Boy Color",
//       .abbreviation = "Game Boy Color",
//       .slug = "gbc",
//       .fileAssociations = {"gbc"},
//       .controllerTypes = {
//           {.id = 1,
//            .name = "Retropad",
//            .imageUrl = "qrc:/images/controllers/gbc",
//            .inputs = {
//                {"A", firelight::input::GamepadInput::EastFace},
//                {"B", firelight::input::GamepadInput::SouthFace},
//                {"Start", firelight::input::GamepadInput::Start},
//                {"Select", firelight::input::GamepadInput::Select},
//                {"D-Pad Up", firelight::input::GamepadInput::DpadUp},
//                {"D-Pad Down", firelight::input::GamepadInput::DpadDown},
//                {"D-Pad Left", firelight::input::GamepadInput::DpadLeft},
//                {"D-Pad Right", firelight::input::GamepadInput::DpadRight},
//            }}}};
//
//   const auto &service = firelight::platforms::PlatformService::getInstance();
//
//   const auto actual = service.getPlatform(
//       firelight::platforms::PlatformService::PLATFORM_ID_GAMEBOY_COLOR);
//   EXPECT_TRUE(actual.has_value());
//
//   assertPlatformsEqual(actual.value(), expected);
// }
//
// TEST_F(PlatformServiceTest, PlatformGameboyAdvanceIsCorrect) {
//   const auto expected = firelight::platforms::Platform{
//       .id =
//       firelight::platforms::PlatformService::PLATFORM_ID_GAMEBOY_ADVANCE,
//       .name = "Game Boy Advance",
//       .abbreviation = "Game Boy Advance",
//       .slug = "gba",
//       .fileAssociations = {"gba"},
//       .controllerTypes = {
//           {.id = 1,
//            .name = "Retropad",
//            .imageUrl = "qrc:/images/controllers/gba",
//            .inputs = {
//                {"A", firelight::input::GamepadInput::EastFace},
//                {"B", firelight::input::GamepadInput::SouthFace},
//                {"A (turbo)", firelight::input::GamepadInput::NorthFace},
//                {"B (turbo)", firelight::input::GamepadInput::WestFace},
//                {"L", firelight::input::GamepadInput::LeftBumper},
//                {"R", firelight::input::GamepadInput::RightBumper},
//                {"L (turbo)", firelight::input::GamepadInput::LeftTrigger},
//                {"R (turbo)", firelight::input::GamepadInput::RightTrigger},
//                {"Start", firelight::input::GamepadInput::Start},
//                {"Select", firelight::input::GamepadInput::Select},
//                {"D-Pad Up", firelight::input::GamepadInput::DpadUp},
//                {"D-Pad Down", firelight::input::GamepadInput::DpadDown},
//                {"D-Pad Left", firelight::input::GamepadInput::DpadLeft},
//                {"D-Pad Right", firelight::input::GamepadInput::DpadRight},
//                {"Darken solar sensor", firelight::input::GamepadInput::L3},
//                {"Lighten solar sensor",
//                firelight::input::GamepadInput::R3}}}}};
//   const auto &service = firelight::platforms::PlatformService::getInstance();
//   const auto actual = service.getPlatform(
//       firelight::platforms::PlatformService::PLATFORM_ID_GAMEBOY_ADVANCE);
//   EXPECT_TRUE(actual.has_value());
//   assertPlatformsEqual(actual.value(), expected);
// }
//
// TEST_F(PlatformServiceTest, PlatformNESIsCorrect) {
//   const auto expected = firelight::platforms::Platform{
//       .id = firelight::platforms::PlatformService::PLATFORM_ID_NES,
//       .name = "NES/Famicom",
//       .abbreviation = "NES",
//       .slug = "nes",
//       .fileAssociations = {"nes"},
//       .controllerTypes = {
//           {.id = 1,
//            .name = "Retropad",
//            .imageUrl = "qrc:/images/controllers/nes",
//            .inputs = {
//                {"A", firelight::input::GamepadInput::EastFace},
//                {"B", firelight::input::GamepadInput::SouthFace},
//                {"Start", firelight::input::GamepadInput::Start},
//                {"Select", firelight::input::GamepadInput::Select},
//                {"D-Pad Up", firelight::input::GamepadInput::DpadUp},
//                {"D-Pad Down", firelight::input::GamepadInput::DpadDown},
//                {"D-Pad Left", firelight::input::GamepadInput::DpadLeft},
//                {"D-Pad Right", firelight::input::GamepadInput::DpadRight},
//            }}}};
//   const auto &service = firelight::platforms::PlatformService::getInstance();
//   const auto actual = service.getPlatform(
//       firelight::platforms::PlatformService::PLATFORM_ID_NES);
//   EXPECT_TRUE(actual.has_value());
//   assertPlatformsEqual(actual.value(), expected);
// }
//
// TEST_F(PlatformServiceTest, PlatformSnesIsCorrect) {
//   const auto expected = firelight::platforms::Platform{
//       .id = firelight::platforms::PlatformService::PLATFORM_ID_SNES,
//       .name = "SNES/Super Famicom",
//       .abbreviation = "SNES",
//       .slug = "snes",
//       .fileAssociations = {"sfc", "smc"},
//       .controllerTypes = {
//           {.id = 1,
//            .name = "Retropad",
//            .imageUrl = "qrc:/images/controllers/snes",
//            .inputs = {
//                {"A", firelight::input::GamepadInput::EastFace},
//                {"B", firelight::input::GamepadInput::SouthFace},
//                {"X", firelight::input::GamepadInput::NorthFace},
//                {"Y", firelight::input::GamepadInput::WestFace},
//                {"L", firelight::input::GamepadInput::LeftBumper},
//                {"R", firelight::input::GamepadInput::RightBumper},
//                {"Start", firelight::input::GamepadInput::Start},
//                {"Select", firelight::input::GamepadInput::Select},
//                {"D-Pad Up", firelight::input::GamepadInput::DpadUp},
//                {"D-Pad Down", firelight::input::GamepadInput::DpadDown},
//                {"D-Pad Left", firelight::input::GamepadInput::DpadLeft},
//                {"D-Pad Right", firelight::input::GamepadInput::DpadRight},
//            }}}};
//   const auto &service = firelight::platforms::PlatformService::getInstance();
//   const auto actual = service.getPlatform(
//       firelight::platforms::PlatformService::PLATFORM_ID_SNES);
//   EXPECT_TRUE(actual.has_value());
//   assertPlatformsEqual(actual.value(), expected);
// }
//
// TEST_F(PlatformServiceTest, PlatformN64IsCorrect) {
//   const auto expected = firelight::platforms::Platform{
//       .id = firelight::platforms::PlatformService::PLATFORM_ID_N64,
//       .name = "Nintendo 64",
//       .abbreviation = "N64",
//       .slug = "n64",
//       .fileAssociations = {"n64", "z64", "v64"},
//       .controllerTypes = {
//           {.id = 1,
//            .name = "Retropad",
//            .imageUrl = "qrc:/images/controllers/n64",
//            .inputs = {
//                {"A", firelight::input::GamepadInput::SouthFace},
//                {"B", firelight::input::GamepadInput::WestFace},
//                {"Z", firelight::input::GamepadInput::LeftTrigger},
//                {"L", firelight::input::GamepadInput::LeftBumper},
//                {"R", firelight::input::GamepadInput::RightBumper},
//                {"Start", firelight::input::GamepadInput::Start},
//                {"Select", firelight::input::GamepadInput::Select},
//                {"C-Up", firelight::input::GamepadInput::RightStickUp},
//                {"C-Down", firelight::input::GamepadInput::RightStickDown},
//                {"C-Left", firelight::input::GamepadInput::RightStickLeft},
//                {"C-Right", firelight::input::GamepadInput::RightStickRight},
//                {"D-Pad Up", firelight::input::GamepadInput::DpadUp},
//                {"D-Pad Down", firelight::input::GamepadInput::DpadDown},
//                {"D-Pad Left", firelight::input::GamepadInput::DpadLeft},
//                {"D-Pad Right", firelight::input::GamepadInput::DpadRight},
//                {"Analog Stick Up",
//                firelight::input::GamepadInput::LeftStickUp},
//                {"Analog Stick Down",
//                 firelight::input::GamepadInput::LeftStickDown},
//                {"Analog Stick Left",
//                 firelight::input::GamepadInput::LeftStickLeft},
//                {"Analog Stick Right",
//                 firelight::input::GamepadInput::LeftStickRight},
//            }}}};
//   const auto &service = firelight::platforms::PlatformService::getInstance();
//   const auto actual = service.getPlatform(
//       firelight::platforms::PlatformService::PLATFORM_ID_N64);
//   EXPECT_TRUE(actual.has_value());
//   assertPlatformsEqual(actual.value(), expected);
// }
//
// TEST_F(PlatformServiceTest, PlatformNintendoDSIsCorrect) {
//   const auto expected = firelight::platforms::Platform{
//       .id = firelight::platforms::PlatformService::PLATFORM_ID_NINTENDO_DS,
//       .name = "Nintendo DS",
//       .abbreviation = "NDS",
//       .slug = "nds",
//       .fileAssociations = {"nds"},
//       .controllerTypes = {
//           {.id = 1,
//            .name = "Retropad",
//            .imageUrl = "qrc:/images/controllers/nds",
//            .inputs = {
//                {"A", firelight::input::GamepadInput::EastFace},
//                {"B", firelight::input::GamepadInput::SouthFace},
//                {"X", firelight::input::GamepadInput::NorthFace},
//                {"Y", firelight::input::GamepadInput::WestFace},
//                {"L", firelight::input::GamepadInput::LeftBumper},
//                {"R", firelight::input::GamepadInput::RightBumper},
//                {"Start", firelight::input::GamepadInput::Start},
//                {"Select", firelight::input::GamepadInput::Select},
//                {"D-Pad Up", firelight::input::GamepadInput::DpadUp},
//                {"D-Pad Down", firelight::input::GamepadInput::DpadDown},
//                {"D-Pad Left", firelight::input::GamepadInput::DpadLeft},
//                {"D-Pad Right", firelight::input::GamepadInput::DpadRight},
//                {"Move cursor up",
//                firelight::input::GamepadInput::RightStickUp},
//                {"Move cursor down",
//                 firelight::input::GamepadInput::RightStickDown},
//                {"Move cursor left",
//                 firelight::input::GamepadInput::RightStickLeft},
//                {"Move cursor right",
//                 firelight::input::GamepadInput::RightStickRight},
//                {"Touch with cursor", firelight::input::GamepadInput::R3},
//                {"Enable microphone (hold)",
//                 firelight::input::GamepadInput::LeftTrigger},
//                {"Change screen layout",
//                 firelight::input::GamepadInput::RightTrigger},
//                {"Close lid", firelight::input::GamepadInput::L3},
//            }}}};
//   const auto &service = firelight::platforms::PlatformService::getInstance();
//   const auto actual = service.getPlatform(
//       firelight::platforms::PlatformService::PLATFORM_ID_NINTENDO_DS);
//   EXPECT_TRUE(actual.has_value());
//   assertPlatformsEqual(actual.value(), expected);
// }
//
// TEST_F(PlatformServiceTest, PlatformSegaMasterSystemIsCorrect) {
//   const auto expected = firelight::platforms::Platform{
//       .id =
//           firelight::platforms::PlatformService::PLATFORM_ID_SEGA_MASTER_SYSTEM,
//       .name = "Master System",
//       .abbreviation = "Master System",
//       .slug = "sms",
//       .fileAssociations = {"sms"},
//       .controllerTypes = {
//           {.id = 1,
//            .name = "Retropad",
//            .imageUrl = "qrc:/images/controllers/sms",
//            .inputs = {
//                {"1", firelight::input::GamepadInput::SouthFace},
//                {"2", firelight::input::GamepadInput::EastFace},
//                {"Start", firelight::input::GamepadInput::Start},
//                {"D-Pad Up", firelight::input::GamepadInput::DpadUp},
//                {"D-Pad Down", firelight::input::GamepadInput::DpadDown},
//                {"D-Pad Left", firelight::input::GamepadInput::DpadLeft},
//                {"D-Pad Right", firelight::input::GamepadInput::DpadRight},
//            }}}};
//   const auto &service = firelight::platforms::PlatformService::getInstance();
//   const auto actual = service.getPlatform(
//       firelight::platforms::PlatformService::PLATFORM_ID_SEGA_MASTER_SYSTEM);
//   EXPECT_TRUE(actual.has_value());
//   assertPlatformsEqual(actual.value(), expected);
// }
//
// TEST_F(PlatformServiceTest, PlatformSegaGenesisIsCorrect) {
//   const auto expected = firelight::platforms::Platform{
//       .id = firelight::platforms::PlatformService::PLATFORM_ID_SEGA_GENESIS,
//       .name = "Genesis/Mega Drive",
//       .abbreviation = "Genesis/MD",
//       .slug = "gen",
//       .fileAssociations = {"md", "gen"},
//       .controllerTypes = {
//           {.id = 1,
//            .name = "Retropad",
//            .imageUrl = "qrc:/images/controllers/gen-threebutton",
//            .inputs = {
//                {"A", firelight::input::GamepadInput::WestFace},
//                {"B", firelight::input::GamepadInput::SouthFace},
//                {"C", firelight::input::GamepadInput::EastFace},
//                {"X", firelight::input::GamepadInput::LeftBumper},
//                {"Y", firelight::input::GamepadInput::NorthFace},
//                {"Z", firelight::input::GamepadInput::RightBumper},
//                {"Start", firelight::input::GamepadInput::Start},
//                {"Mode", firelight::input::GamepadInput::Select},
//                {"D-Pad Up", firelight::input::GamepadInput::DpadUp},
//                {"D-Pad Down", firelight::input::GamepadInput::DpadDown},
//                {"D-Pad Left", firelight::input::GamepadInput::DpadLeft},
//                {"D-Pad Right", firelight::input::GamepadInput::DpadRight},
//            }}}};
//   const auto &service = firelight::platforms::PlatformService::getInstance();
//   const auto actual = service.getPlatform(
//       firelight::platforms::PlatformService::PLATFORM_ID_SEGA_GENESIS);
//   EXPECT_TRUE(actual.has_value());
//   assertPlatformsEqual(actual.value(), expected);
// }
//
// TEST_F(PlatformServiceTest, PlatformSegaGameGearIsCorrect) {
//   const auto expected = firelight::platforms::Platform{
//       .id = firelight::platforms::PlatformService::PLATFORM_ID_SEGA_GAMEGEAR,
//       .name = "Game Gear",
//       .abbreviation = "Game Gear",
//       .slug = "gg",
//       .fileAssociations = {"gg"},
//       .controllerTypes = {
//           {.id = 1,
//            .name = "Retropad",
//            .imageUrl = "qrc:/images/controllers/gg",
//            .inputs = {
//                {"1", firelight::input::GamepadInput::SouthFace},
//                {"2", firelight::input::GamepadInput::EastFace},
//                {"Start", firelight::input::GamepadInput::Start},
//                {"D-Pad Up", firelight::input::GamepadInput::DpadUp},
//                {"D-Pad Down", firelight::input::GamepadInput::DpadDown},
//                {"D-Pad Left", firelight::input::GamepadInput::DpadLeft},
//                {"D-Pad Right", firelight::input::GamepadInput::DpadRight},
//            }}}};
//   const auto &service = firelight::platforms::PlatformService::getInstance();
//   const auto actual = service.getPlatform(
//       firelight::platforms::PlatformService::PLATFORM_ID_SEGA_GAMEGEAR);
//   EXPECT_TRUE(actual.has_value());
//   assertPlatformsEqual(actual.value(), expected);
// }
//
// TEST_F(PlatformServiceTest, PlatformTurboGrafx16IsCorrect) {
//   const auto expected = firelight::platforms::Platform{
//       .id = firelight::platforms::PlatformService::PLATFORM_ID_TURBOGRAFX16,
//       .name = "PC Engine/TurboGrafx-16",
//       .abbreviation = "PCE/TG-16",
//       .slug = "pce",
//       .fileAssociations = {"pce"},
//       .controllerTypes = {
//           {.id = 1,
//            .name = "Retropad",
//            .imageUrl = "qrc:/images/controllers/tgx-twobutton",
//            .inputs = {
//                {"I", firelight::input::GamepadInput::EastFace},
//                {"II", firelight::input::GamepadInput::SouthFace},
//                {"III (six-button mode)",
//                 firelight::input::GamepadInput::WestFace},
//                {"IV (six-button mode)",
//                 firelight::input::GamepadInput::NorthFace},
//                {"V (six-button mode)",
//                 firelight::input::GamepadInput::LeftBumper},
//                {"VI (six-button mode)",
//                 firelight::input::GamepadInput::RightBumper},
//                {"Toggle mode", firelight::input::GamepadInput::LeftTrigger},
//                {"Run", firelight::input::GamepadInput::Start},
//                {"Select", firelight::input::GamepadInput::Select},
//                {"D-Pad Up", firelight::input::GamepadInput::DpadUp},
//                {"D-Pad Down", firelight::input::GamepadInput::DpadDown},
//                {"D-Pad Left", firelight::input::GamepadInput::DpadLeft},
//                {"D-Pad Right", firelight::input::GamepadInput::DpadRight},
//            }}}};
//   const auto &service = firelight::platforms::PlatformService::getInstance();
//   const auto actual = service.getPlatform(
//       firelight::platforms::PlatformService::PLATFORM_ID_TURBOGRAFX16);
//   EXPECT_TRUE(actual.has_value());
//   assertPlatformsEqual(actual.value(), expected);
// }
//
// TEST_F(PlatformServiceTest, PlatformSuperGrafxIsCorrect) {
//   const auto expected = firelight::platforms::Platform{
//       .id = firelight::platforms::PlatformService::PLATFORM_ID_SUPERGRAFX,
//       .name = "SuperGrafx",
//       .abbreviation = "SuperGrafx",
//       .slug = "sgx",
//       .fileAssociations = {"sgx"},
//       .controllerTypes = {
//           {.id = 1,
//            .name = "Retropad",
//            .imageUrl = "qrc:/images/controllers/tgx-twobutton",
//            .inputs = {
//                {"I", firelight::input::GamepadInput::EastFace},
//                {"II", firelight::input::GamepadInput::SouthFace},
//                {"III (six-button mode)",
//                 firelight::input::GamepadInput::WestFace},
//                {"IV (six-button mode)",
//                 firelight::input::GamepadInput::NorthFace},
//                {"V (six-button mode)",
//                 firelight::input::GamepadInput::LeftBumper},
//                {"VI (six-button mode)",
//                 firelight::input::GamepadInput::RightBumper},
//                {"Toggle mode", firelight::input::GamepadInput::LeftTrigger},
//                {"Run", firelight::input::GamepadInput::Start},
//                {"Select", firelight::input::GamepadInput::Select},
//                {"D-Pad Up", firelight::input::GamepadInput::DpadUp},
//                {"D-Pad Down", firelight::input::GamepadInput::DpadDown},
//                {"D-Pad Left", firelight::input::GamepadInput::DpadLeft},
//                {"D-Pad Right", firelight::input::GamepadInput::DpadRight},
//            }}}};
//   const auto &service = firelight::platforms::PlatformService::getInstance();
//   const auto actual = service.getPlatform(
//       firelight::platforms::PlatformService::PLATFORM_ID_SUPERGRAFX);
//   EXPECT_TRUE(actual.has_value());
//   assertPlatformsEqual(actual.value(), expected);
// }
//
// TEST_F(PlatformServiceTest, PlatformPokemonMiniIsCorrect) {
//   const auto expected = firelight::platforms::Platform{
//       .id = firelight::platforms::PlatformService::PLATFORM_ID_POKEMON_MINI,
//       .name = "Pokémon Mini",
//       .abbreviation = "Pokémon Mini",
//       .slug = "pkmn",
//       .fileAssociations = {"min"},
//       .controllerTypes = {
//           {.id = 1,
//            .name = "Retropad",
//            .imageUrl = "qrc:/images/controllers/pkmn-mini",
//            .inputs = {
//                {"A", firelight::input::GamepadInput::EastFace},
//                {"B", firelight::input::GamepadInput::SouthFace},
//                {"C", firelight::input::GamepadInput::RightBumper},
//                {"D-Pad Up", firelight::input::GamepadInput::DpadUp},
//                {"D-Pad Down", firelight::input::GamepadInput::DpadDown},
//                {"D-Pad Left", firelight::input::GamepadInput::DpadLeft},
//                {"D-Pad Right", firelight::input::GamepadInput::DpadRight},
//                {"Shake", firelight::input::GamepadInput::LeftBumper},
//                {"Power", firelight::input::GamepadInput::Select},
//            }}}};
//   const auto &service = firelight::platforms::PlatformService::getInstance();
//   const auto actual = service.getPlatform(
//       firelight::platforms::PlatformService::PLATFORM_ID_POKEMON_MINI);
//   EXPECT_TRUE(actual.has_value());
//   assertPlatformsEqual(actual.value(), expected);
// }
//
// TEST_F(PlatformServiceTest, PlatformWonderSwanIsCorrect) {
//   const auto expected = firelight::platforms::Platform{
//       .id = firelight::platforms::PlatformService::PLATFORM_ID_WONDERSWAN,
//       .name = "WonderSwan",
//       .abbreviation = "WonderSwan",
//       .slug = "ws",
//       .fileAssociations = {"ws", "wsc"},
//       .controllerTypes = {
//           {.id = 1,
//            .name = "Retropad",
//            .imageUrl = "qrc:/images/controllers/ws-horizontal",
//            .inputs = {
//                {"X1 (Horizontal), Y2 (Vertical)",
//                 firelight::input::GamepadInput::DpadUp},
//                {"X2 (Horizontal), Y3 (Vertical)",
//                 firelight::input::GamepadInput::DpadRight},
//                {"X3 (Horizontal), Y4 (Vertical)",
//                 firelight::input::GamepadInput::DpadDown},
//                {"X4 (Horizontal), Y1 (Vertical)",
//                 firelight::input::GamepadInput::DpadLeft},
//                {"A (Horizontal), X3 (Vertical)",
//                 firelight::input::GamepadInput::EastFace},
//                {"B (Horizontal), X4 (Vertical)",
//                 firelight::input::GamepadInput::SouthFace},
//                {"X1 (Horizontal)", firelight::input::GamepadInput::WestFace},
//                {"X2 (Horizontal)",
//                firelight::input::GamepadInput::NorthFace},
//                {"Start", firelight::input::GamepadInput::Start},
//                {"Switch orientation",
//                firelight::input::GamepadInput::Select},
//            }}}};
//   const auto &service = firelight::platforms::PlatformService::getInstance();
//   const auto actual = service.getPlatform(
//       firelight::platforms::PlatformService::PLATFORM_ID_WONDERSWAN);
//   EXPECT_TRUE(actual.has_value());
//   assertPlatformsEqual(actual.value(), expected);
// }
