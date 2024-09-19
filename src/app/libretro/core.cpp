#include "core.hpp"

#include "SDL2/SDL.h"
#include "virtual_filesystem.hpp"
#include <cstdarg>
#include <utility>
#include <bits/fs_path.h>

#include <spdlog/spdlog.h>

namespace libretro {
  void log(enum retro_log_level level, const char *fmt, ...) {
    char msg[4096] = {};
    va_list va;
    va_start(va, fmt);
    vsnprintf(msg, sizeof(msg), fmt, va);
    va_end(va);

    msg[std::remove(msg, msg + strlen(msg), '\n') - msg] = 0;
    msg[std::remove(msg, msg + strlen(msg), '\r') - msg] = 0;

    spdlog::info("[Core] {}", msg);
  }

  // Only supports one core at a time for now, but, eh.
  static Core *currentCore;

  static int16_t inputStateCallback(unsigned port, unsigned device,
                                    unsigned index, unsigned id) {
    if (currentCore == nullptr) {
      // TODO: Report some error
      return 0;
    }

    const auto controllerOpt =
        currentCore->getRetropadProvider()->getRetropadForPlayerIndex(port);
    if (!controllerOpt.has_value()) {
      return 0;
    }

    firelight::libretro::IRetroPad *controller = controllerOpt.value();

    if (device == RETRO_DEVICE_ANALOG) {
      if (index == RETRO_DEVICE_INDEX_ANALOG_LEFT) {
        if (id == RETRO_DEVICE_ID_ANALOG_X) {
          return controller->getLeftStickXPosition();
        }
        if (id == RETRO_DEVICE_ID_ANALOG_Y) {
          return controller->getLeftStickYPosition();
        }
      } else if (index == RETRO_DEVICE_INDEX_ANALOG_RIGHT) {
        if (id == RETRO_DEVICE_ID_ANALOG_X) {
          return controller->getRightStickXPosition();
        }
        if (id == RETRO_DEVICE_ID_ANALOG_Y) {
          return controller->getRightStickYPosition();
        }
      }
    } else if (device == RETRO_DEVICE_JOYPAD) {
      return controller->isButtonPressed(
        static_cast<firelight::libretro::IRetroPad::Button>(id));
    }

    return 0;
  }

  static void videoCallback(const void *data, unsigned width, unsigned height,
                            size_t pitch) {
    currentCore->videoReceiver->receive(data, width, height, pitch);
  }

  static bool envCallback(unsigned cmd, void *data) {
    return currentCore->handleEnvironmentCall(cmd, data);
  }

  bool Core::handleEnvironmentCall(unsigned int cmd, void *data) {
    switch (cmd) {
      case RETRO_ENVIRONMENT_SET_ROTATION:
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_ROTATION");
      //    video->setRotation(*(unsigned *)data);
        return true;
      case (3 | 0x800000): {
        environmentCalls.emplace_back(
          "RETRO_ENVIRONMENT_GET_CLEAR_ALL_THREAD_WAITS_CB");
        auto ptr = static_cast<retro_environment_t *>(data);
        *ptr = [](unsigned int cmd, void *data) {
          printf("Calling weirdo callback");
          return true;
        };
        return true;
      }
      case RETRO_ENVIRONMENT_GET_OVERSCAN:
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_OVERSCAN");
        recordPotentialAPIViolation(
          "Using deprecated environment call GET_OVERSCAN");
        *static_cast<bool *>(data) = false;
        return true;
      case RETRO_ENVIRONMENT_GET_CAN_DUPE: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_CAN_DUPE");
        *static_cast<bool *>(data) = true;
        return true;
      }
      case RETRO_ENVIRONMENT_SET_MESSAGE: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_MESSAGE");
        auto ptr = static_cast<retro_message *>(data);
        // TODO: Do something to queue message
        printf("Got message for %d frames: %s\n", ptr->frames, ptr->msg);
        return true;
      }
      case RETRO_ENVIRONMENT_SHUTDOWN:
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SHUTDOWN");
        shutdown = *static_cast<bool *>(data);
        break;
      case RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL:
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL");
        performanceLevel = *static_cast<unsigned *>(data);
        break;
      case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY");
        // if (systemDirectory.empty()) {
        // return false;
        // }

