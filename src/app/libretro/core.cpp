//
// Created by alexs on 10/12/2023.
//

#include "core.hpp"
#include "SDL2/SDL.h"
#include "retropad.hpp"
#include <cstdarg>

#include "../audio_manager.hpp"
// #include "nlohmann/json.hpp"

// using json = nlohmann::json;

namespace libretro {

static const int THINGY = 5;

void log(enum retro_log_level level, const char *fmt, ...) {
  char msg[4096] = {0};
  va_list va;
  va_start(va, fmt);
  vsnprintf(msg, sizeof(msg), fmt, va);
  va_end(va);
  printf("%s", msg);
}

// Only supports one core at a time for now, but, eh.
static Core *currentCore;

static int16_t inputStateCallback(unsigned port, unsigned device,
                                  unsigned index, unsigned id) {
  if (currentCore == nullptr) {
    // TODO: Report some error
    return 0;
  }

  const auto manager = currentCore->getRetropadProvider();
  const auto controllerOpt = manager->getRetropadForPlayer(port);
  if (!controllerOpt.has_value()) {
    return 0;
  }

  IRetroPad *controller = controllerOpt.value();

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
    return controller->isButtonPressed(static_cast<IRetroPad::Button>(id));
  }

  return 0;
}

static void videoCallback(const void *data, unsigned width, unsigned height,
                          size_t pitch) {
  currentCore->videoReceiver->receive(data, width, height, pitch);
  //  printf("video callback w: %u, h: %u\n", width, height);
  //  currentCore->getVideo()->refreshCoreVideo(data, width, height, pitch);
}

static bool envCallback(unsigned cmd, void *data) {
  return currentCore->handleEnvironmentCall(cmd, data);
}

