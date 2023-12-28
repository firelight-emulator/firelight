//
// Created by alexs on 12/19/2023.
//

#ifndef FIRELIGHT_VIDEO_DATA_RECEIVER_HPP
#define FIRELIGHT_VIDEO_DATA_RECEIVER_HPP

#include <cstddef>
#include <functional>
typedef void (*proc_address_t)();
typedef void (*context_reset_func)();

class CoreVideoDataReceiver {

public:
  virtual ~CoreVideoDataReceiver() = default;
  virtual void receive(const void *data, unsigned width, unsigned height,
                       size_t pitch) = 0;
  virtual proc_address_t get_proc_address(const char *sym) = 0;
  virtual void set_reset_context_func(context_reset_func) = 0;
  virtual uintptr_t get_current_framebuffer_id() = 0;
  virtual void set_system_av_info(retro_system_av_info *info) = 0;
};

#endif // FIRELIGHT_VIDEO_DATA_RECEIVER_HPP
