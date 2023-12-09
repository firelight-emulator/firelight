//
// Created by alexs on 11/8/2023.
//

#include "open_gl_driver.hpp"
#include "shaders.hpp"

namespace FL::Graphics {
// compiler combines adjacent strings apparently
static const GLchar *fragmentShader = "#version 330 core\n"
                                      "out vec4 FragColor;\n"

                                      "uniform vec3 color;\n"

                                      "void main()\n"
                                      "{\n"
                                      " FragColor = vec4(color.rgb, 1.0f);\n"
                                      "}\0";

// compiler combines adjacent strings apparently
static const char *vertexShaderSource =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

std::array<float, 8> OpenGLDriver::calculateVertexArray(int x, int y, int w,
                                                        int h) const {
  std::array<float, 8> result = {0};

  auto topLeftX = x;
  auto bottomLeftX = x;
  auto topRightX = x + w;
  auto bottomRightX = x + w;

  auto topLeftY = y;
  auto bottomLeftY = y + h;
  auto topRightY = y;
  auto bottomRightY = y + h;

  result[0] = float(topLeftX) / float(viewportWidth) * 2 - 1;
  result[1] = (float(topLeftY) / float(viewportHeight) * 2 - 1) * -1;
  result[2] = float(bottomLeftX) / float(viewportWidth) * 2 - 1;
  result[3] = (float(bottomLeftY) / float(viewportHeight) * 2 - 1) * -1;

  result[4] = float(topRightX) / float(viewportWidth) * 2 - 1;
  result[5] = (float(topRightY) / float(viewportHeight) * 2 - 1) * -1;
  result[6] = float(bottomRightX) / float(viewportWidth) * 2 - 1;
  result[7] = (float(bottomRightY) / float(viewportHeight) * 2 - 1) * -1;

  return result;
}

std::array<float, 16> OpenGLDriver::calculateTexVertexArray(int x, int y, int w,
                                                            int h) const {
  std::array<float, 16> result = {0};

  auto topLeftX = x;
  auto bottomLeftX = x;
  auto topRightX = x + w;
  auto bottomRightX = x + w;

  auto topLeftY = y;
  auto bottomLeftY = y + h;
  auto topRightY = y;
  auto bottomRightY = y + h;

  result[0] = float(topLeftX) / float(viewportWidth) * 2 - 1;
  result[1] = (float(topLeftY) / float(viewportHeight) * 2 - 1) * -1;
  result[2] = 0;
  result[3] = 0;

  result[4] = float(bottomLeftX) / float(viewportWidth) * 2 - 1;
  result[5] = (float(bottomLeftY) / float(viewportHeight) * 2 - 1) * -1;
  result[6] = 0;
  result[7] = 1.0;

  result[8] = float(topRightX) / float(viewportWidth) * 2 - 1;
  result[9] = (float(topRightY) / float(viewportHeight) * 2 - 1) * -1;
  result[10] = 1.0f;
  result[11] = 0;

  result[12] = float(bottomRightX) / float(viewportWidth) * 2 - 1;
  result[13] = (float(bottomRightY) / float(viewportHeight) * 2 - 1) * -1;
  result[14] = 1.0f;
  result[15] = 1.0f;

  return result;
}

OpenGLDriver::OpenGLDriver(int w, int h) {
  FL::Graphics::Shaders::initializeAll();
  if (FT_Init_FreeType(&fontLib) != 0) {
    // oops
  }

  if (FT_New_Face(fontLib, "Lexend-Regular.ttf", 0, &fontFace) != 0) {
    // oops
  }

  int size = 12; // pixels high
  if (FT_Set_Char_Size(fontFace, 0, size * 64, 96, 96) != 0) {
    // oops
  }

  for (char i = ' '; i <= '~'; ++i) {
    if (!std::isprint(i)) {
      continue;
    }

    FT_UInt gi = FT_Get_Char_Index(fontFace, i);
    if (gi == 0) {
      // oops
    }

    FT_Load_Glyph(fontFace, gi, FT_LOAD_DEFAULT);

    FT_Render_Glyph(fontFace->glyph, FT_RENDER_MODE_NORMAL);

    FL::Graphics::Character c;
    c.fontSize = size;
    c.glyphHeight = fontFace->glyph->metrics.height / 64;
    c.glyphWidth = fontFace->glyph->metrics.width / 64;
    c.xBearing = fontFace->glyph->metrics.horiBearingX / 64;
    c.yBearing = fontFace->glyph->metrics.horiBearingY / 64;
    c.xAdvance = fontFace->glyph->metrics.horiAdvance / 64;

    glGenTextures(1, &c.texture);
    glBindTexture(GL_TEXTURE_2D, c.texture);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, fontFace->glyph->bitmap.width,
                 fontFace->glyph->bitmap.rows, 0, GL_ALPHA, GL_UNSIGNED_BYTE,
                 fontFace->glyph->bitmap.buffer);
    // set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    auto err = glGetError();
    if (err != GL_NO_ERROR) {
      printf("GL ERROR: %d\n", err);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    characters.try_emplace(i, c);
  }

  viewportWidth = w;
  viewportHeight = h;

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  auto err = glGetError();
  if (err != GL_NO_ERROR) {
    printf("GL ERROR: %d\n", err);
  }

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  err = glGetError();
  if (err != GL_NO_ERROR) {
    printf("GL ERROR: %d\n", err);
  }

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                        (void *)nullptr);
  glEnableVertexAttribArray(0);

