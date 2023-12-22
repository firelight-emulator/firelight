//
// Created by alexs on 12/20/2023.
//

#ifndef FIRELIGHT_EMULATOR_RENDERER_HPP
#define FIRELIGHT_EMULATOR_RENDERER_HPP

#include "src/app/emulation_manager.hpp"
#include <QObject>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QQuickFramebufferObject>
#include <QQuickWindow>
#include <QThread>
#include <QTimer>

class EmulatorRenderer : public QQuickFramebufferObject::Renderer,
                         public QOpenGLFunctions {

public:
  explicit EmulatorRenderer(QQuickWindow *window);

  void measureFrame() {
    auto dt = timer.restart();

    if (dt > 22 || dt < 12) {
      printf("dt: %lld\n", dt);
    }
  }

  //  void measureFrameEnd() {
  //
  //    lastFrame = thisFrame;
  //    thisFrame = timer.elapsed();
  //  }

  void render() override {
    measureFrame();
    auto manager = EmulationManager::getInstance();
    manager->runOneFrame();

    auto fbo = framebufferObject();
    QOpenGLPaintDevice device(fbo->size());

    QPainter painter(&device);
    painter.beginNativePainting();

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    painter.endNativePainting();

    //    painter.setPen(Qt::white);
    //    painter.setFont(QFont("Arial", 72));
    //    painter.drawText(200, 200,
    //                     QString::fromStdString(std::to_string(getCounterValue())));
    auto image = EmulationManager::getInstance()->getImage();
    painter.drawImage(bounds.toRect(), *image);
    image.release();

    //    painter.setRenderHints(QPainter::LosslessImageRendering);
    //
    //    painter.drawText(0, 0,
    //    QString::fromStdString(std::to_string(counter)));
    //    printf("counter: %d\n", counter);
    //    painter.setRenderHints(QPainter::HighQualityAntialiasing);

    // Render stuff here, use either painter or opengl directly.
    // inheriting from QOpenGLFunctions_3_3_Core gives you access to the gl
    // functions
  }

  void synchronize(QQuickFramebufferObject *item) override {
    bounds = item->boundingRect();
    item->update();
    // get your emulator image here, copy it to a member variable or something
    // ready for the render step which happens next
  }

  QOpenGLFramebufferObject *
  createFramebufferObject(const QSize &size) override {
    initializeOpenGLFunctions();
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(8);

    return new QOpenGLFramebufferObject(size, format);
  }

private:
  QElapsedTimer timer;
  qint64 thisFrame;
  qint64 lastFrame;
  QRectF bounds;
  QQuickWindow *m_window;
  QImage m_emulatorFrame; // ???
};

#endif // FIRELIGHT_EMULATOR_RENDERER_HPP
