#pragma once

#include "input/gamepad_input.hpp"
#include "input/platform_input_descriptor.hpp"

#include <firelight/libretro/retropad.hpp>
#include <map>

namespace firelight {
const static std::map<int, std::map<std::string, std::string>>
    defaultPlatformValues = {
        {// GB
         1,
         {{"gambatte_gbc_color_correction", "GBC only"},
          {"gambatte_gbc_color_correction_mode", "accurate"},
          {"gambatte_gbc_frontlight_position", "central"},
          {"gambatte_dark_filter_level", "0"},
          {"gambatte_up_down_allowed", "disabled"},
          {"gambatte_mix_frames", "disabled"},
          {"gambatte_gb_colorization", "auto"},
          {"gambatte_gb_internal_palette", "GB - DMG"},
          {"gambatte_gb_bootloader", "disabled"},
          {"gambatte_gb_hwmode", "auto"}}},
        // GBC
        {2,
         {{"gambatte_gbc_color_correction", "GBC only"},
          {"gambatte_gbc_color_correction_mode", "accurate"},
          {"gambatte_gbc_frontlight_position", "central"},
          {"gambatte_dark_filter_level", "0"},
          {"gambatte_up_down_allowed", "disabled"},
          {"gambatte_mix_frames", "disabled"},
          {"gambatte_gb_colorization", "auto"},
          {"gambatte_gb_internal_palette", "GB - DMG"},
          {"gambatte_gb_bootloader", "disabled"},
          {"gambatte_gb_hwmode", "auto"}}},
        // GBA
        {3,
         {{"mgba_solar_sensor_level", "0"},
          {"mgba_gb_model", "Autodetect"},
          {"mgba_sgb_borders", "ON"},
          {"mgba_gb_colors_preset", "0"},
          {"mgba_gb_colors", "Grayscale"},
          {"mgba_use_bios", "ON"},
          {"mgba_skip_bios", "OFF"},
          {"mgba_sgb_borders", "ON"},
          {"mgba_frameskip", "disabled"},
          {"mgba_frameskip_threshold", "33"},
          {"mgba_frameskip_interval", "0"},
          {"mgba_audio_low_pass_filter", "disabled"},
          {"mgba_audio_low_pass_range", "60"},
          {"mgba_idle_optimization", "Remove Known"},
          {"mgba_force_gbp", "OFF"},
          {"mgba_color_correction", "OFF"},
          {"mgba_interframe_blending", "mix_smart"}}},
        // NES
        {5,
         {{"fceumm_apu_1", "enabled"},
          {"fceumm_apu_2", "enabled"},
          {"fceumm_apu_3", "enabled"},
          {"fceumm_apu_4", "enabled"},
          {"fceumm_apu_5", "enabled"},
          {"fceumm_arkanoid_mode", "mouse"},
          {"fceumm_aspect", "8:7 PAR"}, // TODO: Pixel perfect?
          {"fceumm_game_genie", "disabled"},
          {"fceumm_mouse_sensitivity", "100"},
          {"fceumm_nospritelimit", "disabled"},
          {"fceumm_ntsc_filter", "disabled"},
          {"fceumm_overclocking", "disabled"},
          {"fceumm_overscan_h_left", "0"},
          {"fceumm_overscan_h_right", "0"},
          {"fceumm_overscan_v_bottom", "8"},
          {"fceumm_overscan_v_top", "8"},
          {"fceumm_palette", "default"},
          {"fceumm_ramstate", "$ff"},
          {"fceumm_show_adv_sound_options", "disabled"},
          {"fceumm_show_adv_system_options", "disabled"},
          {"fceumm_show_crosshair", "enabled"},
          {"fceumm_sndlowpass", "disabled"},
          {"fceumm_sndquality", "Very High"},
          {"fceumm_sndstereodelay", "15_ms_delay"},
          {"fceumm_sndvolume", "7"},
          {"fceumm_swapduty", "disabled"},
          {"fceumm_turbo_delay", "3"},
          {"fceumm_turbo_enable", "None"},
          {"fceumm_up_down_allowed", "disabled"},
          {"fceumm_zapper_mode", "mouse"},
          {"fceumm_zapper_sensor", "enabled"},
          {"fceumm_zapper_tolerance", "6"},
          {"fceumm_zapper_trigger", "enabled"},
          {"fceumm_up_down_allowed", "disabled"}}},
        // SNES
        {6,
         {{"snes9x_hires_blend", "disabled"},
          {"snes9x_overclock_superfx", "100%"},
          {"snes9x_up_down_allowed", "disabled"},
          {"snes9x_sndchan_1", "enabled"},
          {"snes9x_sndchan_2", "enabled"},
          {"snes9x_sndchan_3", "enabled"},
          {"snes9x_sndchan_4", "enabled"},
          {"snes9x_sndchan_5", "enabled"},
          {"snes9x_sndchan_6", "enabled"},
          {"snes9x_sndchan_7", "enabled"},
          {"snes9x_sndchan_8", "enabled"},
          {"snes9x_layer_1", "enabled"},
          {"snes9x_layer_2", "enabled"},
          {"snes9x_layer_3", "enabled"},
          {"snes9x_layer_4", "enabled"},
          {"snes9x_layer_5", "enabled"},
          {"snes9x_gfx_clip", "enabled"},
          {"snes9x_gfx_transp", "enabled"},
          {"snes9x_audio_interpolation", "gaussian"},
          {"snes9x_overclock_cycles", "disabled"},
          {"snes9x_reduce_sprite_flicker", "disabled"},
          {"snes9x_randomize_memory", "disabled"},
          {"snes9x_overscan", "enabled"},
          {"snes9x_aspect", "4:3"},
          {"snes9x_region", "auto"},
          {"snes9x_lightgun_mode", "Lightgun"},
          {"snes9x_superscope_reverse_buttons", "disabled"},
          {"snes9x_superscope_crosshair", "2"},
          {"snes9x_superscope_color", "White"},
          {"snes9x_justifier1_crosshair", "4"},
          {"snes9x_justifier1_color", "Blue"},
          {"snes9x_justifier2_crosshair", "4"},
          {"snes9x_justifier2_color", "Pink"},
          {"snes9x_rifle_crosshair", "2"},
          {"snes9x_rifle_color", "White"},
          {"snes9x_block_invalid_vram_access", "enabled"},
          {"snes9x_echo_buffer_hack", "disabled"},
          {"snes9x_blargg", "disabled"}}},
        // N64
        {7,
         {{"mupen64plus-169screensize", "960x540"},
          {"mupen64plus-43screensize", "1280x960"},
          {"mupen64plus-BackgroundMode", "OnePiece"},
          {"mupen64plus-BilinearMode", "standard"},
          {"mupen64plus-CorrectTexrectCoords", "Off"},
          {"mupen64plus-CountPerOp", "0"},
          {"mupen64plus-DitheringPattern", "False"},
          {"mupen64plus-DitheringQuantization", "False"},
          {"mupen64plus-EnableCopyAuxToRDRAM", "False"},
          {"mupen64plus-EnableCopyColorToRDRAM", "Async"},
          {"mupen64plus-EnableCopyDepthToRDRAM", "Software"},
          {"mupen64plus-EnableEnhancedHighResStorage", "False"},
          {"mupen64plus-EnableEnhancedTextureStorage", "False"},
          {"mupen64plus-EnableFBEmulation", "True"},
          {"mupen64plus-EnableFragmentDepthWrite", "True"},
          {"mupen64plus-EnableHWLighting", "False"},
          {"mupen64plus-EnableHiResAltCRC", "False"},
          {"mupen64plus-EnableLODEmulation", "True"},
          {"mupen64plus-EnableLegacyBlending", "False"},
          {"mupen64plus-EnableN64DepthCompare", "False"},
          {"mupen64plus-EnableNativeResFactor", "0"},
          {"mupen64plus-EnableNativeResTexrects", "Disabled"},
          {"mupen64plus-EnableOverscan", "Enabled"},
          {"mupen64plus-EnableShadersStorage", "True"},
          {"mupen64plus-EnableTexCoordBounds", "False"},
          {"mupen64plus-EnableTextureCache", "True"},
          {"mupen64plus-FXAA", "0"},
          {"mupen64plus-ForceDisableExtraMem", "False"},
          {"mupen64plus-FrameDuping", "False"},
          {"mupen64plus-Framerate", "Original"},
          {"mupen64plus-GLideN64IniBehaviour", "late"},
          {"mupen64plus-HybridFilter", "True"},
          {"mupen64plus-IgnoreTLBExceptions", "False"},
          {"mupen64plus-MaxTxCacheSize", "8000"},
          {"mupen64plus-MultiSampling", "0"},
          {"mupen64plus-OverscanBottom", "0"},
          {"mupen64plus-OverscanLeft", "0"},
          {"mupen64plus-OverscanRight", "0"},
          {"mupen64plus-OverscanTop", "0"},
          {"mupen64plus-RDRAMImageDitheringMode", "False"},
          {"mupen64plus-ThreadedRenderer", "False"},
          {"mupen64plus-alt-map", "False"},
          {"mupen64plus-angrylion-multithread", "all threads"},
          {"mupen64plus-angrylion-overscan", "disabled"},
          {"mupen64plus-angrylion-sync", "Low"},
          {"mupen64plus-angrylion-vioverlay", "Filtered"},
          {"mupen64plus-aspect", "4:3"},
          {"mupen64plus-astick-deadzone", "15"},
          {"mupen64plus-astick-sensitivity", "100"},
          {"mupen64plus-cpucore", "dynamic_recompiler"},
          {"mupen64plus-d-cbutton", "C3"},
          {"mupen64plus-l-cbutton", "C2"},
          {"mupen64plus-pak1", "memory"},
          {"mupen64plus-pak2", "none"},
          {"mupen64plus-pak3", "none"},
          {"mupen64plus-pak4", "none"},
          {"mupen64plus-parallel-rdp-deinterlace-method", "Bob"},
          {"mupen64plus-parallel-rdp-dither-filter", "True"},
          {"mupen64plus-parallel-rdp-divot-filter", "True"},
          {"mupen64plus-parallel-rdp-downscaling", "disable"},
          {"mupen64plus-parallel-rdp-gamma-dither", "True"},
          {"mupen64plus-parallel-rdp-native-tex-rect", "True"},
          {"mupen64plus-parallel-rdp-native-texture-lod", "False"},
          {"mupen64plus-parallel-rdp-overscan", "0"},
          {"mupen64plus-parallel-rdp-super-sampled-read-back", "False"},
          {"mupen64plus-parallel-rdp-super-sampled-read-back-dither", "True"},
          {"mupen64plus-parallel-rdp-synchronous", "True"},
          {"mupen64plus-parallel-rdp-upscaling", "1x"},
          {"mupen64plus-parallel-rdp-vi-aa", "True"},
          {"mupen64plus-parallel-rdp-vi-bilinear", "True"},
          {"mupen64plus-r-cbutton", "C1"},
          {"mupen64plus-rdp-plugin", "angrylion"},
          // {"mupen64plus-rsp-plugin", "parallel"},
          {"mupen64plus-txCacheCompression", "True"},
          {"mupen64plus-txEnhancementMode", "None"},
          {"mupen64plus-txFilterIgnoreBG", "True"},
          {"mupen64plus-txFilterMode", "None"},
          {"mupen64plus-txHiresEnable", "False"},
          {"mupen64plus-txHiresFullAlphaChannel", "False"},
          {"mupen64plus-u-cbutton", "C4"},
          {"mupen64plus-virefresh", "Auto"}}},
        {10,
         {{"melonds_homebrew_sdcard", "disabled"},
          // {"melonds_render_mode", "opengl"},
          {"melonds_number_of_screen_layouts", "8"}}}};

class PlatformMetadata {
public:
  static constexpr int PLATFORM_ID_UNKNOWN = -1;
  static constexpr int PLATFORM_ID_GAMEBOY = 1;
  static constexpr int PLATFORM_ID_GAMEBOY_COLOR = 2;
  static constexpr int PLATFORM_ID_GAMEBOY_ADVANCE = 3;
  static constexpr int PLATFORM_ID_NES = 5;
  static constexpr int PLATFORM_ID_SNES = 6;
  static constexpr int PLATFORM_ID_N64 = 7;
  static constexpr int PLATFORM_ID_NINTENDO_DS = 10;
  static constexpr int PLATFORM_ID_SEGA_MASTER_SYSTEM = 12;
  static constexpr int PLATFORM_ID_SEGA_GENESIS = 13;
  static constexpr int PLATFORM_ID_SEGA_GAMEGEAR = 14;
  static constexpr int PLATFORM_ID_SEGA_SATURN = 15;
  static constexpr int PLATFORM_ID_SEGA_32X = 16;
  static constexpr int PLATFORM_ID_SEGA_CD = 17;
  static constexpr int PLATFORM_ID_PS1 = 18;
  static constexpr int PLATFORM_ID_PS2 = 19;
  static constexpr int PLATFORM_ID_PLAYSTATION_PORTABLE = 20;
  static constexpr int PLATFORM_ID_TURBOGRAFX16 = 21;
  static constexpr int PLATFORM_ID_SUPERGRAFX = 22;
  static constexpr int PLATFORM_ID_POKEMON_MINI = 23;
  static constexpr int PLATFORM_ID_WONDERSWAN = 24;
  static constexpr int PLATFORM_ID_SG1000 = 25;
  static constexpr int PLATFORM_ID_NEOGEO_POCKET = 27;