        auto ptr = static_cast<const char **>(data);
        *ptr = R"(C:\Users\alexs\git\firelight\build\system)";
        // *ptr = &systemDirectory[0];
        return true;
      }
      case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_PIXEL_FORMAT");
        auto ptr = static_cast<retro_pixel_format *>(data);
        videoReceiver->setPixelFormat(ptr);
        break;
      }
      case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS");
        auto ptr = static_cast<retro_input_descriptor *>(data);
        // TODO sane default
        for (int i = 0; i < 100; ++i) {
          auto descriptor = ptr[i];
          if (descriptor.description == nullptr) {
            break;
          }

          inputDescriptors.emplace_back(descriptor);

          if (i == 99) {
            recordPotentialAPIViolation("Over 100 input descriptors");
          }
        }
        return true;
      }
      case RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK");
        auto ptr = (retro_keyboard_callback *) data;
        ptr->callback = [](bool down, unsigned keycode, uint32_t character,
                           uint16_t key_modifiers) {
          printf("Calling the keyboard callback\n");
        };
        return true;
      }
      case RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE: {
        environmentCalls.emplace_back(
          "RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE");
        // auto ptr = static_cast<retro_disk_control_callback *>(data);
        // ptr->set_eject_state = nullptr;
        // ptr->get_eject_state = nullptr;
        // ptr->get_image_index = nullptr;
        // ptr->set_image_index = nullptr;
        // ptr->get_num_images = nullptr;
        // ptr->replace_image_index = nullptr;
        // ptr->add_image_index = nullptr;
        return false;
      }
      case RETRO_ENVIRONMENT_SET_HW_RENDER: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_HW_RENDER");
        auto *renderCallback = static_cast<retro_hw_render_callback *>(data);

        renderCallback->context_type = RETRO_HW_CONTEXT_OPENGL_CORE;
        renderCallback->version_major = 4;
        renderCallback->version_minor = 1;

        renderCallback->get_proc_address =
            [](const char *sym) -> retro_proc_address_t {
              return currentCore->videoReceiver->getProcAddress(sym);
            };

        renderCallback->get_current_framebuffer = [] {
          return currentCore->videoReceiver->getCurrentFramebufferId();
        };

        currentCore->videoReceiver->setResetContextFunc(
          renderCallback->context_reset);

        if (renderCallback->context_destroy) {
          printf("context destroy is not null!\n");
          currentCore->videoReceiver->setDestroyContextFunc(
            renderCallback->context_destroy);
          currentCore->destroyContextFunction = renderCallback->context_destroy;
        }

        break;
      }
      case RETRO_ENVIRONMENT_GET_VARIABLE: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_VARIABLE");
        auto ptr = static_cast<retro_variable *>(data);

        auto configProvider = currentCore->m_configurationProvider;
        if (!configProvider) {
          return false;
        }

        auto val = configProvider->getOptionValue(ptr->key);
        if (!val.has_value()) {
          return false;
        }

        ptr->value = val->key.c_str();
        break;
      }

      case RETRO_ENVIRONMENT_SET_VARIABLES: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_VARIABLES");
        auto ptr = static_cast<retro_variable *>(data);

        auto configProvider = currentCore->m_configurationProvider;
        if (!configProvider) {
          return false;
        }

        for (int i = 0; i < 200; ++i) {
          auto opt = ptr[i];
          if (opt.key == nullptr) {
            break;
          }

          firelight::libretro::IConfigurationProvider::Option option;
          option.key = opt.key;
          option.label = opt.value;

          configProvider->registerOption(option);
        }
        break;
      }
      case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE");

        const auto configProvider = currentCore->m_configurationProvider;
        if (!configProvider) {
          *static_cast<bool *>(data) = configProvider->anyOptionValueHasChanged();
        } else {
          *static_cast<bool *>(data) = false;
        }
        break;
      }
      case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME");
        canRunWithNoGame = *static_cast<bool *>(data);
        break;
      }
      case RETRO_ENVIRONMENT_GET_LIBRETRO_PATH: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_LIBRETRO_PATH");
        if (libretroPath.empty()) {
          return false;
        }
        *static_cast<const char **>(data) = &libretroPath[0];
        return true;
      }
      case RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK");
        //    video->setFrameTimeCallback((retro_frame_time_callback
        //    *)data);
        return true;
      }
      case RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK");
        auto ptr = static_cast<retro_audio_callback *>(data);
        ptr->callback = nullptr;
        ptr->set_state = nullptr;
        return false;
      }
      case RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE");
        auto ptr = static_cast<retro_rumble_interface *>(data);
        ptr->set_rumble_state = [](unsigned port, enum retro_rumble_effect effect,
                                   uint16_t strength) {
          const auto con =
              currentCore->getRetropadProvider()->getRetropadForPlayerIndex(port);
          if (!con.has_value()) {
            return true;
          }

          auto controller = con.value();
          if (effect == RETRO_RUMBLE_STRONG) {
            controller->setStrongRumble(strength);
          } else if (effect == RETRO_RUMBLE_WEAK) {
            controller->setWeakRumble(strength);
          }

          return true;
        };
        break;
      }
      case RETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES: {
        environmentCalls.emplace_back(
          "RETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES");
        auto ptr = static_cast<uint64_t *>(data);
        //* Gets a bitmask telling which device type are expected to be
        // * handled properly in a call to retro_input_state_t.
        // * Devices which are not handled or recognized always return
        // * 0 in retro_input_state_t.
        // * Example bitmask: caps = (1 << RETRO_DEVICE_JOYPAD) | (1 <<
        // RETRO_DEVICE_ANALOG).

        *ptr = (1 << RETRO_DEVICE_JOYPAD) | (1 << RETRO_DEVICE_ANALOG);
        return true;
      }
      case RETRO_ENVIRONMENT_GET_SENSOR_INTERFACE: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_SENSOR_INTERFACE");
        auto ptr = static_cast<retro_sensor_interface *>(data);
        ptr->set_sensor_state = nullptr;
        ptr->get_sensor_input = nullptr;
        return false;
      }
      case RETRO_ENVIRONMENT_GET_CAMERA_INTERFACE: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_CAMERA_INTERFACE");

        auto ptr = static_cast<retro_camera_callback *>(data);
        ptr->start = [] {
          printf("Here's where I WOULD start the camera driver\n");
          return false;
        };
        ptr->stop = [] { printf("Here's where I WOULD stop the camera driver\n"); };
        return true;
      }
      case RETRO_ENVIRONMENT_GET_LOG_INTERFACE: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_LOG_INTERFACE");
        auto ptr = static_cast<retro_log_callback *>(data);
        ptr->log = log;
        // return false;
        return true;
      }
      case RETRO_ENVIRONMENT_GET_PERF_INTERFACE: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_PERF_INTERFACE");
        auto ptr = static_cast<retro_perf_callback *>(data);
        // Return current time microseconds (unix epoch?)
        ptr->get_time_usec = [] {
          printf("Getting time usec");
          return static_cast<retro_time_t>(0);
        };
        // Returns a bit-mask of detected CPU features (RETRO_SIMD_*)
        ptr->get_cpu_features = [] {
          uint64_t cpu = 0;
          if (SDL_HasAVX()) {
            cpu |= RETRO_SIMD_AVX;
          }
          if (SDL_HasAVX2()) {
            cpu |= RETRO_SIMD_AVX2;
          }
          if (SDL_HasMMX()) {
            cpu |= RETRO_SIMD_MMX;
          }
          if (SDL_HasSSE()) {
            cpu |= RETRO_SIMD_SSE;
          }
          if (SDL_HasSSE2()) {
            cpu |= RETRO_SIMD_SSE2;
          }
          if (SDL_HasSSE3()) {
            cpu |= RETRO_SIMD_SSE3;
          }
          if (SDL_HasSSE41()) {
            cpu |= RETRO_SIMD_SSE4;
          }
          if (SDL_HasSSE42()) {
            cpu |= RETRO_SIMD_SSE42;
          }
          return cpu;
        };
        /* A simple counter. Usually nanoseconds, but can also be CPU cycles.
         * Can be used directly if desired (when creating a more sophisticated
         * performance counter system).
         * */
        ptr->get_perf_counter = [] { return static_cast<retro_perf_tick_t>(0); };

        ptr->perf_register = [](retro_perf_counter *counter) {
          printf("Registering counter: %s\n", counter->ident);
        };

        ptr->perf_start = [](retro_perf_counter *counter) {
          printf("Starting counter: %s\n", counter->ident);
        };

        ptr->perf_stop = [](retro_perf_counter *counter) {
          printf("Stopping counter: %s\n", counter->ident);
        };

        /* Asks frontend to log and/or display the state of performance
         * counters. Performance counters can always be poked into manually as
         * well.
         */
        ptr->perf_log = [] {
        };

        return false;
      }
      case RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE");
        auto ptr = static_cast<retro_location_callback *>(data);
        ptr->start = nullptr;
        ptr->stop = nullptr;
        ptr->get_position = nullptr;
        ptr->set_interval = nullptr;
        ptr->initialized = nullptr;
        ptr->deinitialized = nullptr;
        return false;
      }
      case RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY: {
        environmentCalls.emplace_back(
          "RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY");
        auto ptr = static_cast<const char **>(data);
        *ptr = &coreAssetsDirectory[0];
        return true;
      }
      case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY");
        // auto ptr = static_cast<const char **>(data);
        // *ptr = R"(C:\Users\alexs\git\firelight\build\system)";
        // *ptr = "./system";
        // *ptr = &saveDirectory[0]; // TODO
        return false;
      }
      case RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO");
        videoReceiver->setSystemAVInfo(static_cast<retro_system_av_info *>(data));
        return true;
      }
      // case RETRO_ENVIRONMENT_SET_PROC_ADDRESS_CALLBACK:
      //   environmentCalls.emplace_back(
      //     "RETRO_ENVIRONMENT_SET_PROC_ADDRESS_CALLBACK");
      //   break;
      case RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO");
        auto ptr = static_cast<retro_subsystem_info *>(data);
        for (int i = 0; i < 100; ++i) {
          auto ssInfo = ptr[i];
          if (ssInfo.desc == nullptr) {
            break;
          }

          subsystemInfo.emplace_back(ssInfo);
          if (i == 99) {
            recordPotentialAPIViolation("Over 100 subsystems");
          }
        }
        return true;
      }
      case RETRO_ENVIRONMENT_SET_CONTROLLER_INFO: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_CONTROLLER_INFO");
        auto ptr = static_cast<retro_controller_info *>(data);


        for (unsigned i = 0; i < 100; ++i) {
          auto info = ptr[i];

          if (!info.types) {
            break;
          }

          for (unsigned j = 0; j < info.num_types; ++j) {
            auto type = info.types[j];
            printf("Type: %d, Value: %s\n", type.id, type.desc);
          }
        }
        // for (unsigned i = 0; i < ptr->num_types; ++i) {
        //   auto info = ptr->types[i];
        //   if (info.desc == nullptr) {
        //     break;
        //   }
        //
        //   controllerInfo.emplace_back(info);
        //   if (i == 100) {
        //     recordPotentialAPIViolation("Over 100 controller infos");
        //   }
        // }
        return true;
      }
      case RETRO_ENVIRONMENT_SET_MEMORY_MAPS: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_MEMORY_MAPS");
        auto ptr = static_cast<retro_memory_map *>(data);
        for (unsigned i = 0; i < ptr->num_descriptors; ++i) {
          if (ptr->descriptors[i].ptr == nullptr) {
            break;
          }
          memoryDescriptors.emplace_back(retro_memory_descriptor{
            ptr->descriptors[i].flags, ptr->descriptors[i].ptr,
            ptr->descriptors[i].offset, ptr->descriptors[i].start,
            ptr->descriptors[i].select, ptr->descriptors[i].disconnect,
            ptr->descriptors[i].len, ptr->descriptors[i].addrspace
          });
        }

        memoryMap.descriptors = &memoryDescriptors[0];
        memoryMap.num_descriptors = ptr->num_descriptors;

        return true;
      }
      case RETRO_ENVIRONMENT_SET_GEOMETRY: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_GEOMETRY");
        retroSystemAVInfo->geometry = *static_cast<retro_game_geometry *>(data);
        videoReceiver->setSystemAVInfo(retroSystemAVInfo);
        //    video->setGameGeometry(&retroSystemAVInfo->geometry);
        return true;
      }
      case RETRO_ENVIRONMENT_GET_USERNAME: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_USERNAME");
        auto ptr = static_cast<const char **>(data);
        *ptr = &username[0];
        return true;
      }
      case RETRO_ENVIRONMENT_GET_LANGUAGE: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_LANGUAGE");
        // TODO: Set by user
        auto ptr = static_cast<retro_language *>(data);
        *ptr = RETRO_LANGUAGE_ENGLISH;
        return true;
      }
      case RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER:
        environmentCalls.emplace_back(
          "RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER");
        break;
      case RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE");
        auto ptr = static_cast<retro_hw_render_interface *>(data);
        ptr->interface_type = RETRO_HW_RENDER_INTERFACE_DUMMY;
        ptr->interface_version = 0;
        return true;
      }
      case RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS");
        supportsAchievements = *static_cast<bool *>(data);
        return true;
      }
      case RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE: {
        environmentCalls.emplace_back(
          "RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE");
        auto ptr = static_cast<retro_hw_render_context_negotiation_interface *>(data);
        auto type = ptr->interface_type;
        auto version = ptr->interface_version;

        break;
      }
      case RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS:
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS");
        spdlog::warn("Ignoring serialization quirks");
        break;
      case RETRO_ENVIRONMENT_SET_HW_SHARED_CONTEXT:
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_HW_SHARED_CONTEXT");
        return true;
      case RETRO_ENVIRONMENT_GET_VFS_INTERFACE: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_VFS_INTERFACE");
        // TODO: Do something here to ensure this was called before we give the
        // core any paths
        auto ptr = static_cast<retro_vfs_interface_info *>(data);
        printf("Required VFS interface version: %d\n",
               ptr->required_interface_version);
        m_vfsInterface.open = vfs::open;
        m_vfsInterface.close = vfs::close;
        m_vfsInterface.size = vfs::size;
        m_vfsInterface.tell = vfs::tell;
        m_vfsInterface.seek = vfs::seek;
        m_vfsInterface.read = vfs::read;
        m_vfsInterface.write = vfs::write;
        m_vfsInterface.flush = vfs::flush;
        m_vfsInterface.remove = vfs::remove;
        m_vfsInterface.rename = vfs::rename;
        m_vfsInterface.truncate = vfs::truncate;
        m_vfsInterface.stat = vfs::stat;
        m_vfsInterface.mkdir = vfs::mkdir;
        m_vfsInterface.opendir = vfs::opendir;
        m_vfsInterface.readdir = vfs::readdir;
        m_vfsInterface.dirent_get_name = vfs::dirent_get_name;
        m_vfsInterface.dirent_is_dir = vfs::dirent_is_dir;
        m_vfsInterface.closedir = vfs::closedir;

        ptr->iface = &m_vfsInterface;
        return false;
      }
      case RETRO_ENVIRONMENT_GET_LED_INTERFACE: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_LED_INTERFACE");
        auto ptr = static_cast<retro_led_interface *>(data);
        ptr->set_led_state = [](int led, int state) {
          printf("Setting LED %d to state %d\n", led, state);
        };
        return true;
      }
      case RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE");
        auto value = static_cast<int *>(data);
        *value = 1 << 0 | 1 << 1;
        return true;
      }
      case RETRO_ENVIRONMENT_GET_MIDI_INTERFACE:
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_MIDI_INTERFACE");
        break;
      case RETRO_ENVIRONMENT_GET_FASTFORWARDING: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_FASTFORWARDING");
        // TODO: Get from video provider?
        auto ptr = static_cast<bool *>(data);
        *ptr = fastforwarding;
        return true;
      }
      case RETRO_ENVIRONMENT_GET_TARGET_REFRESH_RATE:
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_TARGET_REFRESH_RATE");
        break;
      case RETRO_ENVIRONMENT_GET_INPUT_BITMASKS:
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_INPUT_BITMASKS");
      // TODO: Implement
        return false;
      case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION");
        auto ptr = static_cast<unsigned *>(data);
        *ptr = 2;
        break;
      }
      case RETRO_ENVIRONMENT_SET_CORE_OPTIONS: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_CORE_OPTIONS");
        auto ptr = static_cast<retro_core_option_definition **>(data);

        auto configProvider = currentCore->m_configurationProvider;
        if (!configProvider) {
          return false;
        }

        for (int i = 0; i < 200; ++i) {
          auto opt = ptr[i];
          if (opt->key == nullptr) {
            break;
          }

          firelight::libretro::IConfigurationProvider::Option option;
          option.key = opt->key;
          option.label = opt->desc;
          option.description = opt->info;

          if (opt->default_value != nullptr) {
            option.defaultValueKey = opt->default_value;
          } else {
            option.defaultValueKey = opt->values[0].value;
          }

          for (int j = 0; j < 100; ++j) {
            auto val = opt->values[j];
            if (val.value == nullptr) {
              break;
            }

            firelight::libretro::IConfigurationProvider::OptionValue optionValue;
            optionValue.key = val.value;
            if (val.label != nullptr) {
              optionValue.label = val.label;
            } else {
              optionValue.label = val.value;
            }

            option.possibleValues.emplace_back(optionValue);
          }

          configProvider->registerOption(option);
        }
        break;
      }
      case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL");
        auto ptr = static_cast<retro_core_options_intl *>(data);

        auto configProvider = currentCore->m_configurationProvider;
        if (!configProvider) {
          return false;
        }

        for (int i = 0; i < 200; ++i) {
          auto opt = ptr->us[i];
          if (opt.key == nullptr) {
            break;
          }

          firelight::libretro::IConfigurationProvider::Option option;
          option.key = opt.key;
          option.label = opt.desc;
          option.description = opt.info;

          if (opt.default_value != nullptr) {
            option.defaultValueKey = opt.default_value;
          } else {
            option.defaultValueKey = opt.values[0].value;
          }

          for (int j = 0; j < 100; ++j) {
            auto val = opt.values[j];
            if (val.value == nullptr) {
              break;
            }

            firelight::libretro::IConfigurationProvider::OptionValue optionValue;
            optionValue.key = val.value;
            if (val.label != nullptr) {
              optionValue.label = val.label;
            } else {
              optionValue.label = val.value;
            }

            option.possibleValues.emplace_back(optionValue);
          }

          configProvider->registerOption(option);
        }


        break;
      }
      case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY");
        auto ptr = static_cast<retro_core_option_display *>(data);

        auto configProvider = currentCore->m_configurationProvider;
        if (!configProvider) {
          return false;
        }

        configProvider->setOptionVisibility(ptr->key, ptr->visible);
        break;
      }
      case RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER:
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER");
        *static_cast<unsigned *>(data) = RETRO_HW_CONTEXT_OPENGL;
        return true;
      // case RETRO_ENVIRONMENT_GET_DISK_CONTROL_INTERFACE_VERSION:
      //   environmentCalls.emplace_back(
      //     "RETRO_ENVIRONMENT_GET_DISK_CONTROL_INTERFACE_VERSION");
      //   return false;
      // case RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE:
      //   environmentCalls.emplace_back(
      //     "RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE");
      //   return false;
      case RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION: {
        environmentCalls.emplace_back(
          "RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION");
        auto ptr = static_cast<unsigned *>(data);
        *ptr = 1;
        break;
      }
      case RETRO_ENVIRONMENT_SET_MESSAGE_EXT: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_MESSAGE_EXT");
        auto ptr = static_cast<retro_message_ext *>(data);

        // TODO
        printf("Msg: %s\n", ptr->msg);
        break;
      }
      // case RETRO_ENVIRONMENT_GET_INPUT_MAX_USERS:
      //   environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_INPUT_MAX_USERS");
      //   return false;
      case RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK: {
        environmentCalls.emplace_back(
          "RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK");
        if (!data) {
          break;
        }

        auto ptr = static_cast<retro_audio_buffer_status_callback *>(data);

        ptr->callback = [](bool active, unsigned occupancy, bool underrun_likely) {
          printf("Active: %d, Occupancy: %d, Underrun Likely: %d\n", active, occupancy, underrun_likely);
        };

        break;
      }
      case RETRO_ENVIRONMENT_SET_MINIMUM_AUDIO_LATENCY: {
        environmentCalls.emplace_back(
          "RETRO_ENVIRONMENT_SET_MINIMUM_AUDIO_LATENCY");
        // TODO
        break;
      }
      // case RETRO_ENVIRONMENT_SET_FASTFORWARDING_OVERRIDE:
      //   environmentCalls.emplace_back(
      //     "RETRO_ENVIRONMENT_SET_FASTFORWARDING_OVERRIDE");
      //   break;
      case RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDE: {
        environmentCalls.emplace_back(
          "RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDE");
        auto ptr = static_cast<retro_system_content_info_override *>(data);
        for (int i = 0; i < 100; ++i) {
          auto info = ptr[i];
          if (info.extensions == nullptr) {
            break;
          }
        }
        return true;
        // break;
      }
      case RETRO_ENVIRONMENT_GET_GAME_INFO_EXT: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_GAME_INFO_EXT");
        auto ptr = static_cast<retro_game_info_ext **>(data);
        *ptr = new retro_game_info_ext();

        auto filename = std::filesystem::path(game->getPath());

        (*ptr)->file_in_archive = false;
        (*ptr)->archive_file = nullptr;
        (*ptr)->archive_path = nullptr;
        (*ptr)->meta = "";
        (*ptr)->persistent_data = false;
        (*ptr)->dir = R"()";
        (*ptr)->full_path = strdup(filename.filename().string().c_str());
        (*ptr)->ext = strdup(filename.extension().string().substr(1).c_str());
        (*ptr)->name = strdup(filename.stem().string().c_str());
        (*ptr)->data = game->getData();
        (*ptr)->size = game->getSize();

        return true;
      }
      case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2");
        auto ptr = static_cast<retro_core_options_v2 *>(data);

        auto configProvider = currentCore->m_configurationProvider;
        if (!configProvider) {
          return false;
        }

        for (int i = 0; i < 100; ++i) {
          auto opt = ptr->categories[i];
          if (opt.key == nullptr) {
            break;
          }
        }

        for (int i = 0; i < 200; ++i) {
          auto opt = ptr->definitions[i];
          if (opt.key == nullptr) {
            break;
          }

          firelight::libretro::IConfigurationProvider::Option option;
          option.key = opt.key;

          if (opt.default_value != nullptr) {
            option.defaultValueKey = opt.default_value;
          } else {
            option.defaultValueKey = opt.values[0].value;
          }

          if (opt.desc_categorized != nullptr) {
            option.label = opt.desc_categorized;
          } else {
            option.label = opt.desc;
          }

          if (opt.info_categorized != nullptr) {
            option.description = opt.info_categorized;
          } else {
            option.description = opt.info;
          }

          for (int j = 0; j < 100; ++j) {
            auto val = opt.values[j];
            if (val.value == nullptr) {
              break;
            }

            firelight::libretro::IConfigurationProvider::OptionValue optionValue;
            optionValue.key = val.value;
            if (val.label != nullptr) {
              optionValue.label = val.label;
            } else {
              optionValue.label = val.value;
            }

            option.possibleValues.emplace_back(optionValue);
          }

          configProvider->registerOption(option);
        }
        break;
      }
      case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL");
        // TODO
        auto ptr = static_cast<retro_core_options_v2_intl *>(data);

        auto configProvider = currentCore->m_configurationProvider;
        if (!configProvider) {
          return false;
        }

        for (int i = 0; i < 200; ++i) {
          auto opt = ptr->us->definitions[i];
          if (opt.key == nullptr) {
            break;
          }

          firelight::libretro::IConfigurationProvider::Option option;
          option.key = opt.key;

          if (opt.default_value != nullptr) {
            option.defaultValueKey = opt.default_value;
          } else {
            option.defaultValueKey = opt.values[0].value;
          }

          if (opt.desc_categorized != nullptr) {
            option.label = opt.desc_categorized;
          } else if (opt.desc != nullptr) {
            option.label = opt.desc;
          } else {
            option.label = opt.key;
          }

          if (opt.info_categorized != nullptr) {
            option.description = opt.info_categorized;
          } else if (opt.info != nullptr) {
            option.description = opt.info;
          }

          for (int j = 0; j < 100; ++j) {
            auto val = opt.values[j];
            if (val.value == nullptr) {
              break;
            }

            firelight::libretro::IConfigurationProvider::OptionValue optionValue;
            optionValue.key = val.value;
            if (val.label != nullptr) {
              optionValue.label = val.label;
            } else {
              optionValue.label = val.value;
            }

            option.possibleValues.emplace_back(optionValue);
          }

          configProvider->registerOption(option);
        }
        break;
      }
      case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK: {
        environmentCalls.emplace_back(
          "RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK");
        auto ptr = static_cast<retro_core_options_update_display_callback *>(data);
        ptr->callback = []() {
          return true; // TODO I think I actually need to store the callback
          // instead lol
        };
        return true;
      }
      case RETRO_ENVIRONMENT_SET_VARIABLE: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_VARIABLE");
        // TODO: Implement
        break;
      }
      case RETRO_ENVIRONMENT_GET_THROTTLE_STATE: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_THROTTLE_STATE");
        auto ptr = static_cast<retro_throttle_state *>(data);
        /* During normal operation. Rate will be equal to the core's internal
         * FPS.
         */
