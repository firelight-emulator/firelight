#include "core.hpp"
#include "libretro/libretro_vulkan.h"

#include "SDL2/SDL.h"
#include "virtual_filesystem.hpp"
#include <cstdarg>
#include <filesystem>
#include <firelight/input/gamepad_input.hpp>
#include <stdexcept>
#include <vector>
#include <utility>

#include <spdlog/spdlog.h>

namespace libretro {
  // Only supports one core at a time for now, but, eh.
  static Core *currentCore;

  void log(enum retro_log_level level, const char *fmt, ...) {
    char msg[4096] = {};
    va_list va;
    va_start(va, fmt);
    vsnprintf(msg, sizeof(msg), fmt, va);
    va_end(va);

    msg[std::remove(msg, msg + strlen(msg), '\n') - msg] = 0;
    msg[std::remove(msg, msg + strlen(msg), '\r') - msg] = 0;

    // mGBA likes to spam the logs... though I could probably check the level.
    // if (strncmp(msg, "GBA DMA", 7) == 0) {
    //   return;
    // }

    if (level == RETRO_LOG_INFO) {
      spdlog::info("[Core] {}", msg);
    } else if (level == RETRO_LOG_DEBUG) {
      spdlog::debug("[Core] {}", msg);
    } else if (level == RETRO_LOG_WARN) {
      spdlog::warn("[Core] {}", msg);
    } else if (level == RETRO_LOG_ERROR) {
      spdlog::error("[Core] {}", msg);
    } else {
      spdlog::info("[Core] {}", msg); // Default to info for unknown levels.
    }
  }

