//
// Created by alexs on 11/17/2023.
//

#include "open_gl_texture.hpp"
#include <cstdio>

namespace FL::Graphics {

OpenGLTexture::OpenGLTexture() {
  glGenTextures(1, &texId);
  glBindTexture(GL_TEXTURE_2D, texId);

  auto err = glGetError();
  if (err != GL_NO_ERROR) {
    printf("GL ERROR: %d\n", err);
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  err = glGetError();
  if (err != GL_NO_ERROR) {
    printf("GL ERROR: %d\n", err);
  }

  glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLTexture::resize(int w, int h) {}

void OpenGLTexture::setContent(const void *data, unsigned width,
                               unsigned height, unsigned pitch) {
  printf("setting content. width: %u, height: %u, pitch:  %u\n", width, height,
         pitch);
  glBindTexture(GL_TEXTURE_2D, texId);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, pitch / 2);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGB,
               GL_UNSIGNED_SHORT_5_6_5, data);
  glBindTexture(GL_TEXTURE_2D, 0);
  printf("set content\n");
}

} // namespace FL::Graphics