bool Core::handleEnvironmentCall(unsigned int cmd, void *data) {
  switch (cmd) {
  case RETRO_ENVIRONMENT_SET_ROTATION:
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_SET_ROTATION");
    //    this->video->setRotation(*(unsigned *)data);
    return true;
  case (3 | 0x800000): {
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_GET_CLEAR_ALL_THREAD_WAITS_CB");
    auto ptr = (retro_environment_t *)data;
    *ptr = [](unsigned int cmd, void *data) {
      printf("Calling weirdo callback");
      return true;
    };
    return true;
  }
  case RETRO_ENVIRONMENT_GET_OVERSCAN:
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_GET_OVERSCAN");
    this->recordPotentialAPIViolation(
        "Using deprecated environment call GET_OVERSCAN");
    *(bool *)data = false;
    return true;
  case RETRO_ENVIRONMENT_GET_CAN_DUPE: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_GET_CAN_DUPE");
    *(bool *)data = true;
    return true;
  }
  case RETRO_ENVIRONMENT_SET_MESSAGE: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_SET_MESSAGE");
    auto ptr = (retro_message *)data;
    // TODO: Do something to queue message
    printf("Got message for %d frames: %s\n", ptr->frames, ptr->msg);
    return true;
  }
  case RETRO_ENVIRONMENT_SHUTDOWN:
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_SHUTDOWN");
    this->shutdown = *(bool *)data;
    break;
  case RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL:
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL");
    this->performanceLevel = *(unsigned *)data;
    break;
  case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY");
    if (this->systemDirectory.empty()) {
      return false;
    }

    auto ptr = (const char **)data;
    *ptr = &this->systemDirectory[0];
    return true;
  }
  case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_SET_PIXEL_FORMAT");
    printf("pixelformat: %p\n", (retro_pixel_format *)data);
    //    this->video->setPixelFormat((retro_pixel_format *)data);
    return true;
  case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS");
    auto ptr = (retro_input_descriptor *)data;
    // TODO sane default
    for (int i = 0; i < 100; ++i) {
      auto descriptor = ptr[i];

      printf("description: %s, port: %d, device: %d, index: %d, id: %d\n",
             descriptor.description, descriptor.port, descriptor.device,
             descriptor.index, descriptor.id);
      if (descriptor.description == nullptr) {
        break;
      }

      this->inputDescriptors.push_back(descriptor);

      if (i == 100) {
        this->recordPotentialAPIViolation("Over 100 input descriptors");
      }
    }
    return true;
  }
  case RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK");
    auto ptr = (retro_keyboard_callback *)data;
    ptr->callback = [](bool down, unsigned keycode, uint32_t character,
                       uint16_t key_modifiers) {
      printf("Calling the keyboard callback\n");
    };
    return true;
  }
  case RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE: {
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE");
    auto ptr = (retro_disk_control_callback *)data;
    ptr->set_eject_state = nullptr;
    ptr->get_eject_state = nullptr;
    ptr->get_image_index = nullptr;
    ptr->set_image_index = nullptr;
    ptr->get_num_images = nullptr;
    ptr->replace_image_index = nullptr;
    ptr->add_image_index = nullptr;
    return false;
  }
  case RETRO_ENVIRONMENT_SET_HW_RENDER: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_SET_HW_RENDER");
    // TODO I think this is actually mostly stuff informing the frontend
    auto *renderCallback = static_cast<retro_hw_render_callback *>(data);

    renderCallback->get_proc_address =
        [](const char *sym) -> retro_proc_address_t {
      auto add = currentCore->videoReceiver->get_proc_address(sym);
      // printf("address for %s: %p\n", sym, add);
      return add;
    };

    renderCallback->get_current_framebuffer = []() {
      printf("calling get current framebuffer\n");
      auto val = currentCore->videoReceiver->get_current_framebuffer_id();
      printf("framebuffer: %llu\n", val);
      return val;
    };

    currentCore->videoReceiver->set_reset_context_func(
        renderCallback->context_reset);

    return true;
  }
  case RETRO_ENVIRONMENT_GET_VARIABLE: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_GET_VARIABLE");
    auto ptr = (retro_variable *)data;
    for (const auto &opt : this->options) {
      if (strcmp(opt.key, ptr->key) == 0) {
        ptr->value = opt.currentValue;
        return true;
      }
    }
    return true;
  }
  case RETRO_ENVIRONMENT_SET_VARIABLES: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_SET_VARIABLES");
    auto ptr = (retro_variable *)data;
    // TODO sane default
    for (int i = 0; i < 100; ++i) {
      auto opt = ptr[i];
      printf("Variable KEY: %s VALUE: %s\n", opt.key, opt.value);
      if (opt.key == nullptr) {
        break;
      }
    }
    return true;
  }
  case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE");
    *(bool *)data = false; // TODO: actually implement
    return true;
  }
  case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME");
    this->canRunWithNoGame = *(bool *)data;
  }
  case RETRO_ENVIRONMENT_GET_LIBRETRO_PATH: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_GET_LIBRETRO_PATH");
    if (this->libretroPath.empty()) {
      return false;
    }
    *(const char **)data = &this->libretroPath[0];
    return true;
  }
  case RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK: {
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK");
    //    this->video->setFrameTimeCallback((retro_frame_time_callback
    //    *)data);
    return true;
  }
  case RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK");
    auto ptr = (retro_audio_callback *)data;
    ptr->callback = nullptr;
    ptr->set_state = nullptr;
    return false;
  }
  case RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE");
    auto ptr = (retro_rumble_interface *)data;
    ptr->set_rumble_state = [](unsigned port, enum retro_rumble_effect effect,
                               uint16_t strength) {
      printf("Calling rumble callback\n");
      return true;
    };
    return true;
  }
  case RETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES: {
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES");
    auto ptr = (uint64_t *)data;
    //* Gets a bitmask telling which device type are expected to be
    // * handled properly in a call to retro_input_state_t.
    // * Devices which are not handled or recognized always return
    // * 0 in retro_input_state_t.
    // * Example bitmask: caps = (1 << RETRO_DEVICE_JOYPAD) | (1 <<
    // RETRO_DEVICE_ANALOG).

    *ptr = (1 << RETRO_DEVICE_JOYPAD) | (1 << RETRO_DEVICE_ANALOG);
    return false;
  }
  case RETRO_ENVIRONMENT_GET_SENSOR_INTERFACE: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_GET_SENSOR_INTERFACE");
    auto ptr = (retro_sensor_interface *)data;
    ptr->set_sensor_state = nullptr;
    ptr->get_sensor_input = nullptr;
    return false;
  }
  case RETRO_ENVIRONMENT_GET_CAMERA_INTERFACE: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_GET_CAMERA_INTERFACE");
    // todo: actually support camera?
    auto ptr = (retro_camera_callback *)data;
    ptr->start = []() {
      printf("Here's where I WOULD start the camera driver\n");
      return false;
    };
    ptr->stop = []() {
      printf("Here's where I WOULD stop the camera driver\n");
    };
    return true;
  }
  case RETRO_ENVIRONMENT_GET_LOG_INTERFACE: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_GET_LOG_INTERFACE");
    auto ptr = (retro_log_callback *)data;
    ptr->log = log;
    return true;
  }
  case RETRO_ENVIRONMENT_GET_PERF_INTERFACE: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_GET_PERF_INTERFACE");
    auto ptr = (retro_perf_callback *)data;
    // Return current time microseconds (unix epoch?)
    ptr->get_time_usec = []() {
      printf("Getting time usec");
      return retro_time_t(0);
    };
    // Returns a bit-mask of detected CPU features (RETRO_SIMD_*)
    ptr->get_cpu_features = []() {
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
    ptr->get_perf_counter = []() {
      printf("Getting performance counter");
      return retro_perf_tick_t(0);
    };

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
    ptr->perf_log = []() { printf("Logging performance counters\n"); };

    return false;
  }
  case RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE: {
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE");
    auto ptr = (retro_location_callback *)data;
    ptr->start = nullptr;
    ptr->stop = nullptr;
    ptr->get_position = nullptr;
    ptr->set_interval = nullptr;
    ptr->initialized = nullptr;
    ptr->deinitialized = nullptr;
    return false;
  }
  case RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY: {
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY");
    auto ptr = (const char **)data;
    *ptr = &this->coreAssetsDirectory[0];
    return true;
  }
  case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY");
    auto ptr = (const char **)data;
    *ptr = &this->saveDirectory[0]; // TODO
    return true;
  }
  case RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO: {
    this->environmentCalls.emplace_back("RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO");
    videoReceiver->set_system_av_info((retro_system_av_info *)data);
    //    this->video->setGameGeometry(&this->retroSystemAVInfo->geometry);
    return true;
  }
  case RETRO_ENVIRONMENT_SET_PROC_ADDRESS_CALLBACK:
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_SET_PROC_ADDRESS_CALLBACK");
    break;
  case RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO");
    auto ptr = (retro_subsystem_info *)data;
    for (int i = 0; i < 100; ++i) {
      auto ssInfo = ptr[i];
      if (ssInfo.desc == nullptr) {
        break;
      }

      this->subsystemInfo.push_back(ssInfo);
      if (i == 100) {
        this->recordPotentialAPIViolation("Over 100 subsystems");
      }
    }
    return true;
  }
  case RETRO_ENVIRONMENT_SET_CONTROLLER_INFO: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_SET_CONTROLLER_INFO");
    auto ptr = (retro_controller_info *)data;
    for (unsigned i = 0; i < ptr->num_types; ++i) {
      auto info = ptr->types[i];
      if (info.desc == nullptr) {
        break;
      }

      this->controllerInfo.push_back(info);
      if (i == 100) {
        this->recordPotentialAPIViolation("Over 100 controller infos");
      }
    }
    return true;
  }
  case RETRO_ENVIRONMENT_SET_MEMORY_MAPS: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_SET_MEMORY_MAPS");
    auto ptr = (retro_memory_map *)data;
    for (unsigned i = 0; i < ptr->num_descriptors; ++i) {
      if (ptr->descriptors[i].ptr == nullptr) {
        break;
      }
      this->memoryMaps.push_back(ptr->descriptors[i]);
    }
    return true;
  }
  case RETRO_ENVIRONMENT_SET_GEOMETRY: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_SET_GEOMETRY");
    this->retroSystemAVInfo->geometry = *(retro_game_geometry *)data;
    //    this->video->setGameGeometry(&this->retroSystemAVInfo->geometry);
    return true;
  }
  case RETRO_ENVIRONMENT_GET_USERNAME: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_GET_USERNAME");
    auto ptr = (const char **)data;
    *ptr = &this->username[0];
    return true;
  }
  case RETRO_ENVIRONMENT_GET_LANGUAGE: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_GET_LANGUAGE");
    // TODO: Set by user
    auto ptr = (retro_language *)data;
    *ptr = RETRO_LANGUAGE_ENGLISH;
    return true;
  }
  case RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER:
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER");
    break;
  case RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE: {
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE");
    auto ptr = (retro_hw_render_interface *)data;
    ptr->interface_type = RETRO_HW_RENDER_INTERFACE_DUMMY;
    ptr->interface_version = 0;
  }
  case RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS: {
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS");
    this->supportsAchievements = *(bool *)data;
    return true;
  }
  case RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE:
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE");
    break;
  case RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS:
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS");
    break;
  case RETRO_ENVIRONMENT_SET_HW_SHARED_CONTEXT:
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_SET_HW_SHARED_CONTEXT");
    return true;
  case RETRO_ENVIRONMENT_GET_VFS_INTERFACE: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_GET_VFS_INTERFACE");
    // TODO: Do something here to ensure this was called before we give the
    // core any paths
    auto ptr = (retro_vfs_interface_info *)data;
    printf("Required VFS interface version: %d\n",
           ptr->required_interface_version);
    ptr->iface = nullptr;
    return true;
  }
  case RETRO_ENVIRONMENT_GET_LED_INTERFACE: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_GET_LED_INTERFACE");
    auto ptr = (retro_led_interface *)data;
    ptr->set_led_state = [](int led, int state) {
      printf("Setting LED %d to state %d\n", led, state);
    };
    return true;
  }
  case RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE: {
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE");
    int *value = (int *)data;
    *value = 1 << 0 | 1 << 1;
    return true;
  }
  case RETRO_ENVIRONMENT_GET_MIDI_INTERFACE:
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_GET_MIDI_INTERFACE");
    break;
  case RETRO_ENVIRONMENT_GET_FASTFORWARDING: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_GET_FASTFORWARDING");
    auto ptr = (bool *)data;
    *ptr = this->fastforwarding;
    return true;
  }
  case RETRO_ENVIRONMENT_GET_TARGET_REFRESH_RATE:
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_GET_TARGET_REFRESH_RATE");
    break;
  case RETRO_ENVIRONMENT_GET_INPUT_BITMASKS:
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_GET_INPUT_BITMASKS");
    break;
  case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION: {
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION");
    // TODO: Set this behind some user-settable flag?
    auto ptr = (unsigned *)data;
    *ptr = 2;
    return true;
  }
  case RETRO_ENVIRONMENT_SET_CORE_OPTIONS: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_SET_CORE_OPTIONS");
    auto ptr = (retro_core_option_definition **)data;
    // TODO sane default
    for (int i = 0; i < 200; ++i) {
      auto opt = *ptr[i];
      printf("OPTION KEY: %s\n", opt.key);
      if (opt.key == nullptr) {
        break;
      }

      // TODO: pointer?
      CoreOption coreOption(opt);
      this->options.push_back(coreOption);
    }
    return true;
  }
  case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL");
    auto ptr = (retro_core_options_intl *)data;
    // TODO sane default
    for (int i = 0; i < 200; ++i) {
      auto opt = ptr->us[i];
      if (opt.key == nullptr) {
        break;
      }

      CoreOption coreOption(opt);
      this->options.push_back(coreOption);
    }
    return true;
  }
  case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY: {
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY");
    auto ptr = (retro_core_option_display *)data;
    for (auto opt : this->options) {
      if (strcmp(ptr->key, opt.key) == 0) {
        opt.displayToUser = ptr->visible;
        return true;
      }
    }
    return false;
  }
  case RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER:
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER");
    *(unsigned *)data = RETRO_HW_CONTEXT_OPENGL;
    return true;
  case RETRO_ENVIRONMENT_GET_DISK_CONTROL_INTERFACE_VERSION:
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_GET_DISK_CONTROL_INTERFACE_VERSION");
    break;
  case RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE:
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE");
    break;
  case RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION:
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION");
    break;
  case RETRO_ENVIRONMENT_SET_MESSAGE_EXT:
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_SET_MESSAGE_EXT");
    break;
  case RETRO_ENVIRONMENT_GET_INPUT_MAX_USERS:
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_GET_INPUT_MAX_USERS");
    break;
  case RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK:
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK");
    break;
  case RETRO_ENVIRONMENT_SET_MINIMUM_AUDIO_LATENCY:
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_SET_MINIMUM_AUDIO_LATENCY");
    break;
  case RETRO_ENVIRONMENT_SET_FASTFORWARDING_OVERRIDE:
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_SET_FASTFORWARDING_OVERRIDE");
    break;
  case RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDE:
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDE");
    break;
  case RETRO_ENVIRONMENT_GET_GAME_INFO_EXT:
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_GET_GAME_INFO_EXT");
    break;
  case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2");
    auto ptr = (retro_core_options_v2 *)data;
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

      CoreOption coreOption(opt);
      this->options.push_back(coreOption);
    }
    return true;
  }
  case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL: {
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL");
    // TODO
    auto ptr = (retro_core_options_v2_intl *)data;
    for (int i = 0; i < 100; ++i) {
      auto opt = ptr->us->categories[i];
      if (opt.key == nullptr) {
        break;
      }
    }
    for (int i = 0; i < 200; ++i) {
      auto opt = ptr->us->definitions[i];
      if (opt.key == nullptr) {
        break;
      }

      CoreOption coreOption(opt);
      this->options.push_back(coreOption);
    }
    return true;
  }
  case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK: {
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK");
    auto ptr = (retro_core_options_update_display_callback *)data;
    ptr->callback = []() {
      return true; // TODO I think I actually need to store the callback
                   // instead lol
    };
    return true;
  }
  case RETRO_ENVIRONMENT_SET_VARIABLE: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_SET_VARIABLE");
    auto ptr = (retro_variable *)data;
    if (ptr == nullptr) {
      return true;
    }

    for (auto opt : this->options) {
      if (strcmp(opt.key, ptr->key) == 0) {
        for (auto v : opt.values) {
          if (strcmp(ptr->value, v.value) == 0) {
            opt.currentValue = ptr->value;
            return true;
          }
          this->recordPotentialAPIViolation(
              "SET_VARIABLE with unknown value for key TODO");
        }
        // TODO: Make sure value is one of the allowed strings
      }
    }

    return true;
  }
  case RETRO_ENVIRONMENT_GET_THROTTLE_STATE: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_GET_THROTTLE_STATE");
    auto ptr = (retro_throttle_state *)data;
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
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_GET_SAVESTATE_CONTEXT");
    auto ptr = (retro_savestate_context *)data;
    *ptr = RETRO_SAVESTATE_CONTEXT_NORMAL;
    return true;
  }
  case RETRO_ENVIRONMENT_GET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_SUPPORT:
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_GET_HW_RENDER_CONTEXT_"
                                     "NEGOTIATION_INTERFACE_SUPPORT");
    break;
  case RETRO_ENVIRONMENT_GET_JIT_CAPABLE: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_GET_JIT_CAPABLE");
    auto ptr = (bool *)data;
    *ptr = true; // TODO
    return true;
  }
  case RETRO_ENVIRONMENT_GET_MICROPHONE_INTERFACE: {
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_GET_MICROPHONE_INTERFACE");
    auto ptr = (retro_microphone_interface *)data;
    ptr->interface_version = 0;
    return false;
  }
  case RETRO_ENVIRONMENT_SET_NETPACKET_INTERFACE:
    this->environmentCalls.push_back(
        "RETRO_ENVIRONMENT_SET_NETPACKET_INTERFACE");
    break;
  case RETRO_ENVIRONMENT_GET_DEVICE_POWER: {
    this->environmentCalls.push_back("RETRO_ENVIRONMENT_GET_DEVICE_POWER");
    //                auto ptr = (retro_device_power *) softwareBufData;
    return false;
  }
  default:
    printf("Unimplemented env command: %d\n", cmd);
    this->environmentCalls.push_back("UNIMPLEMENTED");
  }
  return false;
}