  static constexpr int OS_ID_UNKNOWN = -1;
  static constexpr int OS_ID_WINDOWS = 0;
  static constexpr int OS_ID_MAC = 1;
  static constexpr int OS_ID_LINUX = 2;

#if _WIN32
  static constexpr auto OS_ID = OS_ID_WINDOWS;
#elif __APPLE__
  static constexpr auto OS_ID = OS_ID_MAC;
#elif __linux__
  static constexpr auto OS_ID = OS_ID_LINUX;
#else
  static constexpr auto OS_ID = OS_ID_UNKNOWN;
#endif

  static std::map<std::string, std::string>
  getDefaultConfigValues(const int platformId) {
    if (!defaultPlatformValues.contains(platformId)) {
      return {};
    }

    return defaultPlatformValues.at(platformId);
  }

  static std::optional<std::string>
  getDefaultConfigValue(const int platformId, const std::string &key) {
    if (!defaultPlatformValues.contains(platformId)) {
      return std::nullopt;
    }

    if (!defaultPlatformValues.at(platformId).contains(key)) {
      return std::nullopt;
    }

    return defaultPlatformValues.at(platformId).at(key);
  }

  static std::string getPlatformName(const int platformId) {
    switch (platformId) {
    case PLATFORM_ID_GAMEBOY:
      return "GameBoy";
    case PLATFORM_ID_GAMEBOY_COLOR:
      return "GameBoy Color";
    case PLATFORM_ID_GAMEBOY_ADVANCE:
      return "GameBoy Advance";
    case PLATFORM_ID_NES:
      return "NES/Famicom";
    case PLATFORM_ID_SNES:
      return "SNES/Super Famicom";
    case PLATFORM_ID_N64:
      return "Nintendo 64";
    case PLATFORM_ID_NINTENDO_DS:
      return "Nintendo DS";
    case PLATFORM_ID_SEGA_MASTER_SYSTEM:
      return "Sega Master System";
    case PLATFORM_ID_SEGA_GENESIS:
      return "Sega Genesis/Mega Drive";
    case PLATFORM_ID_SEGA_GAMEGEAR:
      return "Sega GameGear";
    case PLATFORM_ID_PLAYSTATION_PORTABLE:
      return "PlayStation Portable";
    case PLATFORM_ID_TURBOGRAFX16:
      return "PC Engine/TurboGrafx-16";
    case PLATFORM_ID_SUPERGRAFX:
      return "SuperGrafx";
    case PLATFORM_ID_WONDERSWAN:
      return "WonderSwan";
    case PLATFORM_ID_SG1000:
      return "SG-1000";
    case PLATFORM_ID_NEOGEO_POCKET:
      return "NeoGeo Pocket";
    default:
      return "Unknown";
    }
  }