  err = glGetError();
  if (err != GL_NO_ERROR) {
    printf("GL ERROR: %d\n", err);
  }

  glGenBuffers(1, &texVbo);
  glBindBuffer(GL_ARRAY_BUFFER, texVbo);

  err = glGetError();
  if (err != GL_NO_ERROR) {
    printf("GL ERROR: %d\n", err);
  }

  glGenVertexArrays(1, &texVao);
  glBindVertexArray(texVao);

  err = glGetError();
  if (err != GL_NO_ERROR) {
    printf("GL ERROR: %d\n", err);
  }

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, false, 4 * 4, nullptr);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, false, 4 * 4,
                        (void *)(2 * sizeof(float)));

  err = glGetError();
  if (err != GL_NO_ERROR) {
    printf("GL ERROR: %d\n", err);
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  err = glGetError();
  if (err != GL_NO_ERROR) {
    printf("GL ERROR: %d\n", err);
  }

  auto frag = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag, 1, &fragmentShader, nullptr);
  glCompileShader(frag);

  GLint status;
  glGetShaderiv(frag, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    printf("Oh no, error: %d :(\n", status);
  }

  auto vert = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vert, 1, &vertexShaderSource, nullptr);
  glCompileShader(vert);

  glGetShaderiv(vert, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    printf("Oh no, error: %d :(\n", status);
    GLint logLength;
    glGetShaderiv(vert, GL_INFO_LOG_LENGTH, &logLength);

    char *str = static_cast<char *>(malloc(logLength + 1));
    memset(str, 'a', logLength);
    str[logLength] = '\0';
    glGetShaderInfoLog(vert, logLength, nullptr, str);

    printf("error: %s\n", str);
  }

  program = glCreateProgram();
  glAttachShader(program, frag);
  glAttachShader(program, vert);

  err = glGetError();
  if (err != GL_NO_ERROR) {
    printf("GL ERROR: %d\n", err);
  }

  glLinkProgram(program);
  //  glDetachShader(program, vert);
  //  glDetachShader(program, frag);

  err = glGetError();
  if (err != GL_NO_ERROR) {
    printf("GL ERROR: %d\n", err);
  }

  glDeleteShader(vert);
  glDeleteShader(frag);

  err = glGetError();
  if (err != GL_NO_ERROR) {
    printf("GL ERROR: %d\n", err);
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindVertexArray(0);
}

