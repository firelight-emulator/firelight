//
// Created by alexs on 12/20/2023.
//

#ifndef FIRELIGHT_EMULATOR_RENDERER_HPP
#define FIRELIGHT_EMULATOR_RENDERER_HPP

#include "src/app/emulation_manager.hpp"
#include <QObject>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFramebufferObject>
#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QQuickFramebufferObject>
#include <QQuickWindow>
#include <QThread>
#include <QTimer>

class EmulatorRenderer : public QQuickFramebufferObject::Renderer,
                         public QOpenGLFunctions,
                         FramebufferHandleProvider {
public:
  uintptr_t get_framebuffer_handle() override;

  explicit EmulatorRenderer(QQuickWindow *window);

  //  void measureFrameEnd() {
  //
  //    lastFrame = thisFrame;
  //    thisFrame = timer.elapsed();
  //  }

  [[nodiscard]] uintptr_t get_framebuffer_id() const { return 0; }

  void render() override {
    auto manager = EmulationManager::getInstance();

    auto fbo = framebufferObject();
    QOpenGLPaintDevice device(fbo->size());

    m_window->beginExternalCommands();
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferobject);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    auto module = QOpenGLContext::openGLModuleType();

    GLuint otherTex;
    glGenTextures(1, &otherTex);
    glBindTexture(GL_TEXTURE_2D, otherTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1280, 720, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);

    GLuint framebufferobject;
    glGenFramebuffers(1, &framebufferobject);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferobject);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           otherTex, 0);

    GLuint renderBuffer;
    glGenRenderbuffers(1, &renderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 1280, 720);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, renderBuffer);

    auto s = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (s != GL_FRAMEBUFFER_COMPLETE) {
      printf("frame buffer not complete %d\n", s);
    }

    manager->runOneFrame();
    m_window->endExternalCommands();

    //    painter.setPen(Qt::white);
    //    painter.setFont(QFont("Arial", 72));
    //    painter.drawText(200, 200,
    //                     QString::fromStdString(std::to_string(getCounterValue())));
    auto image = EmulationManager::getInstance()->getImage();
    if (image) {
      QPainter painter(&device);
      painter.drawImage(bounds.toRect(), *image);
    }

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

    glGenTextures(1, &otherTex);
    glBindTexture(GL_TEXTURE_2D, otherTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1280, 720, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);

    glGenFramebuffers(1, &framebufferobject);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferobject);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           otherTex, 0);

    glGenRenderbuffers(1, &renderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 1280, 720);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, renderBuffer);

    return new QOpenGLFramebufferObject(QSize(2000, 2000), format);
  }

private:
  GLuint otherTex;
  GLuint framebufferobject;
  GLuint renderBuffer;
  QElapsedTimer timer;
  qint64 thisFrame;
  qint64 lastFrame;
  QRectF bounds;
  QQuickWindow *m_window;
  QImage m_emulatorFrame; // ???
};

#endif // FIRELIGHT_EMULATOR_RENDERER_HPP