  static int getIdFromFileExtension(const std::string &extension) {
    if (extension == "gb") {
      return PLATFORM_ID_GAMEBOY;
    }
    if (extension == "gbc") {
      return PLATFORM_ID_GAMEBOY_COLOR;
    }
    if (extension == "gba") {
      return PLATFORM_ID_GAMEBOY_ADVANCE;
    }
    if (extension == "nes") {
      return PLATFORM_ID_NES;
    }
    if (extension == "sfc" || extension == "smc") {
      return PLATFORM_ID_SNES;
    }
    if (extension == "n64" || extension == "v64" || extension == "z64") {
      return PLATFORM_ID_N64;
    }
    if (extension == "nds") {
      return PLATFORM_ID_NINTENDO_DS;
    }
    if (extension == "md" || extension == "gen") {
      return PLATFORM_ID_SEGA_GENESIS;
    }
    if (extension == "gg") {
      return PLATFORM_ID_SEGA_GAMEGEAR;
    }
    if (extension == "sms") {
      return PLATFORM_ID_SEGA_MASTER_SYSTEM;
    }
    if (extension == "pce") {
      return PLATFORM_ID_TURBOGRAFX16;
    }
    if (extension == "sgx") {
      return PLATFORM_ID_SUPERGRAFX;
    }
    if (extension == "min") {
      return PLATFORM_ID_POKEMON_MINI;
    }
    if (extension == "ws" || extension == "wsc") {
      return PLATFORM_ID_WONDERSWAN;
    }
    if (extension == "sg") {
      return PLATFORM_ID_SG1000;
    }
    if (extension == "ngp" || extension == "ngc") {
      return PLATFORM_ID_NEOGEO_POCKET;
    }
    return PLATFORM_ID_UNKNOWN;
  }

