//
// Created by alexs on 11/8/2023.
//

#ifndef FIRELIGHT_OPEN_GL_DRIVER_HPP
#define FIRELIGHT_OPEN_GL_DRIVER_HPP

#include "../../math/bbox.hpp"
#include "../character.hpp"
#include "../color.hpp"
#include "../driver.hpp"
#include "GL/glew.h"
#include "freetype/freetype.h"
#include "open_gl_texture.hpp"
#include <array>
#include <unordered_map>

namespace FL::Graphics {

class OpenGLDriver : public FL::Graphics::Driver {
public:
  OpenGLDriver(int w, int h);
  ~OpenGLDriver();

  Texture *newTexture() override;

  void drawRectangle(int x, int y, int w, int h, Color color) override;
  void drawText(std::string text, int x, int y, Color color) override;
  void calculateTextBounds(const std::string &text, int &width,
                           int &height) override;

  void drawTexture(Texture *texture, FL::Math::Box displayBox) override;
  void setDisplayArea(FL::Math::Box box) override;
  int viewportWidth, viewportHeight = 0;

private:
  FL::Math::Box displayArea;
  FT_Library fontLib;
  FT_Face fontFace;
  GLuint fontTex;
  GLuint vao;
  GLuint vbo;
  GLuint program;

  GLuint texVao;
  GLuint texVbo;
  [[nodiscard]] std::array<float, 8> calculateVertexArray(int x, int y, int w,
                                                          int h) const;
  [[nodiscard]] std::array<float, 16>
  calculateTexVertexArray(int x, int y, int w, int h) const;
  std::unordered_map<char, Graphics::Character> characters;
};

} // namespace FL::Graphics

#endif // FIRELIGHT_OPEN_GL_DRIVER_HPP
