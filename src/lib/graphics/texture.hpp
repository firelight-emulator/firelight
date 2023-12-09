//
// Created by alexs on 11/13/2023.
//

#ifndef FIRELIGHT_TEXTURE_HPP
#define FIRELIGHT_TEXTURE_HPP

namespace FL::Graphics {

class Texture {
public:
  virtual void setContent(const void *data, unsigned width, unsigned height,
                          unsigned pitch) = 0;
  virtual void resize(int w, int h) = 0;
};

} // namespace FL::Graphics

#endif // FIRELIGHT_TEXTURE_HPP
