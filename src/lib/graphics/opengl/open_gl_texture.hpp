//
// Created by alexs on 11/17/2023.
//

#ifndef FIRELIGHT_OPEN_GL_TEXTURE_HPP
#define FIRELIGHT_OPEN_GL_TEXTURE_HPP

#include "../texture.hpp"
#include <GL/glew.h>

namespace FL::Graphics {

class OpenGLTexture : public Texture {
public:
  OpenGLTexture();
  void resize(int w, int h) override;
  void setContent(const void *data, unsigned width, unsigned height,
                  unsigned pitch) override;

private:
  friend class OpenGLDriver;
  GLuint texId = -1;

  void *content = nullptr;
};

} // namespace FL::Graphics

#endif // FIRELIGHT_OPEN_GL_TEXTURE_HPP
