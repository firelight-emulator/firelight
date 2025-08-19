#include "../../../src/app/platforms/platform_service.hpp"
#include <gtest/gtest.h>

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
  const auto &service = firelight::platforms::PlatformService::getInstance();
  EXPECT_EQ(14, service.listPlatforms().size());
}

// TEST_F(PlatformServiceTest, PlatformGameboyIsCorrect) {
//   const auto expectedNes = firelight::platforms::Platform{
//       .id = firelight::platforms::PlatformService::PLATFORM_ID_GAMEBOY,
//       .name = "Game Boy",
//       .abbreviation = "Game Boy",
//       .slug = "gb",
//       .fileAssociations = {"gb"},
//       .controllerTypes =
//           {{.id = 1,
//             .name = "Retropad",
//             .imageUrl = "qrc:/images/controllers/gb",
//             .inputs =
//                 {
//                     {"A", firelight::input::GamepadInput::EastFace},
//                     {"B", firelight::input::GamepadInput::SouthFace},
//                     {"Start", firelight::input::GamepadInput::Start},
//                     {"Select", firelight::input::GamepadInput::Select},
//                     {"D-Pad Up", firelight::input::GamepadInput::DpadUp},
//                     {"D-Pad Down", firelight::input::GamepadInput::DpadDown},
//                     {"D-Pad Left", firelight::input::GamepadInput::DpadLeft},
//                     {"D-Pad Right",
//                     firelight::input::GamepadInput::DpadRight},
//                 }}},
//       .settings = {
//           firelight::platforms::PlatformSetting{
//               .key = "gambatte_gbc_color_correction",
//               .defaultValue = "GBC only"},
//           firelight::platforms::PlatformSetting{
//               .key = "gambatte_gbc_color_correction_mode",
//               .defaultValue = "accurate"},
//           firelight::platforms::PlatformSetting{
//               .key = "gambatte_gbc_frontlight_position",
//               .defaultValue = "central"},
//           firelight::platforms::PlatformSetting{
//               .key = "gambatte_dark_filter_level", .defaultValue = "0"},
//           firelight::platforms::PlatformSetting{
//               .key = "gambatte_up_down_allowed", .defaultValue = "disabled"},
//           firelight::platforms::PlatformSetting{.key = "gambatte_mix_frames",
//                                                 .defaultValue = "disabled"},
//           firelight::platforms::PlatformSetting{
//               .key = "gambatte_gb_colorization", .defaultValue = "auto"},
//           firelight::platforms::PlatformSetting{
//               .key = "gambatte_gb_internal_palette",
//               .defaultValue = "GB - DMG"},
//           firelight::platforms::PlatformSetting{.key =
//           "gambatte_gb_bootloader",
//                                                 .defaultValue = "disabled"},
//           firelight::platforms::PlatformSetting{.key = "gambatte_gb_hwmode",
//                                                 .defaultValue = "auto"}}};
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
//       .controllerTypes =
//           {{.id = 1,
//             .name = "Retropad",
//             .imageUrl = "qrc:/images/controllers/gbc",
//             .inputs =
//                 {
//                     {"A", firelight::input::GamepadInput::EastFace},
//                     {"B", firelight::input::GamepadInput::SouthFace},
//                     {"Start", firelight::input::GamepadInput::Start},
//                     {"Select", firelight::input::GamepadInput::Select},
//                     {"D-Pad Up", firelight::input::GamepadInput::DpadUp},
//                     {"D-Pad Down", firelight::input::GamepadInput::DpadDown},
//                     {"D-Pad Left", firelight::input::GamepadInput::DpadLeft},
//                     {"D-Pad Right",
//                     firelight::input::GamepadInput::DpadRight},
//                 }}},
//       .settings = {
//           firelight::platforms::PlatformSetting{
//               .key = "mgba_solar_sensor_level", .defaultValue = "0"},
//           firelight::platforms::PlatformSetting{.key = "mgba_gb_model",
//                                                 .defaultValue =
//                                                 "Autodetect"},
//           firelight::platforms::PlatformSetting{.key = "mgba_sgb_borders",
//                                                 .defaultValue = "ON"},
//           firelight::platforms::PlatformSetting{.key =
//           "mgba_gb_colors_preset",
//                                                 .defaultValue = "0"},
//           firelight::platforms::PlatformSetting{.key = "mgba_gb_colors",
//                                                 .defaultValue = "Grayscale"},
//           firelight::platforms::PlatformSetting{.key = "mgba_use_bios",
//                                                 .defaultValue = "ON"},
//           firelight::platforms::PlatformSetting{.key = "mgba_skip_bios",
//                                                 .defaultValue = "OFF"},
//           firelight::platforms::PlatformSetting{.key = "mgba_frameskip",
//                                                 .defaultValue = "disabled"},
//           firelight::platforms::PlatformSetting{
//               .key = "mgba_frameskip_threshold", .defaultValue = "33"},
//           firelight::platforms::PlatformSetting{
//               .key = "mgba_frameskip_interval", .defaultValue = "0"},
//           firelight::platforms::PlatformSetting{
//               .key = "mgba_audio_low_pass_filter", .defaultValue =
//               "disabled"},
//           firelight::platforms::PlatformSetting{
//               .key = "mgba_audio_low_pass_range", .defaultValue = "60"},
//           firelight::platforms::PlatformSetting{.key =
//           "mgba_idle_optimization",
//                                                 .defaultValue = "Remove
//                                                 Known"},
//           firelight::platforms::PlatformSetting{.key = "mgba_force_gbp",
//                                                 .defaultValue = "OFF"},
//           firelight::platforms::PlatformSetting{.key =
//           "mgba_color_correction",
//                                                 .defaultValue = "OFF"},
//           firelight::platforms::PlatformSetting{
//               .key = "mgba_interframe_blending", .defaultValue =
//               "mix_smart"}}};
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
//       .controllerTypes =
//           {{.id = 1,
//             .name = "Retropad",
//             .imageUrl = "qrc:/images/controllers/gba",
//             .inputs =
//                 {{"A", firelight::input::GamepadInput::EastFace},
//                  {"B", firelight::input::GamepadInput::SouthFace},
//                  {"A (turbo)", firelight::input::GamepadInput::NorthFace},
//                  {"B (turbo)", firelight::input::GamepadInput::WestFace},
//                  {"L", firelight::input::GamepadInput::LeftBumper},
//                  {"R", firelight::input::GamepadInput::RightBumper},
//                  {"L (turbo)", firelight::input::GamepadInput::LeftTrigger},
//                  {"R (turbo)", firelight::input::GamepadInput::RightTrigger},
//                  {"Start", firelight::input::GamepadInput::Start},
//                  {"Select", firelight::input::GamepadInput::Select},
//                  {"D-Pad Up", firelight::input::GamepadInput::DpadUp},
//                  {"D-Pad Down", firelight::input::GamepadInput::DpadDown},
//                  {"D-Pad Left", firelight::input::GamepadInput::DpadLeft},
//                  {"D-Pad Right", firelight::input::GamepadInput::DpadRight},
//                  {"Darken solar sensor", firelight::input::GamepadInput::L3},
//                  {"Lighten solar sensor",
//                   firelight::input::GamepadInput::R3}}}},
//       .settings = {
//           firelight::platforms::PlatformSetting{
//               .key = "mgba_solar_sensor_level", .defaultValue = "0"},
//           firelight::platforms::PlatformSetting{.key = "mgba_gb_model",
//                                                 .defaultValue =
//                                                 "Autodetect"},
//           firelight::platforms::PlatformSetting{.key = "mgba_sgb_borders",
//                                                 .defaultValue = "ON"},
//           firelight::platforms::PlatformSetting{.key =
//           "mgba_gb_colors_preset",
//                                                 .defaultValue = "0"},
//           firelight::platforms::PlatformSetting{
//               .key = "mgba_gb_colors", .defaultValue = "Grayscale"},
//           firelight::platforms::PlatformSetting{.key = "mgba_use_bios",
//                                                 .defaultValue = "ON"},
//           firelight::platforms::PlatformSetting{
//               .key = "mgba_skip_bios", .defaultValue = "OFF"},
//           firelight::platforms::PlatformSetting{
//               .key = "mgba_frameskip", .defaultValue = "disabled"},
//           firelight::platforms::PlatformSetting{
//               .key = "mgba_frameskip_threshold", .defaultValue = "33"},
//           firelight::platforms::PlatformSetting{
//               .key = "mgba_frameskip_interval", .defaultValue = "0"},
//           firelight::platforms::PlatformSetting{
//               .key = "mgba_audio_low_pass_filter", .defaultValue =
//               "disabled"},
//           firelight::platforms::PlatformSetting{
//               .key = "mgba_audio_low_pass_range", .defaultValue = "60"},
//           firelight::platforms::PlatformSetting{.key =
//           "mgba_idle_optimization",
//                                                 .defaultValue = "Remove
//                                                 Known"},
//           firelight::platforms::PlatformSetting{.key = "mgba_force_gbp",
//                                                 .defaultValue = "OFF"},
//           firelight::platforms::PlatformSetting{.key =
//           "mgba_color_correction",
//                                                 .defaultValue = "OFF"},
//           firelight::platforms::PlatformSetting{
//               .key = "mgba_interframe_blending", .defaultValue =
//               "mix_smart"}}};
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
//       .controllerTypes =
//           {{.id = 1,
//             .name = "Retropad",
//             .imageUrl = "qrc:/images/controllers/nes",
//             .inputs =
//                 {
//                     {"A", firelight::input::GamepadInput::EastFace},
//                     {"B", firelight::input::GamepadInput::SouthFace},
//                     {"Start", firelight::input::GamepadInput::Start},
//                     {"Select", firelight::input::GamepadInput::Select},
//                     {"D-Pad Up", firelight::input::GamepadInput::DpadUp},
//                     {"D-Pad Down", firelight::input::GamepadInput::DpadDown},
//                     {"D-Pad Left", firelight::input::GamepadInput::DpadLeft},
//                     {"D-Pad Right",
//                     firelight::input::GamepadInput::DpadRight},
//                 }}},
//       .settings = {
//           firelight::platforms::PlatformSetting{.key = "fceumm_apu_1",
//                                                 .defaultValue = "enabled"},
//           firelight::platforms::PlatformSetting{.key = "fceumm_apu_2",
//                                                 .defaultValue = "enabled"},
//           firelight::platforms::PlatformSetting{.key = "fceumm_apu_3",
//                                                 .defaultValue = "enabled"},
//           firelight::platforms::PlatformSetting{.key = "fceumm_apu_4",
//                                                 .defaultValue = "enabled"},
//           firelight::platforms::PlatformSetting{.key = "fceumm_apu_5",
//                                                 .defaultValue = "enabled"},
//           firelight::platforms::PlatformSetting{.key =
//           "fceumm_arkanoid_mode",
//                                                 .defaultValue = "mouse"},
//           firelight::platforms::PlatformSetting{.key = "fceumm_aspect",
//                                                 .defaultValue = "8:7 PAR"},
//           firelight::platforms::PlatformSetting{.key = "fceumm_game_genie",
//                                                 .defaultValue = "disabled"},
//           firelight::platforms::PlatformSetting{
//               .key = "fceumm_mouse_sensitivity", .defaultValue = "100"},
//           firelight::platforms::PlatformSetting{.key =
//           "fceumm_nospritelimit",
//                                                 .defaultValue = "disabled"},
//           firelight::platforms::PlatformSetting{.key = "fceumm_ntsc_filter",
//                                                 .defaultValue = "disabled"},
//           firelight::platforms::PlatformSetting{.key = "fceumm_overclocking",
//                                                 .defaultValue = "disabled"},
//           firelight::platforms::PlatformSetting{.key =
//           "fceumm_overscan_h_left",
//                                                 .defaultValue = "0"},
//           firelight::platforms::PlatformSetting{
//               .key = "fceumm_overscan_h_right", .defaultValue = "0"},
//           firelight::platforms::PlatformSetting{
//               .key = "fceumm_overscan_v_bottom", .defaultValue = "8"},
//           firelight::platforms::PlatformSetting{.key =
//           "fceumm_overscan_v_top",
//                                                 .defaultValue = "8"},
//           firelight::platforms::PlatformSetting{.key = "fceumm_palette",
//                                                 .defaultValue = "default"},
//           firelight::platforms::PlatformSetting{.key = "fceumm_ramstate",
//                                                 .defaultValue = "$ff"},
//           firelight::platforms::PlatformSetting{
//               .key = "fceumm_show_adv_sound_options",
//               .defaultValue = "disabled"},
//           firelight::platforms::PlatformSetting{
//               .key = "fceumm_show_adv_system_options",
//               .defaultValue = "disabled"},
//           firelight::platforms::PlatformSetting{.key =
//           "fceumm_show_crosshair",
//                                                 .defaultValue = "enabled"},
//           firelight::platforms::PlatformSetting{.key = "fceumm_sndlowpass",
//                                                 .defaultValue = "disabled"},
//           firelight::platforms::PlatformSetting{.key = "fceumm_sndquality",
//                                                 .defaultValue = "Very High"},
//           firelight::platforms::PlatformSetting{.key =
//           "fceumm_sndstereodelay",
//                                                 .defaultValue =
//                                                 "15_ms_delay"},
//           firelight::platforms::PlatformSetting{.key = "fceumm_sndvolume",
//                                                 .defaultValue = "7"},
//           firelight::platforms::PlatformSetting{.key = "fceumm_swapduty",
//                                                 .defaultValue = "disabled"},
//           firelight::platforms::PlatformSetting{.key = "fceumm_turbo_delay",
//                                                 .defaultValue = "3"},
//           firelight::platforms::PlatformSetting{.key = "fceumm_turbo_enable",
//                                                 .defaultValue = "None"},
//           firelight::platforms::PlatformSetting{.key =
//           "fceumm_up_down_allowed",
//                                                 .defaultValue = "disabled"},
//           firelight::platforms::PlatformSetting{.key = "fceumm_zapper_mode",
//                                                 .defaultValue = "clightgun"},
//           firelight::platforms::PlatformSetting{.key =
//           "fceumm_zapper_sensor",
//                                                 .defaultValue = "enabled"},
//           firelight::platforms::PlatformSetting{
//               .key = "fceumm_zapper_tolerance", .defaultValue = "6"},
//           firelight::platforms::PlatformSetting{.key =
//           "fceumm_zapper_trigger",
//                                                 .defaultValue = "enabled"},
//       }};
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
//       .controllerTypes =
//           {{.id = 1,
//             .name = "Retropad",
//             .imageUrl = "qrc:/images/controllers/snes",
//             .inputs =
//                 {
//                     {"A", firelight::input::GamepadInput::EastFace},
//                     {"B", firelight::input::GamepadInput::SouthFace},
//                     {"X", firelight::input::GamepadInput::NorthFace},
//                     {"Y", firelight::input::GamepadInput::WestFace},
//                     {"L", firelight::input::GamepadInput::LeftBumper},
//                     {"R", firelight::input::GamepadInput::RightBumper},
//                     {"Start", firelight::input::GamepadInput::Start},
//                     {"Select", firelight::input::GamepadInput::Select},
//                     {"D-Pad Up", firelight::input::GamepadInput::DpadUp},
//                     {"D-Pad Down", firelight::input::GamepadInput::DpadDown},
//                     {"D-Pad Left", firelight::input::GamepadInput::DpadLeft},
//                     {"D-Pad Right",
//                     firelight::input::GamepadInput::DpadRight},
//                 }}},
//       .settings = {
//           firelight::platforms::PlatformSetting{.key = "snes9x_hires_blend",
//                                                 .defaultValue = "disabled"},
//           firelight::platforms::PlatformSetting{
//               .key = "snes9x_overclock_superfx", .defaultValue = "100%"},
//           firelight::platforms::PlatformSetting{.key =
//           "snes9x_up_down_allowed",
//                                                 .defaultValue = "disabled"},
//           firelight::platforms::PlatformSetting{.key = "snes9x_sndchan_1",
//                                                 .defaultValue = "enabled"},
//           firelight::platforms::PlatformSetting{.key = "snes9x_sndchan_2",
//                                                 .defaultValue = "enabled"},
//           firelight::platforms::PlatformSetting{.key = "snes9x_sndchan_3",
//                                                 .defaultValue = "enabled"},
//           firelight::platforms::PlatformSetting{.key = "snes9x_sndchan_4",
//                                                 .defaultValue = "enabled"},
//           firelight::platforms::PlatformSetting{.key = "snes9x_sndchan_5",
//                                                 .defaultValue = "enabled"},
//           firelight::platforms::PlatformSetting{.key = "snes9x_sndchan_6",
//                                                 .defaultValue = "enabled"},
//           firelight::platforms::PlatformSetting{.key = "snes9x_sndchan_7",
//                                                 .defaultValue = "enabled"},
//           firelight::platforms::PlatformSetting{.key = "snes9x_sndchan_8",
//                                                 .defaultValue = "enabled"},
//           firelight::platforms::PlatformSetting{.key = "snes9x_layer_1",
//                                                 .defaultValue = "enabled"},
//           firelight::platforms::PlatformSetting{.key = "snes9x_layer_2",
//                                                 .defaultValue = "enabled"},
//           firelight::platforms::PlatformSetting{.key = "snes9x_layer_3",
//                                                 .defaultValue = "enabled"},
//           firelight::platforms::PlatformSetting{.key = "snes9x_layer_4",
//                                                 .defaultValue = "enabled"},
//           firelight::platforms::PlatformSetting{.key = "snes9x_layer_5",
//                                                 .defaultValue = "enabled"},
//           firelight::platforms::PlatformSetting{.key = "snes9x_gfx_clip",
//                                                 .defaultValue = "enabled"},
//           firelight::platforms::PlatformSetting{.key = "snes9x_gfx_transp",
//                                                 .defaultValue = "enabled"},
//           firelight::platforms::PlatformSetting{
//               .key = "snes9x_audio_interpolation", .defaultValue =
//               "gaussian"},
//           firelight::platforms::PlatformSetting{
//               .key = "snes9x_overclock_cycles", .defaultValue = "disabled"},
//           firelight::platforms::PlatformSetting{
//               .key = "snes9x_reduce_sprite_flicker",
//               .defaultValue = "disabled"},
//           firelight::platforms::PlatformSetting{
//               .key = "snes9x_randomize_memory", .defaultValue = "disabled"},
//           firelight::platforms::PlatformSetting{.key = "snes9x_overscan",
//                                                 .defaultValue = "enabled"},
//           firelight::platforms::PlatformSetting{.key = "snes9x_aspect",
//                                                 .defaultValue = "4:3"},
//           firelight::platforms::PlatformSetting{.key = "snes9x_region",
//                                                 .defaultValue = "auto"},
//           firelight::platforms::PlatformSetting{.key =
//           "snes9x_lightgun_mode",
//                                                 .defaultValue = "Lightgun"},
//           firelight::platforms::PlatformSetting{
//               .key = "snes9x_superscope_reverse_buttons",
//               .defaultValue = "disabled"},
//           firelight::platforms::PlatformSetting{
//               .key = "snes9x_superscope_crosshair", .defaultValue = "2"},
//           firelight::platforms::PlatformSetting{
//               .key = "snes9x_superscope_color", .defaultValue = "White"},
//           firelight::platforms::PlatformSetting{
//               .key = "snes9x_justifier1_crosshair", .defaultValue = "4"},
//           firelight::platforms::PlatformSetting{
//               .key = "snes9x_justifier1_color", .defaultValue = "Blue"},
//           firelight::platforms::PlatformSetting{
//               .key = "snes9x_justifier2_crosshair", .defaultValue = "4"},
//           firelight::platforms::PlatformSetting{
//               .key = "snes9x_justifier2_color", .defaultValue = "Pink"},
//           firelight::platforms::PlatformSetting{.key =
//           "snes9x_rifle_crosshair",
//                                                 .defaultValue = "2"},
//           firelight::platforms::PlatformSetting{.key = "snes9x_rifle_color",
//                                                 .defaultValue = "White"},
//           firelight::platforms::PlatformSetting{
//               .key = "snes9x_block_invalid_vram_access",
//               .defaultValue = "enabled"},
//           firelight::platforms::PlatformSetting{
//               .key = "snes9x_echo_buffer_hack", .defaultValue = "disabled"},
//           firelight::platforms::PlatformSetting{.key = "snes9x_blargg",
//                                                 .defaultValue = "disabled"},
//       }};
//   const auto &service = firelight::platforms::PlatformService::getInstance();
//   const auto actual = service.getPlatform(
//       firelight::platforms::PlatformService::PLATFORM_ID_SNES);
//   EXPECT_TRUE(actual.has_value());
//   assertPlatformsEqual(actual.value(), expected);
// }
//
// TEST_F(PlatformServiceTest, PlatformN64IsCorrect) {
//   const auto expected =
//       firelight::platforms::
//           Platform{.id =
//           firelight::platforms::PlatformService::PLATFORM_ID_N64,
//                    .name = "Nintendo 64",
//                    .abbreviation = "N64",
//                    .slug = "n64",
//                    .fileAssociations = {"n64", "z64", "v64"},
//                    .controllerTypes =
//                        {{.id = 1,
//                          .name = "Retropad",
//                          .imageUrl = "qrc:/images/controllers/n64",
//                          .inputs =
//                              {
//                                  {"A",
//                                   firelight::input::GamepadInput::SouthFace},
//                                  {"B",
//                                   firelight::input::GamepadInput::WestFace},
//                                  {"Z",
//                                   firelight::input::GamepadInput::LeftTrigger},
//                                  {"L",
//                                   firelight::input::GamepadInput::LeftBumper},
//                                  {"R",
//                                   firelight::input::GamepadInput::RightBumper},
//                                  {"Start",
//                                  firelight::input::GamepadInput::Start},
//                                  {"Select",
//                                   firelight::input::GamepadInput::Select},
//                                  {"C-Up",
//                                  firelight::input::GamepadInput::RightStickUp},
//                                  {"C-Down",
//                                   firelight::input::GamepadInput::RightStickDown},
//                                  {"C-Left",
//                                   firelight::input::GamepadInput::RightStickLeft},
//                                  {"C-Right", firelight::
//                                                  input::
//                                                      GamepadInput::RightStickRight},
//                                  {"D-Pad Up", firelight::
//                                                   input::GamepadInput::DpadUp},
//                                  {"D-Pad Down", firelight::
//                                                     input::
//                                                         GamepadInput::DpadDown},
//                                  {"D-Pad Left", firelight::
//                                                     input::
//                                                         GamepadInput::DpadLeft},
//                                  {"D-Pad Right", firelight::
//                                                      input::
//                                                          GamepadInput::DpadRight},
//                                  {"Analog Stick Up", firelight::
//                                                          input::
//                                                              GamepadInput::LeftStickUp},
//                                  {"Analog Stick Down", firelight::
//                                                            input::
//                                                                GamepadInput::LeftStickDown},
//                                  {"Analog Stick Left", firelight::
//                                                            input::
//                                                                GamepadInput::LeftStickLeft},
//                                  {"Analog Stick Right", firelight::
//                                                             input::
//                                                                 GamepadInput::LeftStickRight},
//                              }}},
//                    .settings = {
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-169screensize",
//                            .defaultValue = "960x540"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-43screensize",
//                            .defaultValue = "1280x960"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-BackgroundMode",
//                            .defaultValue = "OnePiece"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-BilinearMode",
//                            .defaultValue = "standard"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-CorrectTexrectCoords",
//                            .defaultValue = "Off"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-CountPerOp",
//                            .defaultValue = "0"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-DitheringPattern",
//                            .defaultValue = "False"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-DitheringQuantization",
//                            .defaultValue = "False"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-EnableCopyAuxToRDRAM",
//                            .defaultValue = "False"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-EnableCopyColorToRDRAM",
//                            .defaultValue = "Async"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-EnableCopyDepthToRDRAM",
//                            .defaultValue = "Software"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-EnableEnhancedHighResStorage",
//                            .defaultValue = "False"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-EnableEnhancedTextureStorage",
//                            .defaultValue = "False"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-EnableFBEmulation",
//                            .defaultValue = "True"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-EnableFragmentDepthWrite",
//                            .defaultValue = "True"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-EnableHWLighting",
//                            .defaultValue = "False"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-EnableHiResAltCRC",
//                            .defaultValue = "False"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-EnableLODEmulation",
//                            .defaultValue = "True"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-EnableLegacyBlending",
//                            .defaultValue = "False"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-EnableN64DepthCompare",
//                            .defaultValue = "False"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-EnableNativeResFactor",
//                            .defaultValue = "0"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-EnableNativeResTexrects",
//                            .defaultValue = "Disabled"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-EnableOverscan",
//                            .defaultValue = "Enabled"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-EnableShadersStorage",
//                            .defaultValue = "True"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-EnableTexCoordBounds",
//                            .defaultValue = "False"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-EnableTextureCache",
//                            .defaultValue = "True"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-FXAA", .defaultValue = "0"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-ForceDisableExtraMem",
//                            .defaultValue = "False"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-FrameDuping",
//                            .defaultValue = "False"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-Framerate",
//                            .defaultValue = "Original"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-GLideN64IniBehaviour",
//                            .defaultValue = "late"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-HybridFilter",
//                            .defaultValue = "True"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-IgnoreTLBExceptions",
//                            .defaultValue = "False"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-MaxTxCacheSize",
//                            .defaultValue = "8000"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-MultiSampling",
//                            .defaultValue = "0"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-OverscanBottom",
//                            .defaultValue = "0"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-OverscanLeft",
//                            .defaultValue = "0"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-OverscanRight",
//                            .defaultValue = "0"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-OverscanTop",
//                            .defaultValue = "0"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-RDRAMImageDitheringMode",
//                            .defaultValue = "False"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-ThreadedRenderer",
//                            .defaultValue = "False"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-alt-map",
//                            .defaultValue = "False"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-angrylion-multithread",
//                            .defaultValue = "all threads"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-angrylion-overscan",
//                            .defaultValue = "disabled"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-angrylion-sync",
//                            .defaultValue = "Low"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-angrylion-vioverlay",
//                            .defaultValue = "Filtered"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-aspect", .defaultValue =
//                            "4:3"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-astick-deadzone",
//                            .defaultValue = "15"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-astick-sensitivity",
//                            .defaultValue = "100"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-cpucore",
//                            .defaultValue = "dynamic_recompiler"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-d-cbutton",
//                            .defaultValue = "C3"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-l-cbutton",
//                            .defaultValue = "C2"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-pak1", .defaultValue =
//                            "memory"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-pak2", .defaultValue =
//                            "none"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-pak3", .defaultValue =
//                            "none"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-pak4", .defaultValue =
//                            "none"},
//                        firelight::platforms::PlatformSetting{
//                            .key =
//                            "mupen64plus-parallel-rdp-deinterlace-method",
//                            .defaultValue = "Bob"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-parallel-rdp-dither-filter",
//                            .defaultValue = "True"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-parallel-rdp-divot-filter",
//                            .defaultValue = "True"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-parallel-rdp-downscaling",
//                            .defaultValue = "disable"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-parallel-rdp-gamma-dither",
//                            .defaultValue = "True"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-parallel-rdp-native-tex-rect",
//                            .defaultValue = "True"},
//                        firelight::platforms::PlatformSetting{
//                            .key =
//                            "mupen64plus-parallel-rdp-native-texture-lod",
//                            .defaultValue = "False"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-parallel-rdp-overscan",
//                            .defaultValue = "0"},
//                        firelight::platforms::PlatformSetting{
//                            .key =
//                            "mupen64plus-parallel-rdp-super-sampled-read-"
//                                   "back",
//                            .defaultValue = "False"},
//                        firelight::platforms::PlatformSetting{
//                            .key =
//                            "mupen64plus-parallel-rdp-super-sampled-read-"
//                                   "back-dither",
//                            .defaultValue = "True"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-parallel-rdp-synchronous",
//                            .defaultValue = "True"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-parallel-rdp-upscaling",
//                            .defaultValue = "1x"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-parallel-rdp-vi-aa",
//                            .defaultValue = "True"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-parallel-rdp-vi-bilinear",
//                            .defaultValue = "True"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-r-cbutton",
//                            .defaultValue = "C1"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-txCacheCompression",
//                            .defaultValue = "True"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-txEnhancementMode",
//                            .defaultValue = "None"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-txFilterIgnoreBG",
//                            .defaultValue = "True"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-txFilterMode",
//                            .defaultValue = "None"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-txHiresEnable",
//                            .defaultValue = "False"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-txHiresFullAlphaChannel",
//                            .defaultValue = "False"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-u-cbutton",
//                            .defaultValue = "C4"},
//                        firelight::platforms::PlatformSetting{
//                            .key = "mupen64plus-virefresh",
//                            .defaultValue = "Auto"},
//                    }};
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
