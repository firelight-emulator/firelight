//
// Created by alexs on 11/30/2023.
//

#ifndef FIRELIGHT_FRAMEBUFFER_HPP
#define FIRELIGHT_FRAMEBUFFER_HPP

class Framebuffer {
public:
  virtual void setContent(const void *data, unsigned width, unsigned height,
                          unsigned pitch) = 0;
  virtual void resize(int w, int h) = 0;
};

#endif // FIRELIGHT_FRAMEBUFFER_HPP