template <typename T> static T loadRetroFunc(void *dll, const char *name) {
  // TODO error checking
  auto result = reinterpret_cast<T>(SDL_LoadFunction(dll, name));
  if (result == nullptr) {
    std::cout << SDL_GetError() << std::endl;
  }
  return result;
}

//    std::basic_string<char> Core::dumpJson() {
//        json j;
////        j["rotation"] = this->rotation;
////        j["overscan"] = this->useOverscan;
////        j["can_dupe_frames"] = this->canDupeFrames;
//        // set message
//        j["should_shutdown"] = this->shutdown;
//        j["performance_level"] = this->performanceLevel;
//        j["system_directory"] = this->systemDirectory;
////        j["pixel_format"] = *this->pixelFormat;
//        j["supports_no_game"] = this->canRunWithNoGame;
////        j["libretro_path"] = this->libretroPath;
////        j["core_assets_directory"] = this->coreAssetsDirectory;
////        j["save_directory"] = this->saveDirectory;
//
//        json opts = json::array();
//        for (auto o: this->options) {
//            opts.push_back(o.dumpJson());
//        }
//        j["options"] = opts;
//
//        json inDesc = json::array();
//        for (auto i: this->inputDescriptors) {
//            json in;
//            in["id"] = i.id;
//            in["index"] = i.index;
//            in["device"] = i.device;
//            in["description"] = i.description;
//            in["port"] = i.port;
//            inDesc.push_back(in);
//        }
//
//        j["input_descriptors"] = inDesc;
//
//        json subsys = json::array();
//        for (auto sub: this->subsystemInfo) {
//            json ssInfo;
//            ssInfo["id"] = sub.id;
//            ssInfo["description"] = sub.desc;
//            ssInfo["identifier"] = sub.ident;
//            ssInfo["num_roms"] = sub.num_roms;
//            // TODO: ROMS
//
//            subsys.push_back(ssInfo);
//        }
//
//        j["subsystem_info"] = subsys;
//
////        if (this->hardwareRenderCallback) {
////            json hwRender;
////            hwRender["context_type"] =
/// this->hardwareRenderCallback->context_type; /            hwRender["depth"]
/// = this->hardwareRenderCallback->depth; /            hwRender["stencil"] =
/// this->hardwareRenderCallback->stencil; / hwRender["bottom_left_origin"] =
/// this->hardwareRenderCallback->bottom_left_origin; /
/// hwRender["version_major"] = this->hardwareRenderCallback->version_major; /
/// hwRender["version_minor"] = this->hardwareRenderCallback->version_minor; /
/// hwRender["cache_context"] = this->hardwareRenderCallback->cache_context; /
/// hwRender["debug_context"] = this->hardwareRenderCallback->debug_context;
////
////            j["hw_render_callback"] = hwRender;
////        }
//
//        json memMaps = json::array();
//        for (auto m: this->memoryMaps) {
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
//            memMaps.push_back(map);
//        }
//
//        j["memory_descriptors"] = memMaps;
//
//        json envCalls = json::array();
//        for (auto e: this->environmentCalls) {
//            envCalls.push_back(e);
//        }
//
//        j["environment_calls"] = envCalls;
//
//        return j.dump();
//    }

