//
// Created by alexs on 11/3/2023.
//

#ifndef FIRELIGHT_SHADERS_HPP
#define FIRELIGHT_SHADERS_HPP

#include "GL/glew.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
namespace FL::Graphics {
class Shaders {
public:
  inline static GLuint fontProgram;
  inline static GLuint texProgram;
  inline static GLuint solidColorProgram;

  inline static const GLchar *fontFragmentShader;
  inline static const GLchar *fragmentShader;
  inline static const GLchar *vertexShader;

  static void initializeAll();
};

} // namespace FL::Graphics

#endif // FIRELIGHT_SHADERS_HPP