  static int16_t inputStateCallback(unsigned port, unsigned device,
                                    unsigned index, unsigned id) {
    if (currentCore == nullptr) {
      // TODO: Report some error
      return 0;
    }

    // switch (device) {
    //   case RETRO_DEVICE_POINTER:
    //     spdlog::info("Asking for pointer input");
    //     break;
    //   case RETRO_DEVICE_KEYBOARD:
    //     spdlog::info("Asking for keyboard input");
    //     break;
    //   case RETRO_DEVICE_MOUSE:
    //     spdlog::info("Asking for mouse input");
    //     break;
    //   case RETRO_DEVICE_LIGHTGUN:
    //     spdlog::info("Asking for lightgun input");
    //     break;
    // }

    if (device == RETRO_DEVICE_POINTER) {
      const auto pointerProvider = currentCore->getPointerInputProvider();
      if (pointerProvider == nullptr) {
        return 0;
      }

      if (id == RETRO_DEVICE_ID_POINTER_X) {
        return pointerProvider->getPointerPosition().first;
      }
      if (id == RETRO_DEVICE_ID_POINTER_Y) {
        return pointerProvider->getPointerPosition().second;
      }
      if (id == RETRO_DEVICE_ID_POINTER_PRESSED) {
        return pointerProvider->isPressed();
      }
    }

    // Mouse / light gun. We branch on the device the core actually queries
    // (masked) rather than what we set — some cores (e.g. FCEUmm's Zapper,
    // advertised as a MOUSE subclass) query LIGHTGUN ids. Aim/motion come from
    // the pointer provider; buttons OR the physical mouse with any mapped
    // gamepad binding (evaluated for the matching device class).
    const auto deviceClass = device & RETRO_DEVICE_MASK;
    if (deviceClass == RETRO_DEVICE_MOUSE ||
        deviceClass == RETRO_DEVICE_LIGHTGUN) {
      namespace fi = firelight::input;
      // Two independent sources can drive a mouse / light gun:
      //  * the physical mouse — allowed whenever the toggle is on (default), so
      //    the mouse "just works" for these games regardless of the selected
      //    device type; and
      //  * the gamepad (stick aim + mapped buttons) — allowed only when the
      //    player selected that device type for this port, like the other
      //    controller types (the core may internally force e.g. FCEUmm's Zapper,
      //    but a port left on "Gamepad" won't drive it from the pad).
      // If neither source is allowed, the device is inert.
      const int selectedClass = currentCore->getPortInputClass(port);
      const bool gamepadAllowed =
          (deviceClass == RETRO_DEVICE_MOUSE &&
           selectedClass == static_cast<int>(fi::GamepadInputClass::Mouse)) ||
          (deviceClass == RETRO_DEVICE_LIGHTGUN &&
           selectedClass == static_cast<int>(fi::GamepadInputClass::Lightgun));
      const bool mouseAllowed = currentCore->mouseControlsPointerDevices();
      if (!mouseAllowed && !gamepadAllowed) {
        return 0;
      }
      const auto pointer = currentCore->getPointerInputProvider();
      const auto provider = currentCore->getRetropadProvider();
      const auto pad = provider
                         ? provider->getRetropadForPlayerIndex(port)
                         : nullptr;
      const int mapClass = static_cast<int>(
        deviceClass == RETRO_DEVICE_MOUSE
          ? fi::GamepadInputClass::Mouse
          : fi::GamepadInputClass::Lightgun);
      // Gamepad-sourced bindings only count when the gamepad drives this device.
      const auto mapped = [&](const fi::GamepadInput vi) -> bool {
        return gamepadAllowed && pad &&
               pad->isVirtualInputActive(currentCore->m_platformId, mapClass,
                                         static_cast<int>(vi));
      };
      // Physical mouse buttons only count when the mouse may drive this device.
      const auto mouseBtn = [&](const int btn) -> bool {
        return mouseAllowed && pointer && pointer->isMouseButtonPressed(btn);
      };

      if (deviceClass == RETRO_DEVICE_MOUSE) {
        switch (id) {
          case RETRO_DEVICE_ID_MOUSE_X:
            return currentCore->m_frameMouseDelta.first;
          case RETRO_DEVICE_ID_MOUSE_Y:
            return currentCore->m_frameMouseDelta.second;
          case RETRO_DEVICE_ID_MOUSE_LEFT:
            return mouseBtn(RETRO_DEVICE_ID_MOUSE_LEFT) || mapped(fi::MouseLeft);
          case RETRO_DEVICE_ID_MOUSE_RIGHT:
            return mouseBtn(RETRO_DEVICE_ID_MOUSE_RIGHT) || mapped(fi::MouseRight);
          case RETRO_DEVICE_ID_MOUSE_MIDDLE:
            return mouseBtn(RETRO_DEVICE_ID_MOUSE_MIDDLE) ||
                   mapped(fi::MouseMiddle);
          default:
            return 0;
        }
      }

      // RETRO_DEVICE_LIGHTGUN
      switch (id) {
        case RETRO_DEVICE_ID_LIGHTGUN_SCREEN_X:
          return pointer ? pointer->getPointerPosition().first : 0;
        case RETRO_DEVICE_ID_LIGHTGUN_SCREEN_Y:
          return pointer ? pointer->getPointerPosition().second : 0;
        case RETRO_DEVICE_ID_LIGHTGUN_X: // deprecated relative (snes9x)
          return currentCore->m_frameMouseDelta.first;
        case RETRO_DEVICE_ID_LIGHTGUN_Y:
          return currentCore->m_frameMouseDelta.second;
        case RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN:
          // Only the mouse can be "off screen"; when the gamepad drives the aim
          // the cursor is always on the surface.
          return mouseAllowed && pointer && pointer->isPointerOffscreen();
        case RETRO_DEVICE_ID_LIGHTGUN_TRIGGER:
          // Mouse left-click fires (when the mouse may drive it); a mapped gamepad
          // binding fires when the gamepad drives it.
          return (mouseAllowed && pointer && pointer->isPressed()) ||
                 mapped(fi::LightgunTrigger);
        case RETRO_DEVICE_ID_LIGHTGUN_RELOAD:
          // Right-click reloads (shoot off-screen) by convention.
          return mouseBtn(RETRO_DEVICE_ID_MOUSE_RIGHT) || mapped(fi::LightgunReload);
        case RETRO_DEVICE_ID_LIGHTGUN_AUX_A:
          return mapped(fi::LightgunAuxA);
        case RETRO_DEVICE_ID_LIGHTGUN_AUX_B:
          return mapped(fi::LightgunAuxB);
        case RETRO_DEVICE_ID_LIGHTGUN_AUX_C:
          return mapped(fi::LightgunAuxC);
        case RETRO_DEVICE_ID_LIGHTGUN_START:
          return mapped(fi::LightgunStart);
        case RETRO_DEVICE_ID_LIGHTGUN_SELECT:
          return mapped(fi::LightgunSelect);
        case RETRO_DEVICE_ID_LIGHTGUN_DPAD_UP:
          return mapped(fi::LightgunDpadUp);
        case RETRO_DEVICE_ID_LIGHTGUN_DPAD_DOWN:
          return mapped(fi::LightgunDpadDown);
        case RETRO_DEVICE_ID_LIGHTGUN_DPAD_LEFT:
          return mapped(fi::LightgunDpadLeft);
        case RETRO_DEVICE_ID_LIGHTGUN_DPAD_RIGHT:
          return mapped(fi::LightgunDpadRight);
        default:
          return 0;
      }
    }

    // Joypad reads come from the per-frame snapshot (Core::pollInput), so input
    // is stable for the whole retro_run() and each frame is a recordable value.
    if (port >= static_cast<unsigned>(Core::kMaxInputPorts) ||
        !currentCore->m_portActive[port]) {
      return 0;
    }
    const auto &frame = currentCore->m_portFrames[port];

    if (device == RETRO_DEVICE_ANALOG) {
      if (index == RETRO_DEVICE_INDEX_ANALOG_LEFT) {
        if (id == RETRO_DEVICE_ID_ANALOG_X) {
          return frame.leftStickX;
        }
        if (id == RETRO_DEVICE_ID_ANALOG_Y) {
          return frame.leftStickY;
        }
      } else if (index == RETRO_DEVICE_INDEX_ANALOG_RIGHT) {
        if (id == RETRO_DEVICE_ID_ANALOG_X) {
          return frame.rightStickX;
        }
        if (id == RETRO_DEVICE_ID_ANALOG_Y) {
          return frame.rightStickY;
        }
      }
    } else if (device == RETRO_DEVICE_JOYPAD) {
      return frame.button(id) ? 1 : 0;
    }

    return 0;
  }