Core::Core(const std::string &libPath) {
  this->dll = SDL_LoadObject(libPath.c_str());
  if (this->dll == nullptr) {
    // Check error
  }

  this->symRetroInit = loadRetroFunc<void (*)()>(this->dll, "retro_init");
  this->symRetroDeinit = loadRetroFunc<void (*)()>(this->dll, "retro_deinit");

  this->symRetroApiVersion =
      loadRetroFunc<unsigned int (*)()>(this->dll, "retro_api_version");
  this->symRetroGetSystemInfo = loadRetroFunc<void (*)(retro_system_info *)>(
      this->dll, "retro_get_system_info");
  this->symRetroGetSystemAVInfo =
      loadRetroFunc<void (*)(retro_system_av_info *)>(
          this->dll, "retro_get_system_av_info");
  this->symRetroSetControllerPortDevice =
      loadRetroFunc<void (*)(unsigned int, unsigned int)>(
          this->dll, "retro_set_controller_port_device");

  this->symRetroReset = loadRetroFunc<void (*)()>(this->dll, "retro_reset");
  this->symRetroRun = loadRetroFunc<void (*)()>(this->dll, "retro_run");
  this->symRetroSerializeSize =
      loadRetroFunc<size_t (*)()>(this->dll, "retro_serialize_size");
  this->symRetroSerialize =
      loadRetroFunc<bool (*)(void *, size_t)>(this->dll, "retro_serialize");
  this->symRetroUnserialize = loadRetroFunc<bool (*)(const void *, size_t)>(
      this->dll, "retro_unserialize");
  this->symRetroCheatReset =
      loadRetroFunc<void (*)()>(this->dll, "retro_cheat_reset");
  this->symRetroCheatSet =
      loadRetroFunc<void (*)(unsigned, bool, const char *)>(this->dll,
                                                            "retro_cheat_set");
  this->symRetroLoadGame = loadRetroFunc<bool (*)(const retro_game_info *)>(
      this->dll, "retro_load_game");
  this->symRetroLoadGameSpecial =
      loadRetroFunc<bool (*)(unsigned int, const retro_game_info *, size_t)>(
          this->dll, "retro_load_game_special");
  this->symRetroUnloadGame =
      loadRetroFunc<void (*)()>(this->dll, "retro_unload_game");
  this->symRetroGetRegion =
      loadRetroFunc<unsigned int (*)()>(this->dll, "retro_get_region");
  this->symRetroGetMemoryData = loadRetroFunc<void *(*)(unsigned int)>(
      this->dll, "retro_get_memory_data");
  this->symRetroGetMemoryDataSize = loadRetroFunc<size_t (*)(unsigned int)>(
      this->dll, "retro_get_memory_size");

  this->retroSystemInfo = new retro_system_info;
  this->retroSystemAVInfo = new retro_system_av_info;

  //  this->video = new Video(gfxDriver);

  currentCore = this; // todo prob different namespace

  // The next several methods load the callback symbols from the library and
  // set them to our methods defined above. Since we never change those
  // callbacks, we don't need to store the symbols.
  loadRetroFunc<RetroSetEnvironment>(dll, "retro_set_environment")(envCallback);
  loadRetroFunc<RetroSetVideoRefresh>(dll,
                                      "retro_set_video_refresh")(videoCallback);
  loadRetroFunc<RetroSetAudioSample>(dll, "retro_set_audio_sample")(
      [](int16_t left, int16_t right) {});

  auto processAudioLambda = [](const int16_t *data, size_t frames) -> size_t {
    auto core = currentCore;
    if (core == nullptr) {
      printf("core was null in libretro audio callback\n");
      return frames;
    }

    SDL_QueueAudio(core->audioReceiver->get_audio_device(), data, frames * 4);

    return frames;
  };

  loadRetroFunc<RetroSetAudioSampleBatch>(dll, "retro_set_audio_sample_batch")(
      processAudioLambda);
  loadRetroFunc<RetroInputPoll>(dll, "retro_set_input_poll")([]() {});
  loadRetroFunc<RetroInputState>(dll,
                                 "retro_set_input_state")(inputStateCallback);

  //  symRetroSetControllerPortDevice(0, RETRO_DEVICE_ANALOG);
}

