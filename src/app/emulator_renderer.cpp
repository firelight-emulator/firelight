#include "emulator_renderer.hpp"

#include "emulation_manager.hpp"

#include <QOpenGLFramebufferObject>
#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QSGTextureProvider>
#include <spdlog/spdlog.h>

EmulatorRenderer::EmulatorRenderer(const EmulationManager *manager) {
  initializeOpenGLFunctions();
  printf("NEW RENDERER\n");
  m_manager = const_cast<EmulationManager *>(manager);
  m_manager->setReceiveVideoDataFunction(
    [this](const void *data, unsigned width, unsigned height, size_t pitch) {
      receiveVideoData(data, width, height, pitch);
    });

  m_manager->setGetProcAddressFunction([this](const char *sym) {
    return QOpenGLContext::currentContext()->getProcAddress(sym);
  });
}

void EmulatorRenderer::synchronize(QQuickFramebufferObject *fbo) {
  const auto manager = reinterpret_cast<EmulationManager *>(fbo);

  if (m_fbo) {
    manager->setCurrentFboId(m_fbo->handle());
  }

  if (manager->nativeWidth() != m_nativeWidth) {
    invalidateFramebufferObject();
    m_runAFrame = false;
  }

  if (!m_resetContextFunction) {
    m_resetContextFunction = manager->consumeContextResetFunction();
  }

  // if (!m_destroyContextFunction) {
  //   m_destroyContextFunction = manager->consumeContextDestroyFunction();
  // }

  m_nativeWidth = manager->nativeWidth();
  m_nativeHeight = manager->nativeHeight();

  Renderer::synchronize(fbo);
}

QOpenGLFramebufferObject *
EmulatorRenderer::createFramebufferObject(const QSize &size) {
  if (m_nativeWidth != 0 && m_nativeHeight != 0) {
    m_fbo =
        Renderer::createFramebufferObject(QSize(m_nativeWidth, m_nativeHeight));
    return m_fbo;
  }

  m_fbo = Renderer::createFramebufferObject(size);

  return m_fbo;
}

void EmulatorRenderer::render() {
  if (m_resetContextFunction) {
    printf("Calling context reset\n");
    m_resetContextFunction();
    m_resetContextFunction = nullptr;
  }

  if (m_manager->runFrame()) {
    update();
    m_runAFrame = true;
  } else if (!m_runAFrame) {
    m_fbo->bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    m_fbo->release();
  }
}

void EmulatorRenderer::receiveVideoData(const void *data, unsigned width,
                                        unsigned height, size_t pitch) const {
  QOpenGLPaintDevice paint_device;
  paint_device.setSize(m_fbo->size());
  QPainter painter(&paint_device);

  m_fbo->bind();
  const QImage image((uchar *) data, width, height, pitch, QImage::Format_RGB16);

  painter.drawImage(QRect(0, 0, m_fbo->width(), m_fbo->height()), image,
                    image.rect());
  m_fbo->release();
}