#define RETRO_THROTTLE_NONE 0

        /* While paused or stepping single frames. Rate will be 0. */
#define RETRO_THROTTLE_FRAME_STEPPING 1

        /* During fast forwarding.
         * Rate will be 0 if not specifically limited to a maximum speed. */
#define RETRO_THROTTLE_FAST_FORWARD 2

        /* During slow motion. Rate will be less than the core's internal FPS.
         */
#define RETRO_THROTTLE_SLOW_MOTION 3

        /* While rewinding recorded save states. Rate can vary depending on the
         * rewind speed or be 0 if the frontend is not aiming for a specific
         * rate.
         */
#define RETRO_THROTTLE_REWINDING 4

        /* While vsync is active in the video driver and the target refresh
         * rate is lower than the core's internal FPS. Rate is the target
         * refresh rate. */
#define RETRO_THROTTLE_VSYNC 5

        /* When the frontend does not throttle in any way. Rate will be 0.
         * An example could be if no vsync or audio output is active. */
#define RETRO_THROTTLE_UNBLOCKED 6
        ptr->mode = 0;
        ptr->rate = 0.0;
        return false;
      }
      case RETRO_ENVIRONMENT_GET_SAVESTATE_CONTEXT: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_SAVESTATE_CONTEXT");
        auto ptr = static_cast<retro_savestate_context *>(data);
        *ptr = RETRO_SAVESTATE_CONTEXT_NORMAL;
        return true;
      }
      case RETRO_ENVIRONMENT_GET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_SUPPORT:
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_HW_RENDER_CONTEXT_"
          "NEGOTIATION_INTERFACE_SUPPORT");
        break;
      case RETRO_ENVIRONMENT_GET_JIT_CAPABLE: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_JIT_CAPABLE");
        auto ptr = (bool *) data;
        *ptr = false; // TODO
        return false;
      }
      case RETRO_ENVIRONMENT_GET_MICROPHONE_INTERFACE: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_MICROPHONE_INTERFACE");
        auto ptr = static_cast<retro_microphone_interface *>(data);
        ptr->interface_version = 0;
        return false;
      }
      case RETRO_ENVIRONMENT_SET_NETPACKET_INTERFACE:
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_NETPACKET_INTERFACE");
        break;
      case RETRO_ENVIRONMENT_GET_DEVICE_POWER: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_DEVICE_POWER");
        //                auto ptr = (retro_device_power *) softwareBufData;
        return false;
      }
      default:
        printf("Unimplemented env command: %d\n", cmd);
        environmentCalls.emplace_back("UNIMPLEMENTED");
        return false;
    }

    return true;
  }

  template<typename T>
  static T loadRetroFunc(void *dll, const char *name) {
    // TODO error checking
    auto result = reinterpret_cast<T>(SDL_LoadFunction(dll, name));
    if (result == nullptr) {
      // std::cout << SDL_GetError() << std::endl;
    }
    return result;
  }

  //    std::basic_string<char> Core::dumpJson() {
  //        json j;
  ////        j["rotation"] = rotation;
  ////        j["overscan"] = useOverscan;
  ////        j["can_dupe_frames"] = canDupeFrames;
  //        // set message
  //        j["should_shutdown"] = shutdown;
  //        j["performance_level"] = performanceLevel;
  //        j["system_directory"] = systemDirectory;
  ////        j["pixel_format"] = *pixelFormat;
  //        j["supports_no_game"] = canRunWithNoGame;
  ////        j["libretro_path"] = libretroPath;
  ////        j["core_assets_directory"] = coreAssetsDirectory;
  ////        j["save_directory"] = saveDirectory;
  //
  //        json opts = json::array();
  //        for (auto o: options) {
  //            opts.emplace_back(o.dumpJson());
  //        }
  //        j["options"] = opts;
  //
  //        json inDesc = json::array();
  //        for (auto i: inputDescriptors) {
  //            json in;
  //            in["id"] = i.id;
  //            in["index"] = i.index;
  //            in["device"] = i.device;
  //            in["description"] = i.description;
  //            in["port"] = i.port;
  //            inDesc.emplace_back(in);
  //        }
  //
  //        j["input_descriptors"] = inDesc;
  //
  //        json subsys = json::array();
  //        for (auto sub: subsystemInfo) {
  //            json ssInfo;
  //            ssInfo["id"] = sub.id;
  //            ssInfo["description"] = sub.desc;
  //            ssInfo["identifier"] = sub.ident;
  //            ssInfo["num_roms"] = sub.num_roms;
  //            // TODO: ROMS
  //
  //            subsys.emplace_back(ssInfo);
  //        }
  //
  //        j["subsystem_info"] = subsys;
  //
  ////        if (hardwareRenderCallback) {
  ////            json hwRender;
  ////            hwRender["context_type"] =
  /// hardwareRenderCallback->context_type; /            hwRender["depth"]
  /// = hardwareRenderCallback->depth; /            hwRender["stencil"] =
  /// hardwareRenderCallback->stencil; / hwRender["bottom_left_origin"] =
  /// hardwareRenderCallback->bottom_left_origin; /
  /// hwRender["version_major"] = hardwareRenderCallback->version_major; /
  /// hwRender["version_minor"] = hardwareRenderCallback->version_minor; /
  /// hwRender["cache_context"] = hardwareRenderCallback->cache_context; /
  /// hwRender["debug_context"] = hardwareRenderCallback->debug_context;
  ////
  ////            j["hw_render_callback"] = hwRender;
  ////        }
  //
  //        json memMaps = json::array();
  //        for (auto m: memoryMaps) {
  //            json map;
  //            map["flags"] = m.flags;
  //            if (m.ptr != nullptr) {
  //                map["ptr"] = (uintptr_t) m.ptr;
  //            }
  //            map["offset"] = m.offset;
  //            map["start"] = m.start;
  //            map["select"] = m.select;
  //            map["disconnect"] = m.disconnect;
  //            map["len"] = m.len;
  //            if (m.addrspace != nullptr) {
  //                map["addrspace"] = m.addrspace;
  //            }
  //
  //            memMaps.emplace_back(map);
  //        }
  //
  //        j["memory_descriptors"] = memMaps;
  //
  //        json envCalls = json::array();
  //        for (auto e: environmentCalls) {
  //            envCalls.emplace_back(e);
  //        }
  //
  //        j["environment_calls"] = envCalls;
  //
  //        return j.dump();
  //    }

  Core::Core(const std::string &libPath,
             std::shared_ptr<firelight::libretro::IConfigurationProvider> configProvider) : m_configurationProvider(
    std::move(configProvider)) {
    coreLib = std::make_unique<QLibrary>(QString::fromStdString(libPath));

    // dll = SDL_LoadObject(libPath.c_str());
    // if (dll == nullptr) {
    // // Check error
    // }

    symRetroInit = coreLib->resolve("retro_init");
    symRetroDeinit = coreLib->resolve("retro_deinit");
    symRetroApiVersion = reinterpret_cast<unsigned int (*)()>(
      coreLib->resolve("retro_api_version"));
    symRetroGetSystemInfo = reinterpret_cast<void (*)(retro_system_info *)>(
      coreLib->resolve("retro_get_system_info"));
    symRetroGetSystemAVInfo = reinterpret_cast<void (*)(retro_system_av_info *)>(
      coreLib->resolve("retro_get_system_av_info"));
    symRetroSetControllerPortDevice =
        reinterpret_cast<void (*)(unsigned int, unsigned int)>(
          coreLib->resolve("retro_set_controller_port_device"));
    symRetroReset = coreLib->resolve("retro_reset");
    symRetroRun = coreLib->resolve("retro_run");
    symRetroSerializeSize =
        reinterpret_cast<size_t (*)()>(coreLib->resolve("retro_serialize_size"));
    symRetroSerialize = reinterpret_cast<bool (*)(void *, size_t)>(
      coreLib->resolve("retro_serialize"));
    symRetroUnserialize = reinterpret_cast<bool (*)(const void *, size_t)>(
      coreLib->resolve("retro_unserialize"));
    symRetroCheatReset =
        coreLib->resolve("retro_cheat_reset");
    symRetroCheatSet = reinterpret_cast<void (*)(unsigned, bool, const char *)>(
      coreLib->resolve("retro_cheat_set"));

    symRetroLoadGame = reinterpret_cast<bool (*)(const retro_game_info *)>(
      coreLib->resolve("retro_load_game"));
    symRetroLoadGameSpecial =
        reinterpret_cast<bool (*)(unsigned int, const retro_game_info *, size_t)>(
          coreLib->resolve("retro_load_game_special"));
    symRetroUnloadGame =
        coreLib->resolve("retro_unload_game");
    symRetroGetRegion = reinterpret_cast<unsigned int (*)()>(
      coreLib->resolve("retro_get_region"));

    symRetroGetMemoryData = reinterpret_cast<void *(*)(unsigned int)>(
      coreLib->resolve("retro_get_memory_data"));
    symRetroGetMemoryDataSize = reinterpret_cast<size_t (*)(unsigned int)>(
      coreLib->resolve("retro_get_memory_size"));

    // symRetroInit = loadRetroFunc<void (*)()>(dll, "retro_init");
    // symRetroDeinit = loadRetroFunc<void (*)()>(dll, "retro_deinit");

    // symRetroApiVersion =
    // loadRetroFunc<unsigned int (*)()>(dll, "retro_api_version");
    // symRetroGetSystemInfo = loadRetroFunc<void (*)(retro_system_info *)>(
    // dll, "retro_get_system_info");
    // symRetroGetSystemAVInfo = loadRetroFunc<void (*)(retro_system_av_info *)>(
    // dll, "retro_get_system_av_info");
    // symRetroSetControllerPortDevice =
    // loadRetroFunc<void (*)(unsigned int, unsigned int)>(
    // dll, "retro_set_controller_port_device");

    // symRetroReset = loadRetroFunc<void (*)()>(dll, "retro_reset");
    // symRetroRun = loadRetroFunc<void (*)()>(dll, "retro_run");
    // symRetroSerializeSize =
    // loadRetroFunc<size_t (*)()>(dll, "retro_serialize_size");
    // symRetroSerialize =
    // loadRetroFunc<bool (*)(void *, size_t)>(dll, "retro_serialize");
    // symRetroUnserialize =
    // loadRetroFunc<bool (*)(const void *, size_t)>(dll,
    // "retro_unserialize");
    // symRetroCheatReset = loadRetroFunc<void (*)()>(dll, "retro_cheat_reset");
    // symRetroCheatSet = loadRetroFunc<void (*)(unsigned, bool, const char *)>(
    // dll, "retro_cheat_set");
    // symRetroLoadGame =
    // loadRetroFunc<bool (*)(const retro_game_info *)>(dll,
    // "retro_load_game");
    // symRetroLoadGameSpecial =
    // loadRetroFunc<bool (*)(unsigned int, const retro_game_info *, size_t)>(
    // dll, "retro_load_game_special");
    // symRetroUnloadGame = loadRetroFunc<void (*)()>(dll, "retro_unload_game");
    // symRetroGetRegion =
    // loadRetroFunc<unsigned int (*)()>(dll, "retro_get_region");
    // symRetroGetMemoryData =
    // loadRetroFunc<void *(*)(unsigned int)>(dll, "retro_get_memory_data");
    // symRetroGetMemoryDataSize =
    // loadRetroFunc<size_t (*)(unsigned int)>(dll, "retro_get_memory_size");

    retroSystemInfo = new retro_system_info;
    retroSystemAVInfo = new retro_system_av_info;

    currentCore = this; // todo prob different namespace

    reinterpret_cast<RetroSetEnvironment>(
      coreLib->resolve("retro_set_environment"))(envCallback);
    reinterpret_cast<RetroSetVideoRefresh>(
      coreLib->resolve("retro_set_video_refresh"))(videoCallback);
    reinterpret_cast<RetroSetAudioSample>(coreLib->resolve(
      "retro_set_audio_sample"))([](int16_t left, int16_t right) {
    });

    // The next several methods load the callback symbols from the library and
    // set them to our methods defined above. Since we never change those
    // callbacks, we don't need to store the symbols.
    // loadRetroFunc<RetroSetEnvironment>(dll,
    // "retro_set_environment")(envCallback);
    // loadRetroFunc<RetroSetVideoRefresh>(dll,
    // "retro_set_video_refresh")(videoCallback);
    // loadRetroFunc<RetroSetAudioSample>(dll, "retro_set_audio_sample")(
    // [](int16_t left, int16_t right) {});

    auto processAudioLambda = [](const int16_t *data, size_t frames) -> size_t {
      auto core = currentCore;
      if (core == nullptr) {
        printf("core was null in libretro audio callback\n");
        return frames;
      }

      return core->audioReceiver->receive(data, frames);
    };

    reinterpret_cast<RetroSetAudioSampleBatch>(
      coreLib->resolve("retro_set_audio_sample_batch"))(processAudioLambda);
    reinterpret_cast<RetroInputPoll>(coreLib->resolve("retro_set_input_poll"))(
      [] {
      });
    reinterpret_cast<RetroInputState>(coreLib->resolve("retro_set_input_state"))(
      inputStateCallback);

    // loadRetroFunc<RetroSetAudioSampleBatch>(dll,
    // "retro_set_audio_sample_batch")(
    // processAudioLambda);
    // loadRetroFunc<RetroInputPoll>(dll, "retro_set_input_poll")([]() {});
    // loadRetroFunc<RetroInputState>(dll,
    // "retro_set_input_state")(inputStateCallback);
  }

  Core::~Core() {
    unloadGame();
    deinit();
    coreLib->unload();
    currentCore = nullptr;
  }

  bool Core::loadGame(Game *game) {
    this->game = game;
    retro_game_info info{};
    // info.path = game->getPath().c_str();
    info.path = R"()";
    info.data = game->getData();
    info.size = game->getSize();
    info.meta = "";

    // spdlog::warn("Path before c_str: {}", game->getPath());
    // spdlog::warn("Path after c_str: {}", string(info.path));
    // TODO: meta?
    auto result = symRetroLoadGame(&info);

    symRetroGetSystemAVInfo(retroSystemAVInfo);
    videoReceiver->setSystemAVInfo(retroSystemAVInfo);
    //  video->setGameGeometry(&retroSystemAVInfo->geometry);

    audioReceiver->initialize(retroSystemAVInfo->timing.sample_rate);
    this->game = nullptr;
    return result;
  }

  void Core::unloadGame() {
    symRetroUnloadGame();
  }

  std::vector<uint8_t> Core::serializeState() const {
    const auto size = symRetroSerializeSize();

    std::vector<uint8_t> data(size);
    if (!symRetroSerialize(data.data(), size)) {
      printf("Some issue??\n");
      return {};
    }

    return data;
  }

  void Core::deserializeState(const std::vector<uint8_t> &data) const {
    const auto size = getSerializeSize();

    if (data.size() != size) {
      printf("um sizes don't match. data: %zu, size: %zu\n", data.size(), size);
    }

    symRetroUnserialize(data.data(), size);
  }

  size_t Core::getSerializeSize() const { return symRetroSerializeSize(); }

  void Core::init() {
    symRetroInit();
    symRetroGetSystemInfo(retroSystemInfo);
  }

  void Core::deinit() { symRetroDeinit(); }

  void Core::reset() { symRetroReset(); }

  void Core::run(double deltaTime) { symRetroRun(); }

  void Core::setSystemDirectory(const string &frontendSystemDirectory) {
    systemDirectory = frontendSystemDirectory;
  }

  void Core::setSaveDirectory(const string &frontendSaveDirectory) {
    saveDirectory = frontendSaveDirectory;
  }

  void Core::recordPotentialAPIViolation(const std::string &msg) {
    printf("Potential API violation: %s\n", msg.c_str());
  }

  std::vector<char> Core::getMemoryData(const MemoryType memType) const {
    const auto size = symRetroGetMemoryDataSize(static_cast<unsigned>(memType));
    const auto ptr = symRetroGetMemoryData(static_cast<unsigned>(memType));

    const auto end = static_cast<char *>(ptr) + size;

    vector memData(static_cast<char *>(ptr), end);
    memData.resize(size);
    return memData;
  }

  void Core::writeMemoryData(const MemoryType memType,
                             const std::vector<char> &data) {
    const auto size = symRetroGetMemoryDataSize(static_cast<unsigned>(memType));
    const auto ptr = symRetroGetMemoryData(static_cast<unsigned>(memType));

    //  if (data.size() != size) {
    //    printf("um sizes don't match. data: %zu, size: %zu\n", data.size(),
    //    size);
    //  }

    memcpy(ptr, data.data(), size);
  }

  void *Core::getMemoryData(const unsigned id) const {
    return symRetroGetMemoryData(id);
  }

  size_t Core::getMemorySize(const unsigned id) const {
    return symRetroGetMemoryDataSize(id);
  }

  retro_memory_map *Core::getMemoryMap() { return &memoryMap; }

  void Core::setVideoReceiver(firelight::libretro::IVideoDataReceiver *receiver) {
    videoReceiver = receiver;
  }

  void Core::setRetropadProvider(
    firelight::libretro::IRetropadProvider *provider) {
    m_retropadProvider = provider;
  }

  firelight::libretro::IRetropadProvider *Core::getRetropadProvider() const {
    return m_retropadProvider;
  }

  void Core::setAudioReceiver(std::shared_ptr<IAudioDataReceiver> receiver) {
    audioReceiver = std::move(receiver);
  }
} // namespace libretro
