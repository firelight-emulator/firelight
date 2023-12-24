//
// Created by alexs on 12/23/2023.
//

#ifndef FRAMEBUFFERHANDLEPROVIDER_HPP
#define FRAMEBUFFERHANDLEPROVIDER_HPP
#include <cstdint>

class FramebufferHandleProvider {
public:
  virtual uintptr_t get_framebuffer_handle() = 0;
  virtual ~FramebufferHandleProvider() = default;
};

#endif // FRAMEBUFFERHANDLEPROVIDER_HPP
