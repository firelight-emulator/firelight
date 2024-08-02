#pragma once

namespace firelight {
  const static std::map<int, std::map<std::string, std::string> > defaultPlatformValues = {
    {
      // GB
      1, {
        {"gambatte_gbc_color_correction", "GBC only"},
        {"gambatte_gbc_color_correction_mode", "accurate"},
        {"gambatte_gbc_frontlight_position", "central"},
        {"gambatte_dark_filter_level", "0"},
        {"gambatte_up_down_allowed", "disabled"},
        {"gambatte_mix_frames", "disabled"},
        {"gambatte_gb_colorization", "auto"},
        {"gambatte_gb_internal_palette", "GB - DMG"},
        {"gambatte_gb_bootloader", "disabled"},
        {"gambatte_gb_hwmode", "auto"}
      }
    },
    // GBC
    {
      2, {
        {"gambatte_gbc_color_correction", "GBC only"},
        {"gambatte_gbc_color_correction_mode", "accurate"},
        {"gambatte_gbc_frontlight_position", "central"},
        {"gambatte_dark_filter_level", "0"},
        {"gambatte_up_down_allowed", "disabled"},
        {"gambatte_mix_frames", "disabled"},
        {"gambatte_gb_colorization", "auto"},
        {"gambatte_gb_internal_palette", "GB - DMG"},
        {"gambatte_gb_bootloader", "disabled"},
        {"gambatte_gb_hwmode", "auto"}
      }
    },
    // GBA
    {
      3, {
        {"mgba_solar_sensor_level", "0"},
        {"mgba_gb_model", "Autodetect"},
        {"mgba_sgb_borders", "ON"},
        {"mgba_gb_colors_preset", "0"},
        {"mgba_gb_colors", "Grayscale"},
        {"mgba_use_bios", "OFF"},
        {"mgba_skip_bios", "ON"},
        {"mgba_sgb_borders", "ON"},
        {"mgba_frameskip", "disabled"},
        {"mgba_frameskip_threshold", "33"},
        {"mgba_frameskip_interval", "0"},
        {"mgba_audio_low_pass_filter", "disabled"},
        {"mgba_audio_low_pass_range", "60"},
        {"mgba_idle_optimization", "Remove Known"},
        {"mgba_force_gbp", "OFF"},
        {"mgba_color_correction", "OFF"},
        {"mgba_interframe_blending", "mix_smart"}
      }
    },
    // NES
    {
      5, {
        {"fceumm_apu_1", "enabled"},
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
        {"fceumm_zapper_mode", "clightgun"},
        {"fceumm_zapper_sensor", "enabled"},
        {"fceumm_zapper_tolerance", "6"},
        {"fceumm_zapper_trigger", "enabled"},
        {"fceumm_up_down_allowed", "disabled"}
      }
    },
    // SNES
    {
      6, {
        {"snes9x_hires_blend", "disabled"},
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
        {"snes9x_blargg", "disabled"}
      }
    },
    // N64
    {
      7, {
        {"mupen64plus-169screensize", "960x540"},
        {"mupen64plus-43screensize", "640x480"},
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
        {"mupen64plus-rdp-plugin", "gliden64"},
        {"mupen64plus-rsp-plugin", "hle"},
        {"mupen64plus-txCacheCompression", "True"},
        {"mupen64plus-txEnhancementMode", "None"},
        {"mupen64plus-txFilterIgnoreBG", "True"},
        {"mupen64plus-txFilterMode", "None"},
        {"mupen64plus-txHiresEnable", "False"},
        {"mupen64plus-txHiresFullAlphaChannel", "False"},
        {"mupen64plus-u-cbutton", "C4"},
        {"mupen64plus-virefresh", "Auto"}
      }
    }
  };

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
    static constexpr int PLATFORM_ID_SEGA_GENESIS = 13;

    static std::optional<std::string> getDefaultConfigValue(int platformId, std::string key) {
      if (defaultPlatformValues.find(platformId) == defaultPlatformValues.end()) {
        return std::nullopt;
      }

      if (defaultPlatformValues.at(platformId).find(key) == defaultPlatformValues.at(platformId).end()) {
        return std::nullopt;
      }

      return defaultPlatformValues.at(platformId).at(key);
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
      if (extension == "md") {
        return PLATFORM_ID_SEGA_GENESIS;
      }
      return PLATFORM_ID_UNKNOWN;
    }

    static std::string getCoreDllPath(const int platformId) {
      switch (platformId) {
        case PLATFORM_ID_GAMEBOY:
        case PLATFORM_ID_GAMEBOY_COLOR:
          return "./system/_cores/gambatte_libretro.dll";
        case PLATFORM_ID_GAMEBOY_ADVANCE:
          return "./system/_cores/mgba_libretro.dll";
        case PLATFORM_ID_NES:
          return "./system/_cores/fceumm_libretro.dll";
        case PLATFORM_ID_SNES:
          return "./system/_cores/snes9x_libretro.dll";
        case PLATFORM_ID_N64:
          return "./system/_cores/mupen64plus_next_libretro.dll";
        case PLATFORM_ID_NINTENDO_DS:
          return "./system/_cores/melondsds_libretro.dll";
        case PLATFORM_ID_SEGA_GENESIS:
          return "./system/_cores/genesis_plus_gx_libretro.dll";
        default:
          return "";
      }
    }
  };
} // firelight
