//
// Created by alexs on 11/3/2023.
//

#include "shaders.hpp"

namespace FL::Graphics {

void Shaders::initializeAll() {
  fontFragmentShader = "#version 410\n"
                       "uniform sampler2D Texture;\n"
                       "uniform vec3 color;\n"

                       "in vec2 fragTexCoord;\n"

                       "out vec4 FragColor;\n"

                       "void main() {\n"
                       "float alpha = texture(Texture, fragTexCoord).a;\n"
                       "FragColor = vec4(color.rgb, alpha);\n"
                       "}\0";

  fragmentShader = "#version 410\n"
                   "uniform sampler2D Texture;\n"

                   "in vec2 fragTexCoord;\n"

                   "out vec4 FragColor;\n"

                   "void main() {\n"
                   "vec4 c = texture(Texture, fragTexCoord);\n"
                   "FragColor = c;\n"
                   "}\0";

  vertexShader = "#version 410\n"
                 "layout(location = 0) in vec2 vert;\n"
                 "layout(location = 1) in vec2 vertTexCoord;\n"

                 "out vec2 fragTexCoord;\n"

                 "void main() {\n"
                 "fragTexCoord = vertTexCoord;\n"
                 "gl_Position = vec4(vert, 0.0, 1.0);\n"
                 "}\0";

  auto frag = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag, 1, &fragmentShader, nullptr);
  glCompileShader(frag);

  GLint status;
  glGetShaderiv(frag, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    printf("Oh no, error: %d :(\n", status);
  }

  auto err = glGetError();
  if (err != GL_NO_ERROR) {
    printf("GL ERROR: %d\n", err);
  }

  auto font = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(font, 1, &fontFragmentShader, nullptr);
  glCompileShader(font);

  glGetShaderiv(font, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    printf("Oh no, error: %d :(\n", status);
  }

  err = glGetError();
  if (err != GL_NO_ERROR) {
    printf("GL ERROR: %d\n", err);
  }

  auto vert = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vert, 1, &vertexShader, nullptr);
  glCompileShader(vert);

  GLint status2;

  glGetShaderiv(vert, GL_COMPILE_STATUS, &status2);
  if (status2 == GL_FALSE) {
    printf("Oh no, error: %d :(\n", status2);
    GLint logLength;
    glGetShaderiv(vert, GL_INFO_LOG_LENGTH, &logLength);

    char *str = static_cast<char *>(malloc(logLength + 1));
    memset(str, 'a', logLength);
    str[logLength] = '\0';
    glGetShaderInfoLog(vert, logLength, nullptr, str);

    printf("error: %s\n", str);
  }

  err = glGetError();
  if (err != GL_NO_ERROR) {
    printf("GL ERROR: %d\n", err);
  }

  texProgram = glCreateProgram();
  glAttachShader(texProgram, frag);
  glAttachShader(texProgram, vert);

  glLinkProgram(texProgram);
  glDetachShader(texProgram, vert);
  glDetachShader(texProgram, frag);

  fontProgram = glCreateProgram();
  glAttachShader(fontProgram, font);
  glAttachShader(fontProgram, vert);

  glLinkProgram(fontProgram);
  glDetachShader(fontProgram, vert);
  glDetachShader(fontProgram, font);

  glDeleteShader(vert);
  glDeleteShader(frag);
  glDeleteShader(font);

  err = glGetError();
  if (err != GL_NO_ERROR) {
    printf("GL ERROR: %d\n", err);
  }
}

} // namespace FL::Graphics