  static bool isPossibleRomFileExtension(const std::string &extension) {
    return getIdFromFileExtension(extension) != PLATFORM_ID_UNKNOWN;
  }

  static std::string getDiscordLargeImageName(const int platformId) {
    switch (platformId) {
    case PLATFORM_ID_GAMEBOY:
      return "gb";
    case PLATFORM_ID_GAMEBOY_COLOR:
      return "gbc";
    case PLATFORM_ID_GAMEBOY_ADVANCE:
      return "gba";
    case PLATFORM_ID_NES:
      return "nes";
    case PLATFORM_ID_SNES:
      return "snes";
    case PLATFORM_ID_N64:
      return "n64";
    case PLATFORM_ID_NINTENDO_DS:
      return "ds";
    case PLATFORM_ID_SEGA_MASTER_SYSTEM:
      return "sms";
    case PLATFORM_ID_SEGA_GENESIS:
      return "gen";
    case PLATFORM_ID_SEGA_GAMEGEAR:
      return "gg";
    case PLATFORM_ID_PLAYSTATION_PORTABLE:
      return "psp";
    case PLATFORM_ID_TURBOGRAFX16:
      return "pce";
    case PLATFORM_ID_SUPERGRAFX:
      return "sgx";
    case PLATFORM_ID_WONDERSWAN:
      return "ws";
    case PLATFORM_ID_POKEMON_MINI:
      return "pkmn";
    case PLATFORM_ID_SG1000:
      return "sg";
    case PLATFORM_ID_NEOGEO_POCKET:
      return "ngp";
    default:
      return "firelight-logo-white";
    }
  }