void OpenGLDriver::drawRectangle(int x, int y, int w, int h,
                                 FL::Graphics::Color color) {
  auto vertices = calculateVertexArray(x, y, w, h);

  glUseProgram(program);
  glUniform3f(glGetUniformLocation(program, "color"), color.R, color.G,
              color.B);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(),
               GL_STATIC_DRAW);

  glBindVertexArray(vao);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void OpenGLDriver::drawText(std::string text, int x, int y, Color color) {
  // go through each character and calculate the total width
  // while doing that, keep track of which character goes highest above the
  // baseline and which character goes lowest below the baseline
  auto maxAscent = 0;
  auto maxDescent = 0;

  auto cursorX2 = 0;

  FL::Graphics::Character lastChar;
  for (unsigned char glyph : text) {
    if (characters.find(glyph) == characters.end()) {
      continue;
    }

    auto c = characters.at(glyph);

    auto ascent = c.yBearing;
    if (ascent > maxAscent) {
      maxAscent = ascent;
    }

    auto descent = c.glyphHeight - c.yBearing;
    if (descent > maxDescent) {
      maxDescent = descent;
    }

    cursorX2 += c.xAdvance; // TODO: don't do this for the last one. For now
                            // whatever
    lastChar = c;
  }

  // We added the advance each time, but we don't want to do that for the
  // last character. So, we just subtract the difference here to simplify the
  // loop.
  cursorX2 -= (lastChar.xAdvance - lastChar.glyphWidth);

  //  drawRectangle(x - 10, y - 10, cursorX2 + 20, maxAscent + maxDescent +
  //  20,
  //                FL::Graphics::Color{1.0, 0.5, 0.2});

  auto cursorX = x;
  auto cursorY = y + maxAscent;
  for (unsigned char glyph : text) {
    if (characters.find(glyph) == characters.end()) {
      continue;
    }
    auto c = characters.at(glyph);

    auto vertices =
        calculateTexVertexArray(cursorX + c.xBearing, cursorY - c.yBearing + 1,
                                c.glyphWidth, c.glyphHeight);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, c.texture);
    glUseProgram(FL::Graphics::Shaders::fontProgram);
    glUniform3f(
        glGetUniformLocation(FL::Graphics::Shaders::fontProgram, "color"),
        color.R, color.G, color.B);

    glBindBuffer(GL_ARRAY_BUFFER, texVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(),
                 GL_DYNAMIC_DRAW);

    glBindVertexArray(texVao);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    //
    //    drawTexture(c.texture, cursorX + c.xBearing, cursorY - c.yBearing + 1,
    //                c.glyphWidth, c.glyphHeight);

    cursorX += c.xAdvance;
  }
}

OpenGLDriver::~OpenGLDriver() { FT_Done_FreeType(fontLib); }
void OpenGLDriver::calculateTextBounds(const std::string &text, int &width,
                                       int &height) {
  auto maxAscent = 0;
  auto maxDescent = 0;

  width = 0;

  FL::Graphics::Character lastChar;
  for (unsigned char glyph : text) {
    if (characters.find(glyph) == characters.end()) {
      continue;
    }
    auto c = characters.at(glyph);

    auto ascent = c.yBearing;
    if (ascent > maxAscent) {
      maxAscent = ascent;
    }

    auto descent = c.glyphHeight - c.yBearing;
    if (descent > maxDescent) {
      maxDescent = descent;
    }

    width += c.xAdvance; // TODO: don't do this for the last one. For now
                         // whatever
    lastChar = c;
  }

  // We added the advance each time, but we don't want to do that for the
  // last character. So, we just subtract the difference here to simplify the
  // loop.
  width -= (lastChar.xAdvance - lastChar.glyphWidth);
  height = (maxAscent + maxDescent);
}

Texture *OpenGLDriver::newTexture() { return new OpenGLTexture(); }

void OpenGLDriver::drawTexture(Texture *texture, FL::Math::Box displayBox) {
  auto *actualTex = reinterpret_cast<OpenGLTexture *>(texture);
  // TODO: Error handle I guess

  auto vertices = calculateTexVertexArray(
      displayBox.xPx, displayBox.yPx, displayBox.widthPx, displayBox.heightPx);

  glBindTexture(GL_TEXTURE_2D, actualTex->texId);
  glUseProgram(FL::Graphics::Shaders::texProgram);

  glBindBuffer(GL_ARRAY_BUFFER, texVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(),
               GL_DYNAMIC_DRAW);

  glBindVertexArray(texVao);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLDriver::setDisplayArea(FL::Math::Box box) {
  displayArea = box;
  viewportWidth = box.widthPx;
  viewportHeight = box.heightPx;
}

} // namespace FL::Graphics