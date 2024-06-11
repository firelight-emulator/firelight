#pragma once

#include "libretro/libretro.h"
#include <cstddef>

typedef void (*proc_address_t)();
typedef void (*context_reset_func)();
typedef void (*context_destroy_func)();

namespace firelight::libretro {

class IVideoDataReceiver {
public:
  virtual ~IVideoDataReceiver() = default;
  virtual void receive(const void *data, unsigned width, unsigned height,
                       size_t pitch) = 0;
  virtual proc_address_t getProcAddress(const char *sym) = 0;
  virtual void setResetContextFunc(context_reset_func) = 0;
  virtual void setDestroyContextFunc(context_destroy_func) = 0;
  virtual uintptr_t getCurrentFramebufferId() = 0;
  virtual void setSystemAVInfo(retro_system_av_info *info) = 0;
};

} // namespace firelight::libretro