  static std::string getCoreDirectoryPath() {
    switch (OS_ID) {
    case OS_ID_WINDOWS:
      return "./system/_cores/windows/";
    case OS_ID_LINUX:
      return "./system/_cores/linux/";
    default:
    case OS_ID_MAC:
      return "";
    }
  }

  static std::string getCoreDllExtension() {
    switch (OS_ID) {
    case OS_ID_WINDOWS:
      return ".dll";
    case OS_ID_MAC:
      return ".dylib";
    case OS_ID_LINUX:
      return ".so";
    default:
      return "";
    }
  }

  static std::string getCoreDllPath(const int platformId) {
    const auto corePath = getCoreDirectoryPath();
    const auto coreExt = getCoreDllExtension();
    auto coreName = "";

    switch (platformId) {
    case PLATFORM_ID_GAMEBOY:
    case PLATFORM_ID_GAMEBOY_COLOR:
      coreName = "gambatte_libretro";
      break;
    case PLATFORM_ID_GAMEBOY_ADVANCE:
      coreName = "mgba_libretro";
      break;
    case PLATFORM_ID_NES:
      coreName = "fceumm_libretro";
      break;
    case PLATFORM_ID_SNES:
      coreName = "snes9x_libretro";
      break;
    case PLATFORM_ID_N64:
      coreName = "mupen64plus_next_libretro";
      break;
    case PLATFORM_ID_NINTENDO_DS:
      coreName = "melondsds_libretro";
      break;
    case PLATFORM_ID_SG1000:
    case PLATFORM_ID_SEGA_GENESIS:
    case PLATFORM_ID_SEGA_GAMEGEAR:
    case PLATFORM_ID_SEGA_MASTER_SYSTEM:
      coreName = "genesis_plus_gx_libretro";
      break;
    case PLATFORM_ID_PLAYSTATION_PORTABLE:
      coreName = "ppsspp_libretro";
      break;
    case PLATFORM_ID_TURBOGRAFX16:
    case PLATFORM_ID_SUPERGRAFX:
      coreName = "mednafen_supergrafx_libretro";
      break;
    case PLATFORM_ID_POKEMON_MINI:
      coreName = "pokemini_libretro";
      break;
    case PLATFORM_ID_WONDERSWAN:
      coreName = "mednafen_wswan_libretro";
      break;
    case PLATFORM_ID_NEOGEO_POCKET:
      coreName = "mednafen_ngp_libretro";
      break;
    default:
      coreName = "";
      break;
    }

    const auto coreDllPath = corePath + coreName + coreExt;
    return coreDllPath;
  }