Core::~Core() {
  SDL_UnloadObject(dll);
  // close DL handle
  // need to close symbol handles or free their memory?
  delete this->retroSystemInfo;
  delete this->retroSystemAVInfo;
  //  delete this->video;
}

bool Core::loadGame(Game *game) {
  retro_game_info info{};
  info.path = game->getPath().c_str();
  info.data = game->getData();
  info.size = game->getSize();
  info.meta = "";
  // TODO: meta?
  auto result = this->symRetroLoadGame(&info);

  this->symRetroGetSystemInfo(this->retroSystemInfo);
  this->symRetroGetSystemAVInfo(this->retroSystemAVInfo);
  videoReceiver->set_system_av_info(this->retroSystemAVInfo);
  //  this->video->setGameGeometry(&this->retroSystemAVInfo->geometry);
  printf("New Audio Sample Rate: %f \n", retroSystemAVInfo->timing.sample_rate);
  audioReceiver->initialize(retroSystemAVInfo->timing.sample_rate);
  return result;
}

void Core::init() { symRetroInit(); }

void Core::deinit() { this->symRetroDeinit(); }

void Core::reset() { this->symRetroReset(); }

void Core::run(double deltaTime) { this->symRetroRun(); }

void Core::setSystemDirectory(string frontendSystemDirectory) {
  this->systemDirectory = frontendSystemDirectory;
}

void Core::setSaveDirectory(const string &frontendSaveDirectory) {
  this->saveDirectory = frontendSaveDirectory;
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

void Core::set_video_receiver(IVideoDataReceiver *receiver) {
  videoReceiver = receiver;
}

void Core::setRetropadProvider(IRetropadProvider *provider) {
  m_retropadProvider = provider;
}

IRetropadProvider *Core::getRetropadProvider() { return m_retropadProvider; }

void Core::set_audio_receiver(CoreAudioDataReceiver *receiver) {
  audioReceiver = receiver;
}

} // namespace libretro