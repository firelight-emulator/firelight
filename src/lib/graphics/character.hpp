//
// Created by alexs on 11/2/2023.
//

#ifndef FIRELIGHT_CHARACTER_HPP
#define FIRELIGHT_CHARACTER_HPP

#include "GL/glew.h"

namespace FL::Graphics {

/**
 * A Character represents a single glyph in a specific typeface and font size
 * . It does not account for kerning or ligature.
 */
class Character {
public:
  GLuint texture{};

  // height of the glyph's bounding box
  unsigned glyphHeight = 0;

  // width of the glyph's bounding box
  unsigned glyphWidth = 0;

  // x distance from origin (bottom left) to start drawing
  unsigned xBearing = 0;

  // y distance from origin (bottom left) to start drawing
  unsigned yBearing = 0;

  unsigned fontSize = 0;

  unsigned xAdvance = 0;
};

} // namespace FL::Graphics

#endif // FIRELIGHT_CHARACTER_HPP