  static std::string getInputName(const input::GamepadInput input) {
    switch (input) {
    case input::GamepadInput::SouthFace:
      return "South Face";
    case input::GamepadInput::EastFace:
      return "East Face";
    case input::GamepadInput::WestFace:
      return "West Face";
    case input::GamepadInput::NorthFace:
      return "North Face";
    case input::GamepadInput::LeftBumper:
      return "Left Bumper";
    case input::GamepadInput::RightBumper:
      return "Right Bumper";
    case input::GamepadInput::LeftTrigger:
      return "Left Trigger";
    case input::GamepadInput::RightTrigger:
      return "Right Trigger";
    case input::GamepadInput::L3:
      return "L3";
    case input::GamepadInput::R3:
      return "R3";
    case input::GamepadInput::Start:
      return "Start";
    case input::GamepadInput::Select:
      return "Select";
    case input::GamepadInput::DpadUp:
      return "D-Pad Up";
    case input::GamepadInput::DpadDown:
      return "D-Pad Down";
    case input::GamepadInput::DpadLeft:
      return "D-Pad Left";
    case input::GamepadInput::DpadRight:
      return "D-Pad Right";
    case input::GamepadInput::LeftStickUp:
      return "Left Stick Up";
    case input::GamepadInput::LeftStickDown:
      return "Left Stick Down";
    case input::GamepadInput::LeftStickLeft:
      return "Left Stick Left";
    case input::GamepadInput::LeftStickRight:
      return "Left Stick Right";
    case input::GamepadInput::RightStickUp:
      return "Right Stick Up";
    case input::GamepadInput::RightStickDown:
      return "Right Stick Down";
    case input::GamepadInput::RightStickLeft:
      return "Right Stick Left";
    case input::GamepadInput::RightStickRight:
      return "Right Stick Right";
    default:
      return "Unknown";
      return "South Face";
    }
  }
};
} // namespace firelight