  static void videoCallback(const void *data, unsigned width, unsigned height,
                            size_t pitch) {
    if (!currentCore->videoReceiver) {
      return;
    }
    currentCore->videoReceiver->receive(data, width, height, pitch);
  }

  static bool envCallback(unsigned cmd, void *data) {
    return currentCore->handleEnvironmentCall(cmd, data);
  }

  bool Core::handleEnvironmentCall(unsigned int cmd, void *data) {
    switch (cmd) {
      case RETRO_ENVIRONMENT_SET_ROTATION: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_ROTATION");
        auto rotation = *static_cast<unsigned *>(data);
        videoReceiver->setScreenRotation(rotation);
        return true;
      }
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
        if (systemDirectory.empty()) {
          return false;
        }

        auto ptr = static_cast<const char **>(data);
        *ptr = strdup(systemDirectory.c_str());
        return true;
      }
      case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_PIXEL_FORMAT");
        if (!videoReceiver) {
          spdlog::warn("No video receiver set; cannot set pixel format");
          return false;
        }
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

          spdlog::info("Got input descriptor: port {}, device {}, index {}, id {}",
                       descriptor.port, descriptor.device, descriptor.index,
                       descriptor.id);

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
        // The core hands us its disk-control callbacks; copy them by value (the
        // pointed-to struct may not persist) so we can drive disc swaps later.
        if (const auto *cb =
            static_cast<const retro_disk_control_callback *>(data)) {
          m_diskControl = *cb;
          m_hasDiskControl = true;
        }
        return true;
      }
      case RETRO_ENVIRONMENT_SET_HW_RENDER: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_HW_RENDER");
        if (!videoReceiver) {
          spdlog::warn("No video receiver set; cannot set HW render interface");
          return false;
        }
        auto *renderCallback = static_cast<retro_hw_render_callback *>(data);
        videoReceiver->setHwRenderInterface(renderCallback);
        m_destroyContextFunction = renderCallback->context_destroy;

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

        ptr->value = strdup(val->c_str());
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
        if (configProvider) {
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
          const auto controller =
              currentCore->getRetropadProvider()->getRetropadForPlayerIndex(port);
          if (!controller) {
            return true;
          }

          if (effect == RETRO_RUMBLE_STRONG) {
            controller->setStrongRumble(currentCore->m_platformId, strength);
          } else if (effect == RETRO_RUMBLE_WEAK) {
            controller->setWeakRumble(currentCore->m_platformId, strength);
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

        // Advertise the device types inputStateCallback actually services
        // (pointer for DS touch; mouse + light gun for WS2 device support).
        *ptr = (1 << RETRO_DEVICE_JOYPAD) | (1 << RETRO_DEVICE_ANALOG) |
               (1 << RETRO_DEVICE_POINTER) | (1 << RETRO_DEVICE_MOUSE) |
               (1 << RETRO_DEVICE_LIGHTGUN);
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

        // Camera not supported; hand back no-op callbacks and decline.
        auto ptr = static_cast<retro_camera_callback *>(data);
        ptr->start = [] { return false; };
        ptr->stop = [] {
        };
        return false;
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
        if (systemDirectory.empty()) {
          return false;
        }

        auto ptr = static_cast<const char **>(data);
        *ptr = strdup(systemDirectory.c_str());
        return true;
      }
      case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY");
        // Hand back the Firelight-managed per-game/per-slot save directory (set
        // in EmulatorInstance::initialize), NOT the system dir.
        if (saveDirectory.empty()) {
          return false;
        }

        auto ptr = static_cast<const char **>(data);
        *ptr = strdup(saveDirectory.c_str());
        return true;
      }
      case RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO");
        if (!videoReceiver) {
          spdlog::warn("No video receiver set; cannot set system AV info");
          return false;
        }
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
        // The core advertises the device types it accepts on each port; store a
        // copy (strings may be transient) so the UI can offer a per-port device
        // choice and we can call retro_set_controller_port_device.
        auto ptr = static_cast<retro_controller_info *>(data);
        m_controllerDevices.clear();
        for (unsigned port = 0; port < 100; ++port) {
          const auto &info = ptr[port];
          if (!info.types) {
            break;
          }
          std::vector<ControllerDeviceOption> devices;
          devices.reserve(info.num_types);
          for (unsigned j = 0; j < info.num_types; ++j) {
            const auto &type = info.types[j];
            devices.push_back(
              {type.id, type.desc != nullptr ? type.desc : std::string{}});
          }
          m_controllerDevices.push_back(std::move(devices));
        }
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
        if (!videoReceiver) {
          spdlog::warn("No video receiver set; cannot set geometry");
          return false;
        }
        retroSystemAVInfo->geometry = *static_cast<retro_game_geometry *>(data);
        videoReceiver->setSystemAVInfo(retroSystemAVInfo);
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
        if (!videoReceiver) {
          spdlog::warn("No video receiver set; cannot get HW render interface");
          return false;
        }

        auto ptr = static_cast<retro_hw_render_interface **>(data);
        currentCore->videoReceiver->getHwRenderInterface(ptr);
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
        if (!videoReceiver) {
          spdlog::warn("No video receiver set; cannot set HW render context "
            "negotiation interface");
          return false;
        }
        auto ptr =
            static_cast<retro_hw_render_context_negotiation_interface *>(data);
        currentCore->videoReceiver->setHwRenderContextNegotiationInterface(ptr);
        break;
      }
      case RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS:
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS");
      // Record the quirks the core reports for its savestate format (consulted
      // when we harden rewind/savestate; we accept the core's set as-is).
        if (data) {
          serializationQuirksBitmap = *static_cast<uint64_t *>(data);
        }
        break;
      case RETRO_ENVIRONMENT_SET_HW_SHARED_CONTEXT:
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_HW_SHARED_CONTEXT");
      // TODO: ?
        return true;
      case RETRO_ENVIRONMENT_GET_VFS_INTERFACE: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_VFS_INTERFACE");
        // Intentionally declined: Firelight does not virtualize core file I/O.
        // Cores fall back to stdio, which is correct given the managed system +
        // per-slot save directories we hand them. (A vfs:: impl exists in the
        // codebase if we ever want to intercept I/O.)
        return false;
      }
      case RETRO_ENVIRONMENT_GET_LED_INTERFACE: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_LED_INTERFACE");
        auto ptr = static_cast<retro_led_interface *>(data);
        ptr->set_led_state = [](int led, int state) {
          spdlog::trace("Core set LED {} to state {}", led, state);
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
          option.key = strdup(opt->key);
          option.label = strdup(opt->desc);
          option.description = strdup(opt->info);

          if (opt->default_value != nullptr) {
            option.defaultValueKey = strdup(opt->default_value);
          } else {
            option.defaultValueKey = strdup(opt->values[0].value);
          }

          for (int j = 0; j < 100; ++j) {
            auto val = opt->values[j];
            if (val.value == nullptr) {
              break;
            }

            firelight::libretro::IConfigurationProvider::OptionValue optionValue;
            optionValue.key = strdup(val.value);
            if (val.label != nullptr) {
              optionValue.label = strdup(val.label);
            } else {
              optionValue.label = strdup(val.value);
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
          option.key = strdup(opt.key);
          option.label = strdup(opt.desc);
          option.description = strdup(opt.info);

          if (opt.default_value != nullptr) {
            option.defaultValueKey = strdup(opt.default_value);
          } else {
            option.defaultValueKey = strdup(opt.values[0].value);
          }

          for (int j = 0; j < 100; ++j) {
            auto val = opt.values[j];
            if (val.value == nullptr) {
              break;
            }

            firelight::libretro::IConfigurationProvider::OptionValue optionValue;
            optionValue.key = strdup(val.value);
            if (val.label != nullptr) {
              optionValue.label = strdup(val.label);
            } else {
              optionValue.label = strdup(val.value);
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
        if (!videoReceiver) {
          spdlog::warn("No video receiver set; cannot get preferred HW render");
          return false;
        }
        *static_cast<unsigned *>(data) =
            currentCore->videoReceiver->getPreferredHwRender();
        return true;
      case RETRO_ENVIRONMENT_GET_DISK_CONTROL_INTERFACE_VERSION: {
        environmentCalls.emplace_back(
          "RETRO_ENVIRONMENT_GET_DISK_CONTROL_INTERFACE_VERSION");
        *static_cast<unsigned *>(data) = 1;
        return true;
      }
      case RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE: {
        environmentCalls.emplace_back(
          "RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE");
        // Extended interface (adds disc labels/paths); we use its shared base
        // functions for index-based swapping. Stored by value like the plain one.
        if (const auto *cb =
            static_cast<const retro_disk_control_ext_callback *>(data)) {
          m_diskControlExt = *cb;
          m_hasDiskControlExt = true;
        }
        return true;
      }
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
        // printf("Msg: %s\n", ptr->msg);
        return false;
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
          printf("Active: %d, Occupancy: %d, Underrun Likely: %d\n", active,
                 occupancy, underrun_likely);
        };

        break;
      }
      case RETRO_ENVIRONMENT_SET_MINIMUM_AUDIO_LATENCY: {
        environmentCalls.emplace_back(
          "RETRO_ENVIRONMENT_SET_MINIMUM_AUDIO_LATENCY");
        // Not needed as we implement dynamic rate control.
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

        std::map<std::string, std::string> categoryLabels;
        for (int i = 0; i < 100; ++i) {
          auto cat = ptr->categories[i];
          if (cat.key == nullptr) {
            break;
          }
          categoryLabels[cat.key] = cat.desc != nullptr ? cat.desc : cat.key;
        }

        for (int i = 0; i < 200; ++i) {
          auto opt = ptr->definitions[i];
          if (opt.key == nullptr) {
            break;
          }

          firelight::libretro::IConfigurationProvider::Option option;
          option.key = strdup(opt.key);

          if (opt.category_key != nullptr) {
            option.category = opt.category_key;
            const auto it = categoryLabels.find(opt.category_key);
            option.categoryLabel =
                it != categoryLabels.end() ? it->second : opt.category_key;
          }

          if (opt.default_value != nullptr) {
            option.defaultValueKey = strdup(opt.default_value);
          } else {
            option.defaultValueKey = strdup(opt.values[0].value);
          }

          if (opt.desc_categorized != nullptr) {
            option.label = strdup(opt.desc_categorized);
          } else {
            option.label = strdup(opt.desc);
          }

          if (opt.info_categorized != nullptr) {
            option.description = strdup(opt.info_categorized);
          } else if (opt.info != nullptr) {
            option.description = strdup(opt.info);
          } else {
            option.description = strdup(opt.desc);
          }

          for (int j = 0; j < 100; ++j) {
            auto val = opt.values[j];
            if (val.value == nullptr) {
              break;
            }

            firelight::libretro::IConfigurationProvider::OptionValue optionValue;
            optionValue.key = strdup(val.value);
            if (val.label != nullptr) {
              optionValue.label = strdup(val.label);
            } else {
              optionValue.label = strdup(val.value);
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

        std::map<std::string, std::string> categoryLabels;
        if (ptr->us != nullptr && ptr->us->categories != nullptr) {
          for (int i = 0; i < 100; ++i) {
            auto cat = ptr->us->categories[i];
            if (cat.key == nullptr) {
              break;
            }
            categoryLabels[cat.key] = cat.desc != nullptr ? cat.desc : cat.key;
          }
        }

        for (int i = 0; i < 200; ++i) {
          auto opt = ptr->us->definitions[i];
          if (opt.key == nullptr) {
            break;
          }

          firelight::libretro::IConfigurationProvider::Option option;
          option.key = strdup(opt.key);

          if (opt.category_key != nullptr) {
            option.category = opt.category_key;
            const auto it = categoryLabels.find(opt.category_key);
            option.categoryLabel =
                it != categoryLabels.end() ? it->second : opt.category_key;
          }

          if (opt.default_value != nullptr) {
            option.defaultValueKey = strdup(opt.default_value);
          } else {
            option.defaultValueKey = strdup(opt.values[0].value);
          }

          if (opt.desc_categorized != nullptr) {
            option.label = strdup(opt.desc_categorized);
          } else if (opt.desc != nullptr) {
            option.label = strdup(opt.desc);
          } else {
            option.label = strdup(opt.key);
          }

          if (opt.info_categorized != nullptr) {
            option.description = strdup(opt.info_categorized);
          } else if (opt.info != nullptr) {
            option.description = strdup(opt.info);
          }

          for (int j = 0; j < 100; ++j) {
            auto val = opt.values[j];
            if (val.value == nullptr) {
              break;
            }

            firelight::libretro::IConfigurationProvider::OptionValue optionValue;
            optionValue.key = strdup(val.value);
            if (val.label != nullptr) {
              optionValue.label = strdup(val.label);
            } else {
              optionValue.label = strdup(val.value);
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
        return false;
      }
      case RETRO_ENVIRONMENT_SET_VARIABLE: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_VARIABLE");
        // TODO: Implement
        break;
      }
      case RETRO_ENVIRONMENT_GET_THROTTLE_STATE: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_THROTTLE_STATE");
        auto ptr = static_cast<retro_throttle_state *>(data);
        // Not needed as far as I'm aware.
        return false;
      }
      case RETRO_ENVIRONMENT_GET_SAVESTATE_CONTEXT: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_SAVESTATE_CONTEXT");
        auto ptr = static_cast<retro_savestate_context *>(data);
        *ptr = RETRO_SAVESTATE_CONTEXT_NORMAL;
        return true;
      }
      case RETRO_ENVIRONMENT_GET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_SUPPORT: {
        environmentCalls.emplace_back("RETRO_ENVIRONMENT_GET_HW_RENDER_CONTEXT_"
          "NEGOTIATION_INTERFACE_SUPPORT");
        auto ptr =
            static_cast<retro_hw_render_context_negotiation_interface_type *>(data);
        *ptr = RETRO_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_VULKAN;
        return true;
      }
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
        // Intentionally unhandled libretro env commands (camera, location,
        // MIDI, microphone, JIT, device-power, throttle, playlist/file-browser
        // dirs, proc-address, input-max-users, …): declining is correct — the
        // core falls back to its defaults.
        spdlog::debug("Unhandled libretro env command: {}", cmd);
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

  Core::Core(int platformId, const std::string &libPath,
             const std::shared_ptr<firelight::libretro::IConfigurationProvider>
             &configProvider,
             std::string systemDirectory)
    : m_platformId(platformId), m_configurationProvider(configProvider),
      systemDirectory(std::move(systemDirectory)) {
    coreLib = std::make_unique<QLibrary>(QString::fromStdString(libPath));

    // dll = SDL_LoadObject(libPath.c_str());
    // if (dll == nullptr) {
    // // Check error
    // }

    if (!coreLib->load()) {
      throw std::runtime_error("Failed to load libretro core '" + libPath +
                               "': " + coreLib->errorString().toStdString());
    }

    // Resolve every libretro entry point up front. A compliant core exports all
    // of them (features it doesn't support are stubbed), so a missing symbol
    // means a broken/incompatible core; collect any misses and fail with a
    // clear message instead of crashing on a null call later.
    std::vector<std::string> missingSymbols;
    const auto req = [&](const char *name) -> QFunctionPointer {
      const auto ptr = coreLib->resolve(name);
      if (!ptr) {
        missingSymbols.emplace_back(name);
      }
      return ptr;
    };

    symRetroInit = req("retro_init");
    symRetroDeinit = req("retro_deinit");
    symRetroApiVersion =
        reinterpret_cast<unsigned int (*)()>(req("retro_api_version"));
    symRetroGetSystemInfo = reinterpret_cast<void (*)(retro_system_info *)>(
      req("retro_get_system_info"));
    symRetroGetSystemAVInfo = reinterpret_cast<void (*)(retro_system_av_info *)>(
      req("retro_get_system_av_info"));
    symRetroSetControllerPortDevice =
        reinterpret_cast<void (*)(unsigned int, unsigned int)>(
          req("retro_set_controller_port_device"));
    symRetroReset = req("retro_reset");
    symRetroRun = req("retro_run");
    symRetroSerializeSize =
        reinterpret_cast<size_t (*)()>(req("retro_serialize_size"));
    symRetroSerialize =
        reinterpret_cast<bool (*)(void *, size_t)>(req("retro_serialize"));
    symRetroUnserialize = reinterpret_cast<bool (*)(const void *, size_t)>(
      req("retro_unserialize"));
    symRetroCheatReset = req("retro_cheat_reset");
    symRetroCheatSet = reinterpret_cast<void (*)(unsigned, bool, const char *)>(
      req("retro_cheat_set"));

    symRetroLoadGame = reinterpret_cast<bool (*)(const retro_game_info *)>(
      req("retro_load_game"));
    symRetroLoadGameSpecial =
        reinterpret_cast<bool (*)(unsigned int, const retro_game_info *, size_t)>(
          req("retro_load_game_special"));
    symRetroUnloadGame = req("retro_unload_game");
    symRetroGetRegion =
        reinterpret_cast<unsigned int (*)()>(req("retro_get_region"));

    symRetroGetMemoryData =
        reinterpret_cast<void *(*)(unsigned int)>(req("retro_get_memory_data"));
    symRetroGetMemoryDataSize =
        reinterpret_cast<size_t (*)(unsigned int)>(req("retro_get_memory_size"));

    const auto setEnvironment =
        reinterpret_cast<RetroSetEnvironment>(req("retro_set_environment"));
    const auto setVideoRefresh =
        reinterpret_cast<RetroSetVideoRefresh>(req("retro_set_video_refresh"));
    const auto setAudioSample =
        reinterpret_cast<RetroSetAudioSample>(req("retro_set_audio_sample"));
    const auto setAudioSampleBatch = reinterpret_cast<RetroSetAudioSampleBatch>(
      req("retro_set_audio_sample_batch"));
    const auto setInputPoll =
        reinterpret_cast<RetroInputPoll>(req("retro_set_input_poll"));
    const auto setInputState =
        reinterpret_cast<RetroInputState>(req("retro_set_input_state"));

    if (!missingSymbols.empty()) {
      std::string joined;
      for (size_t i = 0; i < missingSymbols.size(); ++i) {
        joined += (i == 0 ? "" : ", ") + missingSymbols[i];
      }
      coreLib->unload();
      throw std::runtime_error("libretro core '" + libPath +
                               "' is missing required symbol(s): " + joined);
    }

    retroSystemInfo = new retro_system_info;
    retroSystemAVInfo = new retro_system_av_info;

    currentCore = this; // todo prob different namespace

    setEnvironment(envCallback);
    setVideoRefresh(videoCallback);
    setAudioSample([](int16_t left, int16_t right) {
    });

    auto processAudioLambda = [](const int16_t *data, size_t frames) -> size_t {
      auto core = currentCore;
      if (core == nullptr) {
        printf("core was null in libretro audio callback\n");
        return frames;
      }

      return core->audioReceiver->receive(data, frames);
    };

    setAudioSampleBatch(processAudioLambda);
    setInputPoll([] {
      if (currentCore) {
        currentCore->pollInput();
      }
    });
    setInputState(inputStateCallback);
  }

  Core::~Core() {
    spdlog::info("[Core] Unloading core");
    // Teardown order matters for HW-rendered cores (e.g. PPSSPP): the frontend
    // owns the VkDevice, but the core keeps using it through retro_unload_game /
    // retro_deinit — PPSSPP's Shutdown() calls vkDeviceWaitIdle, and its own
    // vkDestroyDevice is a no-op (the frontend must destroy the device). So run
    // the core's context_destroy, fully deinit the core, and only THEN tear down
    // the HW context/device — all while the DLL is still loaded, since
    // destroy_device is invoked from destroyHwContext() and needs the DLL.
    if (m_destroyContextFunction) {
      m_destroyContextFunction();
      m_destroyContextFunction = nullptr;
    }
    if (currentCore != nullptr && coreLib != nullptr) {
      unloadGame();
      deinit();
      if (videoReceiver) {
        videoReceiver->destroyHwContext();
      }
      coreLib->unload();
      currentCore = nullptr;
    } else if (videoReceiver) {
      // No loaded DLL to deinit, but still release any HW context we created.
      videoReceiver->destroyHwContext();
    }
  }

  bool Core::loadGame(Game *game) {
    this->game = game;
    retro_game_info info{};

    // info.path = game->getPath().c_str();
    info.path = strdup(
      std::filesystem::path(game->getPath()).string().c_str());
    // info.path = R"()";
    info.data = game->getData();
    info.size = game->getSize();
    info.meta = "";

    // spdlog::warn("Path before c_str: {}", game->getPath());
    // spdlog::warn("Path after c_str: {}", string(info.path));
    // TODO: meta?
    auto result = symRetroLoadGame(&info);

    symRetroGetSystemAVInfo(retroSystemAVInfo);
    if (videoReceiver) {
      videoReceiver->setSystemAVInfo(retroSystemAVInfo);
    }
    //  video->setGameGeometry(&retroSystemAVInfo->geometry);

    audioReceiver->initialize(retroSystemAVInfo->timing.sample_rate);
    // this->game = nullptr;
    return result;
  }

  void Core::unloadGame() { symRetroUnloadGame(); }

  std::vector<uint8_t> Core::serializeState() const {
    const auto size = symRetroSerializeSize();

    std::vector<uint8_t> data(size);
    if (!symRetroSerialize(data.data(), size)) {
      spdlog::error("[Core] Failed to serialize state ({} bytes)", size);
      return {};
    }

    return data;
  }

  bool Core::deserializeState(const std::vector<uint8_t> &data) const {
    const auto size = getSerializeSize();

    // A mismatched size means the state is for a different game/core version;
    // unserializing it could corrupt the running game, so refuse.
    if (data.size() != size) {
      spdlog::error(
        "[Core] Refusing to load state: size mismatch (data: {}, expected: {})",
        data.size(), size);
      return false;
    }

    if (!symRetroUnserialize(data.data(), size)) {
      spdlog::error("[Core] Core rejected state ({} bytes)", size);
      return false;
    }
    return true;
  }

  std::size_t Core::getSerializeSize() const { return symRetroSerializeSize(); }

  void Core::init() {
    symRetroInit();
    symRetroGetSystemInfo(retroSystemInfo);

    spdlog::info("[Core] Libretro core loaded. Core info:");
    spdlog::info("[Core]   Library name: {}", retroSystemInfo->library_name);
    spdlog::info("[Core]   Library version: {}", retroSystemInfo->library_version);
    spdlog::info("[Core]   Valid extensions: {}", retroSystemInfo->valid_extensions);
    spdlog::info("[Core]   Need full path: {}", retroSystemInfo->need_fullpath);
    spdlog::info("[Core]   Block extraction: {}", retroSystemInfo->block_extract);
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
    const auto ptr = static_cast<char *>(
      symRetroGetMemoryData(static_cast<unsigned>(memType)));

    return std::vector(ptr, ptr + size);
  }

  void Core::writeMemoryData(const MemoryType memType,
                             const std::vector<char> &data) {
    const auto size = symRetroGetMemoryDataSize(static_cast<unsigned>(memType));
    const auto ptr = symRetroGetMemoryData(static_cast<unsigned>(memType));

    if (data.size() > size) {
      spdlog::error("Data size is larger than memory size");
      return;
    }

    memcpy(ptr, data.data(), data.size());
  }

  void *Core::getMemoryData(const unsigned id) const {
    return symRetroGetMemoryData(id);
  }

  size_t Core::getMemorySize(const unsigned id) const {
    return symRetroGetMemoryDataSize(id);
  }

  retro_memory_map *Core::getMemoryMap() { return &memoryMap; }

  unsigned Core::getDiskCount() const {
    if (m_hasDiskControlExt && m_diskControlExt.get_num_images) {
      return m_diskControlExt.get_num_images();
    }
    if (m_hasDiskControl && m_diskControl.get_num_images) {
      return m_diskControl.get_num_images();
    }
    return 0;
  }

  unsigned Core::getCurrentDiskIndex() const {
    if (m_hasDiskControlExt && m_diskControlExt.get_image_index) {
      return m_diskControlExt.get_image_index();
    }
    if (m_hasDiskControl && m_diskControl.get_image_index) {
      return m_diskControl.get_image_index();
    }
    return 0;
  }

  bool Core::setDiskIndex(const unsigned index) {
    // Prefer the extended interface's functions when present; both expose the
    // same base eject/set-index/count callbacks.
    const auto setEject =
        m_hasDiskControlExt
          ? m_diskControlExt.set_eject_state
          : (m_hasDiskControl
               ? m_diskControl.set_eject_state
               : nullptr);
    const auto setIndex =
        m_hasDiskControlExt
          ? m_diskControlExt.set_image_index
          : (m_hasDiskControl
               ? m_diskControl.set_image_index
               : nullptr);
    if (!setEject || !setIndex || index >= getDiskCount()) {
      return false;
    }
    // A disc swap is eject -> select -> insert.
    setEject(true);
    const bool ok = setIndex(index);
    setEject(false);
    return ok;
  }

  std::vector<std::vector<Core::ControllerDeviceOption> >
  Core::getControllerDevices() const {
    return m_controllerDevices;
  }

  void Core::setControllerPortDevice(const unsigned port,
                                     const unsigned device) {
    if (symRetroSetControllerPortDevice) {
      symRetroSetControllerPortDevice(port, device);
    }
  }

  void Core::setPortInputDeviceClass(const unsigned port, const int deviceClass) {
    m_portInputClass[port] = deviceClass;
  }

  void Core::setAnalogPointerSpeed(const double stepPerFrame) {
    m_analogPointerSpeed = stepPerFrame;
  }

  void Core::setMouseControlsPointerDevices(const bool enabled) {
    m_mouseControlsPointerDevices = enabled;
  }

  int Core::getPortInputClass(const unsigned port) const {
    const auto it = m_portInputClass.find(port);
    return it != m_portInputClass.end()
             ? it->second
             : static_cast<int>(firelight::input::GamepadInputClass::Joypad);
  }

  void Core::setCheat(const unsigned index, const bool enabled,
                      const std::string &code) {
    if (symRetroCheatSet) {
      symRetroCheatSet(index, enabled, code.c_str());
    }
  }

  void Core::clearCheats() {
    if (symRetroCheatReset) {
      symRetroCheatReset();
    }
  }

  void Core::setVideoReceiver(firelight::libretro::IVideoDataReceiver *receiver) {
    videoReceiver = receiver;
  }

  void Core::setRetropadProvider(
    firelight::libretro::IRetropadProvider *provider) {
    m_retropadProvider = provider;
  }

  void Core::setPointerInputProvider(
    firelight::libretro::IPointerInputProvider *provider) {
    m_pointerInputProvider = provider;
  }

  firelight::libretro::IPointerInputProvider *
  Core::getPointerInputProvider() const {
    return m_pointerInputProvider;
  }

  firelight::libretro::IRetropadProvider *Core::getRetropadProvider() const {
    return m_retropadProvider;
  }

  void Core::pollInput() {
    // Drive the pointer cursor from a gamepad analog stick for any Mouse /
    // Light-Gun port ("velocity glide": cursor speed proportional to stick
    // deflection). Runs before the mouse-delta snapshot below so a stick-driven
    // MOUSE device also sees the motion this frame.
    if (m_pointerInputProvider && m_retropadProvider) {
      namespace fi = firelight::input;
      for (const auto &[port, deviceClass]: m_portInputClass) {
        if (deviceClass != static_cast<int>(fi::GamepadInputClass::Mouse) &&
            deviceClass != static_cast<int>(fi::GamepadInputClass::Lightgun)) {
          continue;
        }
        // The controller on that port aims it; fall back to player 0 so a single
        // controller drives the gun even when the game auto-placed it elsewhere
        // (e.g. Duck Hunt's Zapper on port 2).
        auto pad =
            m_retropadProvider->getRetropadForPlayerIndex(static_cast<int>(port));
        if (!pad) {
          pad = m_retropadProvider->getRetropadForPlayerIndex(0);
        }
        if (!pad) {
          continue;
        }
        const auto stickX = pad->getLeftStickXPosition(m_platformId, 1);
        const auto stickY = pad->getLeftStickYPosition(m_platformId, 1);
        m_pointerInputProvider->nudgeCursor(
          firelight::libretro::cursorGlideDelta(stickX, m_analogPointerSpeed),
          firelight::libretro::cursorGlideDelta(stickY, m_analogPointerSpeed));
        break; // One shared cursor: the first Mouse/Light-Gun port drives it.
      }
    }

    // Snapshot the frame's mouse motion once (getRelativeMotion consumes it) so
    // the MOUSE_X/MOUSE_Y (and deprecated LIGHTGUN_X/Y) reads share one value.
    if (m_pointerInputProvider) {
      m_frameMouseDelta = m_pointerInputProvider->getRelativeMotion();
    } else {
      m_frameMouseDelta = {0, 0};
    }

    // Snapshot each port's joypad state once, so the input callback reads stable
    // input for the whole frame (buttons/axes can't shift mid-run) and the frame
    // is a recordable InputFrame. Ports with no controller are marked inactive.
    for (int port = 0; port < kMaxInputPorts; ++port) {
      const auto pad =
          m_retropadProvider
            ? m_retropadProvider->getRetropadForPlayerIndex(port)
            : nullptr;
      if (pad) {
        m_portFrames[port] =
            firelight::input::captureJoypadFrame(*pad, m_platformId, 1);
        m_portActive[port] = true;
      } else {
        m_portActive[port] = false;
      }
    }
  }

  void Core::setAudioReceiver(std::shared_ptr<IAudioDataReceiver> receiver) {
    audioReceiver = std::move(receiver);
  }
} // namespace libretro
