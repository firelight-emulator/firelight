//
// Created by alexs on 12/19/2023.
//

#ifndef FIRELIGHT_FL_GAME_HPP
#define FIRELIGHT_FL_GAME_HPP

#include "src/app/controller/controller_manager.hpp"
#include "src/app/libretro/core.hpp"
#include <QObject>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QPainter>
#include <QQuickFramebufferObject>
#include <QQuickPaintedItem>
#include <QtQmlIntegration>
#include <SDL_timer.h>

class FLGame : public QQuickPaintedItem,
               public CoreVideoDataReceiver,
               protected QOpenGLFunctions {
  Q_OBJECT

public:
  explicit FLGame(QQuickItem *parent = nullptr);
  Q_INVOKABLE void init();
  Q_INVOKABLE void runOneFrame();
  void receive(const void *data, unsigned int width, unsigned int height,
               size_t pitch) override;
  void paint(QPainter *painter) override;

public slots:
  void setRefreshRate(qreal refreshRate);

private:
  QOpenGLTexture *tex;
  boolean openGlInitialized = false;
  QOpenGLVertexArrayObject *actualVao;
  GLuint vao;
  GLuint vbo;
  GLuint program;

  GLuint texVao;
  GLuint texVbo;
  float framerate = 60; // Shouldn't need a default, but eh.
  int dumb = 0;
  void *m_data;
  unsigned m_width;
  unsigned m_height;
  size_t m_pitch;
  QVariant m_author;
  boolean running = false;
  Uint64 thisTick;
  Uint64 lastTick;
  std::unique_ptr<libretro::Core> core;
  FL::Input::ControllerManager conManager;
};

#endif // FIRELIGHT_FL_GAME_HPP
