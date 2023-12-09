//
// Created by alexs on 10/17/2023.
//

#ifndef FIRELIGHT_VIDEO_HPP
#define FIRELIGHT_VIDEO_HPP

#include "../../lib/graphics/driver.hpp"
#include "../../lib/graphics/texture.hpp"
#include "../../lib/math/bbox.hpp"
#include "GL/glew.h"
#include "libretro.h"
#include <array>
#include <cstddef>

namespace libretro {

/**
 * Represents the way the game's image will be displayed on the user's screen.
 */
enum PictureMode {
  // Display using the console or game's native resolution.
  ORIGINAL,

  // Maintain the game's aspect ratio and scale to the size of the window.
  ASPECT_RATIO,

  // Fill the entire window, ignoring aspect ratio.
  STRETCH
};

class Video {

public:
  Video(FL::Graphics::Driver *driver);

  void initialize(int x2, int y2, int screenWidth, int screenHeight);

  retro_hw_render_callback *hardwareRenderCallback;

  void initializeHwFramebuffer();

  void setPictureMode(PictureMode);

  void draw();

  void setScreenDimensions(int x, int y, int width, int height);

  void refreshCoreVideo(const void *data, unsigned width, unsigned height,
                        size_t pitch);

  void setRotation(unsigned int);

  bool getOverscan() const;

  bool getAllowDupeFrames() const;

  void setGameGeometry(retro_game_geometry *);

  void setPixelFormat(retro_pixel_format *);

  void setHardwareRenderCallback(retro_hw_render_callback *);

  void setFrameTimeCallback(retro_frame_time_callback *);

private:
  FL::Math::Box displayBox;
  FL::Graphics::Driver *gfxDriver;
  int x, y = 0;
  int windowWidth, windowHeight;

  FL::Graphics::Texture *gameTexture;
  FL::Graphics::Texture *fullScreenTexture;

  bool hardwareRendering = false;

  PictureMode displayMode = ASPECT_RATIO;

  GLuint renderBuffer;
  GLuint fbo;
  GLuint otherTex;

  GLuint intermediateVao;
  GLuint intermediateVbo;
  GLuint intermediateTex;
  GLuint intermediateFbo;

  void recalcVertexArray();

  GLuint vao;
  GLuint vbo;

  std::array<float, 16> vertices = {-1.0, -1.0, 0, 1.0, 1.0, -1.0, 1.0, 1.0,
                                    -1.0, 1.0,  0, 0,   1.0, 1.0,  1.0, 0};

  std::array<float, 16> fullScreenVertices = {-1.0, -1.0, 0,    1.0, 1.0, -1.0,
                                              1.0,  1.0,  -1.0, 1.0, 0,   0,
                                              1.0,  1.0,  1.0,  0};

  retro_game_geometry *gameGeometry;

  retro_frame_time_callback *frameTimeCallback;
  unsigned preferredHardwareRenderer; // need to set early based on window
  unsigned rotation = 0;
  bool useOverscan = false;
  bool canDupeFrames = true;
  retro_pixel_format *pixelFormat;
  retro_framebuffer *softwareFramebuffer;
  retro_hw_render_interface *hardwareRenderInterface;
  retro_hw_render_context_negotiation_interface
      *hwRenderContextNegotiationInterface;
  bool hardwareSharedContext; // TODO what is this
  float targetRefreshRate;
};

} // namespace libretro

#endif // FIRELIGHT_VIDEO_HPP
