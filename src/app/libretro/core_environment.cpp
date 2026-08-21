#include "core_environment.hpp"

#include "SDL2/SDL.h"
#include "libretro/libretro_vulkan.h"

#include <cstring>
#include <filesystem>
#include <map>
#include <spdlog/spdlog.h>

namespace libretro {
static retro_microphone_t *RETRO_CALLCONV micOpen(const retro_microphone_params_t * /*params*/) {
  auto *p = (g_ctx ? g_ctx->audioInput : nullptr);
  return p ? p->openMicrophone() : nullptr;
}

static void RETRO_CALLCONV micClose(retro_microphone_t *mic) {
  if (auto *p = (g_ctx ? g_ctx->audioInput : nullptr)) {
    p->closeMicrophone(mic);
  }
}

static bool RETRO_CALLCONV micGetParams(const retro_microphone_t *mic, retro_microphone_params_t *params) {
  auto *p = (g_ctx ? g_ctx->audioInput : nullptr);
  return p && p->getMicrophoneParameters(mic, params);
}

static bool RETRO_CALLCONV micSetState(retro_microphone_t *mic, bool state) {
  auto *p = (g_ctx ? g_ctx->audioInput : nullptr);
  return p && p->setMicrophoneState(mic, state);
}

static bool RETRO_CALLCONV micGetState(const retro_microphone_t *mic) {
  auto *p = (g_ctx ? g_ctx->audioInput : nullptr);
  return p && p->getMicrophoneState(mic);
}

static int RETRO_CALLCONV micRead(retro_microphone_t *mic, int16_t *samples, size_t num_samples) {
  auto *p = (g_ctx ? g_ctx->audioInput : nullptr);
  return p ? p->readMicrophone(mic, samples, num_samples) : 0;
}

bool Core::handleEnvironmentCall(unsigned int cmd, void *data) {
  if (m_envHandlers.empty()) {
    buildEnvironmentHandlers();
  }
  const auto it = m_envHandlers.find(cmd);
  if (it == m_envHandlers.end()) {
    // Intentionally unhandled libretro env commands (camera, location, MIDI,
    // JIT, device-power, throttle, playlist dirs, proc-address, …): declining
    // is correct — the core falls back to its defaults
    spdlog::debug("Unhandled libretro env command: {}", cmd);
    environmentCalls.emplace_back("UNIMPLEMENTED");
    return false;
  }
  environmentCalls.emplace_back(it->second.name);
  return it->second.handler(data);
}

void Core::buildEnvironmentHandlers() {
  m_envHandlers[RETRO_ENVIRONMENT_SET_ROTATION] = {"RETRO_ENVIRONMENT_SET_ROTATION", [this](void *data) -> bool {
                                                     auto rotation = *static_cast<unsigned *>(data);
                                                     videoReceiver->setScreenRotation(rotation);
                                                     return true;
                                                   }};
  m_envHandlers[(3 | 0x800000)] = {"RETRO_ENVIRONMENT_GET_CLEAR_ALL_THREAD_WAITS_CB", [this](void *data) -> bool {
                                     auto ptr = static_cast<retro_environment_t *>(data);
                                     *ptr = [](unsigned int cmd, void *data) {
                                       printf("Calling weirdo callback");
                                       return true;
                                     };
                                     return true;
                                   }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_OVERSCAN] = {"RETRO_ENVIRONMENT_GET_OVERSCAN", [this](void *data) -> bool {
                                                     recordPotentialAPIViolation(
                                                         "Using deprecated environment call GET_OVERSCAN");
                                                     *static_cast<bool *>(data) = false;
                                                     return true;
                                                   }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_CAN_DUPE] = {"RETRO_ENVIRONMENT_GET_CAN_DUPE", [this](void *data) -> bool {
                                                     *static_cast<bool *>(data) = true;
                                                     return true;
                                                   }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_MESSAGE] = {"RETRO_ENVIRONMENT_SET_MESSAGE", [this](void *data) -> bool {
                                                    auto ptr = static_cast<retro_message *>(data);
                                                    // TODO: Do something to queue message
                                                    printf("Got message for %d frames: %s\n", ptr->frames, ptr->msg);
                                                    return true;
                                                  }};
  m_envHandlers[RETRO_ENVIRONMENT_SHUTDOWN] = {"RETRO_ENVIRONMENT_SHUTDOWN", [this](void *data) -> bool {
                                                 shutdown = *static_cast<bool *>(data);
                                                 return true;
                                               }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL] = {"RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL",
                                                            [this](void *data) -> bool {
                                                              performanceLevel = *static_cast<unsigned *>(data);
                                                              return true;
                                                            }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY] = {"RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY",
                                                           [this](void *data) -> bool {
                                                             if (systemDirectory.empty()) {
                                                               return false;
                                                             }

                                                             auto ptr = static_cast<const char **>(data);
                                                             *ptr = strdup(systemDirectory.c_str());
                                                             return true;
                                                           }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_PIXEL_FORMAT] = {
      "RETRO_ENVIRONMENT_SET_PIXEL_FORMAT", [this](void *data) -> bool {
        if (!videoReceiver) {
          spdlog::warn("No video receiver set; cannot set pixel format");
          return false;
        }
        auto ptr = static_cast<retro_pixel_format *>(data);
        videoReceiver->setPixelFormat(ptr);
        return true;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS] = {
      "RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS", [this](void *data) -> bool {
        auto ptr = static_cast<retro_input_descriptor *>(data);
        // TODO sane default
        for (int i = 0; i < 100; ++i) {
          auto descriptor = ptr[i];
          if (descriptor.description == nullptr) {
            break;
          }

          spdlog::info("Got input descriptor: port {}, device {}, index {}, id {}", descriptor.port, descriptor.device,
                       descriptor.index, descriptor.id);

          inputDescriptors.emplace_back(descriptor);

          if (i == 99) {
            recordPotentialAPIViolation("Over 100 input descriptors");
          }
        }
        return true;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK] = {
      "RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK", [this](void *data) -> bool {
        // The core hands us a function to call on every key; keep it. (This
        // used to assign *into* ptr->callback, overwriting the core's own
        // pointer with a stub — so cores that drive themselves from the
        // keyboard, like DOS and Amiga, never saw a key.)
        const auto *ptr = static_cast<retro_keyboard_callback *>(data);
        keyboardCallback = ptr->callback;
        return true;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE] = {
      "RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE", [this](void *data) -> bool {
        // The core hands us its disk-control callbacks; copy them by value (the
        // pointed-to struct may not persist) so we can drive disc swaps later
        if (const auto *cb = static_cast<const retro_disk_control_callback *>(data)) {
          m_diskControl = *cb;
          m_hasDiskControl = true;
        }
        return true;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_HW_RENDER] = {
      "RETRO_ENVIRONMENT_SET_HW_RENDER", [this](void *data) -> bool {
        if (!videoReceiver) {
          spdlog::warn("No video receiver set; cannot set HW render interface");
          return false;
        }
        auto *renderCallback = static_cast<retro_hw_render_callback *>(data);
        videoReceiver->setHwRenderInterface(renderCallback);
        m_destroyContextFunction = renderCallback->context_destroy;

        return true;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_VARIABLE] = {"RETRO_ENVIRONMENT_GET_VARIABLE", [this](void *data) -> bool {
                                                     auto ptr = static_cast<retro_variable *>(data);

                                                     auto configProvider = m_configurationProvider;
                                                     if (!configProvider) {
                                                       return false;
                                                     }

                                                     auto val = configProvider->getOptionValue(ptr->key);
                                                     if (!val.has_value()) {
                                                       return false;
                                                     }

                                                     ptr->value = strdup(val->c_str());
                                                     return true;
                                                   }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_VARIABLES] = {"RETRO_ENVIRONMENT_SET_VARIABLES", [this](void *data) -> bool {
                                                      auto ptr = static_cast<retro_variable *>(data);

                                                      auto configProvider = m_configurationProvider;
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
                                                      return true;
                                                    }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE] = {
      "RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE", [this](void *data) -> bool {
        const auto configProvider = m_configurationProvider;
        if (configProvider) {
          *static_cast<bool *>(data) = configProvider->anyOptionValueHasChanged();
        } else {
          *static_cast<bool *>(data) = false;
        }
        return true;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME] = {"RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME",
                                                          [this](void *data) -> bool {
                                                            canRunWithNoGame = *static_cast<bool *>(data);
                                                            return true;
                                                          }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_LIBRETRO_PATH] = {"RETRO_ENVIRONMENT_GET_LIBRETRO_PATH",
                                                        [this](void *data) -> bool {
                                                          if (libretroPath.empty()) {
                                                            return false;
                                                          }
                                                          *static_cast<const char **>(data) = &libretroPath[0];
                                                          return true;
                                                        }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK] = {"RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK",
                                                              [this](void *data) -> bool {
                                                                //    video->setFrameTimeCallback((retro_frame_time_callback
                                                                //    *)data);
                                                                return true;
                                                              }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK] = {"RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK",
                                                         [this](void *data) -> bool {
                                                           auto ptr = static_cast<retro_audio_callback *>(data);
                                                           ptr->callback = nullptr;
                                                           ptr->set_state = nullptr;
                                                           return false;
                                                         }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE] = {
      "RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE", [this](void *data) -> bool {
        auto ptr = static_cast<retro_rumble_interface *>(data);
        ptr->set_rumble_state = [](unsigned port, enum retro_rumble_effect effect, uint16_t strength) {
          const auto retropad = g_ctx && g_ctx->input ? g_ctx->input->getRetropadProvider() : nullptr;
          const auto controller = retropad ? retropad->getRetropadForPlayerIndex(port) : nullptr;
          if (!controller) {
            return true;
          }

          if (effect == RETRO_RUMBLE_STRONG) {
            controller->setStrongRumble(g_ctx->platformId, strength);
          } else if (effect == RETRO_RUMBLE_WEAK) {
            controller->setWeakRumble(g_ctx->platformId, strength);
          }

          return true;
        };
        return true;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES] = {
      "RETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES", [this](void *data) -> bool {
        auto ptr = static_cast<uint64_t *>(data);
        //* Gets a bitmask telling which device type are expected to be
        // * handled properly in a call to retro_input_state_t
        // * Devices which are not handled or recognized always return
        // * 0 in retro_input_state_t
        // * Example bitmask: caps = (1 << RETRO_DEVICE_JOYPAD) | (1 <<
        // RETRO_DEVICE_ANALOG)

        // Advertise the device types inputStateCallback actually services
        // (pointer for DS touch; mouse + light gun for WS2 device support)
        *ptr = (1 << RETRO_DEVICE_JOYPAD) | (1 << RETRO_DEVICE_ANALOG) | (1 << RETRO_DEVICE_POINTER) |
               (1 << RETRO_DEVICE_MOUSE) | (1 << RETRO_DEVICE_LIGHTGUN);
        return true;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_SENSOR_INTERFACE] = {"RETRO_ENVIRONMENT_GET_SENSOR_INTERFACE",
                                                           [this](void *data) -> bool {
                                                             auto ptr = static_cast<retro_sensor_interface *>(data);
                                                             ptr->set_sensor_state = nullptr;
                                                             ptr->get_sensor_input = nullptr;
                                                             return false;
                                                           }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_CAMERA_INTERFACE] = {"RETRO_ENVIRONMENT_GET_CAMERA_INTERFACE",
                                                           [this](void *data) -> bool {
                                                             // Camera not supported; hand back no-op callbacks and
                                                             // decline
                                                             auto ptr = static_cast<retro_camera_callback *>(data);
                                                             ptr->start = [] { return false; };
                                                             ptr->stop = [] {};
                                                             return false;
                                                           }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_LOG_INTERFACE] = {"RETRO_ENVIRONMENT_GET_LOG_INTERFACE",
                                                        [this](void *data) -> bool {
                                                          auto ptr = static_cast<retro_log_callback *>(data);
                                                          ptr->log = log;
                                                          // return false;
                                                          return true;
                                                        }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_PERF_INTERFACE] = {
      "RETRO_ENVIRONMENT_GET_PERF_INTERFACE", [this](void *data) -> bool {
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
         * performance counter system)
         * */
        ptr->get_perf_counter = [] { return static_cast<retro_perf_tick_t>(0); };

        ptr->perf_register = [](retro_perf_counter *counter) { printf("Registering counter: %s\n", counter->ident); };

        ptr->perf_start = [](retro_perf_counter *counter) { printf("Starting counter: %s\n", counter->ident); };

        ptr->perf_stop = [](retro_perf_counter *counter) { printf("Stopping counter: %s\n", counter->ident); };

        /* Asks frontend to log and/or display the state of performance
         * counters. Performance counters can always be poked into manually as
         * well
         */
        ptr->perf_log = [] {};

        return false;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE] = {"RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE",
                                                             [this](void *data) -> bool {
                                                               auto ptr = static_cast<retro_location_callback *>(data);
                                                               ptr->start = nullptr;
                                                               ptr->stop = nullptr;
                                                               ptr->get_position = nullptr;
                                                               ptr->set_interval = nullptr;
                                                               ptr->initialized = nullptr;
                                                               ptr->deinitialized = nullptr;
                                                               return false;
                                                             }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY] = {"RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY",
                                                                [this](void *data) -> bool {
                                                                  if (systemDirectory.empty()) {
                                                                    return false;
                                                                  }

                                                                  auto ptr = static_cast<const char **>(data);
                                                                  *ptr = strdup(systemDirectory.c_str());
                                                                  return true;
                                                                }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY] = {"RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY",
                                                         [this](void *data) -> bool {
                                                           // Hand back the Firelight-managed per-game/per-slot save
                                                           // directory (set in EmulatorInstance::initialize), NOT the
                                                           // system dir
                                                           if (m_saveDirectory.empty()) {
                                                             return false;
                                                           }

                                                           auto ptr = static_cast<const char **>(data);
                                                           *ptr = strdup(m_saveDirectory.c_str());
                                                           return true;
                                                         }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO] = {
      "RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO", [this](void *data) -> bool {
        if (!videoReceiver) {
          spdlog::warn("No video receiver set; cannot set system AV info");
          return false;
        }
        videoReceiver->setSystemAVInfo(static_cast<retro_system_av_info *>(data));
        return true;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO] = {"RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO",
                                                         [this](void *data) -> bool {
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
                                                         }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_CONTROLLER_INFO] = {
      "RETRO_ENVIRONMENT_SET_CONTROLLER_INFO", [this](void *data) -> bool {
        // The core advertises the device types it accepts on each port; store a
        // copy (strings may be transient) so the UI can offer a per-port device
        // choice and we can call retro_set_controller_port_device
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
            devices.push_back({type.id, type.desc != nullptr ? type.desc : std::string{}});
          }
          m_controllerDevices.push_back(std::move(devices));
        }
        return true;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_MEMORY_MAPS] = {
      "RETRO_ENVIRONMENT_SET_MEMORY_MAPS", [this](void *data) -> bool {
        auto ptr = static_cast<retro_memory_map *>(data);
        for (unsigned i = 0; i < ptr->num_descriptors; ++i) {
          if (ptr->descriptors[i].ptr == nullptr) {
            break;
          }
          memoryDescriptors.emplace_back(retro_memory_descriptor{
              ptr->descriptors[i].flags, ptr->descriptors[i].ptr, ptr->descriptors[i].offset, ptr->descriptors[i].start,
              ptr->descriptors[i].select, ptr->descriptors[i].disconnect, ptr->descriptors[i].len,
              ptr->descriptors[i].addrspace});
        }

        memoryMap.descriptors = &memoryDescriptors[0];
        memoryMap.num_descriptors = ptr->num_descriptors;

        return true;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_GEOMETRY] = {"RETRO_ENVIRONMENT_SET_GEOMETRY", [this](void *data) -> bool {
                                                     if (!videoReceiver) {
                                                       spdlog::warn("No video receiver set; cannot set geometry");
                                                       return false;
                                                     }
                                                     retroSystemAVInfo->geometry =
                                                         *static_cast<retro_game_geometry *>(data);
                                                     videoReceiver->setSystemAVInfo(retroSystemAVInfo);
                                                     return true;
                                                   }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_USERNAME] = {"RETRO_ENVIRONMENT_GET_USERNAME", [this](void *data) -> bool {
                                                     auto ptr = static_cast<const char **>(data);
                                                     *ptr = &username[0];
                                                     return true;
                                                   }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_LANGUAGE] = {"RETRO_ENVIRONMENT_GET_LANGUAGE", [this](void *data) -> bool {
                                                     // TODO: Set by user
                                                     auto ptr = static_cast<retro_language *>(data);
                                                     *ptr = RETRO_LANGUAGE_ENGLISH;
                                                     return true;
                                                   }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER] = {
      "RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER", [this](void *data) -> bool { return true; }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE] = {
      "RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE", [this](void *data) -> bool {
        if (!videoReceiver) {
          spdlog::warn("No video receiver set; cannot get HW render interface");
          return false;
        }

        auto ptr = static_cast<retro_hw_render_interface **>(data);
        videoReceiver->getHwRenderInterface(ptr);
        return true;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS] = {"RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS",
                                                               [this](void *data) -> bool {
                                                                 supportsAchievements = *static_cast<bool *>(data);
                                                                 return true;
                                                               }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE] = {
      "RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE", [this](void *data) -> bool {
        if (!videoReceiver) {
          spdlog::warn("No video receiver set; cannot set HW render context "
                       "negotiation interface");
          return false;
        }
        auto ptr = static_cast<retro_hw_render_context_negotiation_interface *>(data);
        videoReceiver->setHwRenderContextNegotiationInterface(ptr);
        return true;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS] = {
      "RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS", [this](void *data) -> bool {
        // Record the quirks the core reports for its savestate format (consulted
        // when we harden rewind/savestate; we accept the core's set as-is)
        if (data) {
          serializationQuirksBitmap = *static_cast<uint64_t *>(data);
        }
        return true;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_HW_SHARED_CONTEXT] = {"RETRO_ENVIRONMENT_SET_HW_SHARED_CONTEXT",
                                                            [this](void *data) -> bool {
                                                              // TODO: ?
                                                              return true;
                                                            }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_VFS_INTERFACE] = {"RETRO_ENVIRONMENT_GET_VFS_INTERFACE",
                                                        [this](void *data) -> bool {
                                                          // Intentionally declined: Firelight does not virtualize core
                                                          // file I/O Cores fall back to stdio, which is correct given
                                                          // the managed system + per-slot save directories we hand
                                                          // them. (A vfs:: impl exists in the codebase if we ever want
                                                          // to intercept I/O.)
                                                          return false;
                                                        }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_LED_INTERFACE] = {
      "RETRO_ENVIRONMENT_GET_LED_INTERFACE", [this](void *data) -> bool {
        auto ptr = static_cast<retro_led_interface *>(data);
        ptr->set_led_state = [](int led, int state) { spdlog::trace("Core set LED {} to state {}", led, state); };
        return true;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE] = {"RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE",
                                                             [this](void *data) -> bool {
                                                               auto value = static_cast<int *>(data);
                                                               *value = 1 << 0 | 1 << 1;
                                                               return true;
                                                             }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_MIDI_INTERFACE] = {"RETRO_ENVIRONMENT_GET_MIDI_INTERFACE",
                                                         [this](void *data) -> bool { return true; }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_FASTFORWARDING] = {"RETRO_ENVIRONMENT_GET_FASTFORWARDING",
                                                         [this](void *data) -> bool {
                                                           // TODO: Get from video provider?
                                                           auto ptr = static_cast<bool *>(data);
                                                           *ptr = fastforwarding;
                                                           return true;
                                                         }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_TARGET_REFRESH_RATE] = {"RETRO_ENVIRONMENT_GET_TARGET_REFRESH_RATE",
                                                              [this](void *data) -> bool { return true; }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_INPUT_BITMASKS] = {"RETRO_ENVIRONMENT_GET_INPUT_BITMASKS",
                                                         [this](void *data) -> bool { return true; }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION] = {"RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION",
                                                               [this](void *data) -> bool {
                                                                 auto ptr = static_cast<unsigned *>(data);
                                                                 *ptr = 2;
                                                                 return true;
                                                               }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_CORE_OPTIONS] = {
      "RETRO_ENVIRONMENT_SET_CORE_OPTIONS", [this](void *data) -> bool {
        auto ptr = static_cast<retro_core_option_definition **>(data);

        auto configProvider = m_configurationProvider;
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
        return true;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL] = {
      "RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL", [this](void *data) -> bool {
        auto ptr = static_cast<retro_core_options_intl *>(data);

        auto configProvider = m_configurationProvider;
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

        return true;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY] = {
      "RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY", [this](void *data) -> bool {
        auto ptr = static_cast<retro_core_option_display *>(data);

        auto configProvider = m_configurationProvider;
        if (!configProvider) {
          return false;
        }

        configProvider->setOptionVisibility(ptr->key, ptr->visible);
        return true;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER] = {
      "RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER", [this](void *data) -> bool {
        if (!videoReceiver) {
          spdlog::warn("No video receiver set; cannot get preferred HW render");
          return false;
        }
        *static_cast<unsigned *>(data) = videoReceiver->getPreferredHwRender();
        return true;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_DISK_CONTROL_INTERFACE_VERSION] = {
      "RETRO_ENVIRONMENT_GET_DISK_CONTROL_INTERFACE_VERSION", [this](void *data) -> bool {
        *static_cast<unsigned *>(data) = 1;
        return true;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE] = {
      "RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE", [this](void *data) -> bool {
        // Extended interface (adds disc labels/paths); we use its shared base
        // functions for index-based swapping. Stored by value like the plain one
        if (const auto *cb = static_cast<const retro_disk_control_ext_callback *>(data)) {
          m_diskControlExt = *cb;
          m_hasDiskControlExt = true;
        }
        return true;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION] = {"RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION",
                                                                    [this](void *data) -> bool {
                                                                      auto ptr = static_cast<unsigned *>(data);
                                                                      *ptr = 1;
                                                                      return true;
                                                                    }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_MESSAGE_EXT] = {"RETRO_ENVIRONMENT_SET_MESSAGE_EXT", [this](void *data) -> bool {
                                                        auto ptr = static_cast<retro_message_ext *>(data);

                                                        // TODO
                                                        // printf("Msg: %s\n", ptr->msg);
                                                        return false;
                                                      }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK] = {
      "RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK", [this](void *data) -> bool {
        spdlog::info("Intentionally declining SET_AUDIO_BUFFER_STATUS_CALLBACK since Firelight implements DRC");
        return false;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_MINIMUM_AUDIO_LATENCY] = {"RETRO_ENVIRONMENT_SET_MINIMUM_AUDIO_LATENCY",
                                                                [this](void *data) -> bool {
                                                                  // Not needed as we implement dynamic rate control
                                                                  return true;
                                                                }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDE] = {
      "RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDE", [this](void *data) -> bool {
        auto ptr = static_cast<retro_system_content_info_override *>(data);
        for (int i = 0; i < 100; ++i) {
          auto info = ptr[i];
          if (info.extensions == nullptr) {
            break;
          }
        }
        return true;
        // break;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_GAME_INFO_EXT] = {
      "RETRO_ENVIRONMENT_GET_GAME_INFO_EXT", [this](void *data) -> bool {
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
      }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2] = {
      "RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2", [this](void *data) -> bool {
        auto ptr = static_cast<retro_core_options_v2 *>(data);

        auto configProvider = m_configurationProvider;
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
            option.categoryLabel = it != categoryLabels.end() ? it->second : opt.category_key;
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
        return true;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL] = {
      "RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL", [this](void *data) -> bool {
        // TODO
        auto ptr = static_cast<retro_core_options_v2_intl *>(data);

        auto configProvider = m_configurationProvider;
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
            option.categoryLabel = it != categoryLabels.end() ? it->second : opt.category_key;
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
        return true;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK] = {
      "RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK", [this](void *data) -> bool {
        auto ptr = static_cast<retro_core_options_update_display_callback *>(data);
        ptr->callback = []() {
          return true; // TODO I think I actually need to store the callback
          // instead lol
        };
        return false;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_VARIABLE] = {"RETRO_ENVIRONMENT_SET_VARIABLE", [this](void *data) -> bool {
                                                     // TODO: Implement
                                                     return true;
                                                   }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_THROTTLE_STATE] = {"RETRO_ENVIRONMENT_GET_THROTTLE_STATE",
                                                         [this](void *data) -> bool {
                                                           auto ptr = static_cast<retro_throttle_state *>(data);
                                                           // Not needed as far as I'm aware
                                                           return false;
                                                         }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_SAVESTATE_CONTEXT] = {"RETRO_ENVIRONMENT_GET_SAVESTATE_CONTEXT",
                                                            [this](void *data) -> bool {
                                                              auto ptr = static_cast<retro_savestate_context *>(data);
                                                              *ptr = RETRO_SAVESTATE_CONTEXT_NORMAL;
                                                              return true;
                                                            }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_SUPPORT] = {
      "RETRO_ENVIRONMENT_GET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_SUPPORT", [this](void *data) -> bool {
        auto ptr = static_cast<retro_hw_render_context_negotiation_interface_type *>(data);
        *ptr = RETRO_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_VULKAN;
        return true;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_JIT_CAPABLE] = {"RETRO_ENVIRONMENT_GET_JIT_CAPABLE", [this](void *data) -> bool {
                                                        auto ptr = (bool *)data;
                                                        *ptr = false; // TODO
                                                        return false;
                                                      }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_MICROPHONE_INTERFACE] = {
      "RETRO_ENVIRONMENT_GET_MICROPHONE_INTERFACE", [this](void *data) -> bool {
        auto *iface = static_cast<retro_microphone_interface *>(data);
        if (!m_audioInputProvider || !iface) {
          return false;
        }
        iface->interface_version = RETRO_MICROPHONE_INTERFACE_VERSION;
        iface->open_mic = micOpen;
        iface->close_mic = micClose;
        iface->get_params = micGetParams;
        iface->set_mic_state = micSetState;
        iface->get_mic_state = micGetState;
        iface->read_mic = micRead;
        return true;
      }};
  m_envHandlers[RETRO_ENVIRONMENT_SET_NETPACKET_INTERFACE] = {"RETRO_ENVIRONMENT_SET_NETPACKET_INTERFACE",
                                                              [this](void *data) -> bool { return true; }};
  m_envHandlers[RETRO_ENVIRONMENT_GET_DEVICE_POWER] = {"RETRO_ENVIRONMENT_GET_DEVICE_POWER",
                                                       [this](void *data) -> bool {
                                                         //                auto ptr = (retro_device_power *)
                                                         //                softwareBufData;
                                                         return false;
                                                       }};
}
} // namespace libretro
