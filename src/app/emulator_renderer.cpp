#include "emulator_renderer.hpp"

#include "emulation_manager.hpp"

#include <QOpenGLFramebufferObject>
#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QSGTextureProvider>
#include <spdlog/spdlog.h>

#include "audio_manager.hpp"
#include "../gui/game_image_provider.hpp"

static constexpr int AUTOSAVE_INTERVAL_MILLIS = 10000;

EmulatorRenderer::EmulatorRenderer() {
  initializeOpenGLFunctions();
  autosaveTimer.setSingleShot(false);
  autosaveTimer.setInterval(AUTOSAVE_INTERVAL_MILLIS);
  QObject::connect(&autosaveTimer, &QTimer::timeout,
                   [this] {
                     m_shouldSave = true;
                   }
  );
  suspendPointTimer.setSingleShot(false);
  suspendPointTimer.setInterval(3000);
  QObject::connect(&suspendPointTimer, &QTimer::timeout,
                   [this] {
                     m_shouldCreateSuspendPoint = true;
                   });
}

EmulatorRenderer::~EmulatorRenderer() {
  autosaveTimer.stop();
  getAchievementManager()->unloadGame();

  save(true);
  // TODO: SAVE

  if (m_destroyContextFunction) {
    m_destroyContextFunction();
  }
}

proc_address_t EmulatorRenderer::getProcAddress(const char *sym) {
  return QOpenGLContext::currentContext()->getProcAddress(sym);
}

void EmulatorRenderer::setResetContextFunc(context_reset_func func) {
  m_usingHwRendering = true;
  m_resetContextFunction = func;
}

void EmulatorRenderer::setDestroyContextFunc(context_destroy_func func) {
  m_usingHwRendering = true;
  m_destroyContextFunction = func;
}

uintptr_t EmulatorRenderer::getCurrentFramebufferId() {
  if (!m_fbo) {
    return -1;
  }

  return m_fbo->handle();
}

void EmulatorRenderer::setSystemAVInfo(retro_system_av_info *info) {
  invalidateFramebufferObject();

  const auto width = info->geometry.base_width;
  const auto height = info->geometry.base_height;

  if (width > 0 && height > 0) {
    m_nativeWidth = width;
    m_nativeHeight = height;

    const auto aspectRatio =
        static_cast<float>(width) / static_cast<float>(height);
    m_nativeAspectRatio = aspectRatio;
  }

  update();
}

void EmulatorRenderer::setPixelFormat(retro_pixel_format *format) {
  switch (*format) {
    case RETRO_PIXEL_FORMAT_0RGB1555:
      printf("Pixel format: 0RGB1555\n");
      break;
    case RETRO_PIXEL_FORMAT_XRGB8888:
      m_pixelFormat = QImage::Format_RGB32;
      break;
    case RETRO_PIXEL_FORMAT_RGB565:
      m_pixelFormat = QImage::Format_RGB16;
      break;
    case RETRO_PIXEL_FORMAT_UNKNOWN:
      printf("Pixel format: UNKNOWN\n");
      break;
  }
}

void EmulatorRenderer::save(const bool waitForFinish) {
  spdlog::debug("Saving game data\n");
  firelight::saves::Savefile saveData(
    m_core->getMemoryData(libretro::SAVE_RAM));
  // saveData.setImage(m_fbo->toImage());

  QFuture<bool> result =
      getSaveManager()->writeSaveDataForEntry(m_currentEntry, saveData);

  if (waitForFinish) {
    result.waitForFinished();
  }
}

void EmulatorRenderer::synchronize(QQuickFramebufferObject *fbo) {
  const auto manager = reinterpret_cast<EmulationManager *>(fbo);

  m_paused = manager->m_paused;
  m_gameReady = manager->m_gameReady;

  if (m_core && manager->m_rewindPointIndex != -1) {
    spdlog::info("Loading rewind point {}", manager->m_rewindPointIndex);
    m_core->deserializeState(m_suspendPoints[manager->m_rewindPointIndex - 1].state);

    for (auto i = manager->m_rewindPointIndex - 1; i >= 0; --i) {
      m_suspendPoints.remove(i);
    }


    // m_suspendPoints.removeAt(manager->m_rewindPointIndex);
    // manager->rewindPointLoaded();

    // const auto image = m_suspendPoints[manager->m_rewindPointIndex - 1].image;
    manager->m_rewindPointIndex = -1;

    manager->rewindPointLoaded();
  }

  if (m_core && manager->m_shouldGetRewindPoints) {
    QList<QJsonObject> points;
    int i = 1;

    auto now = QDateTime::currentMSecsSinceEpoch();
    for (const auto &point: m_suspendPoints) {
      auto time = QDateTime::fromMSecsSinceEpoch(point.timestamp).time();
      auto diff = time.secsTo(QDateTime::fromMSecsSinceEpoch(now).time());

      QJsonObject obj;
      getGameImageProvider()->setImage(QString::number(i), point.image);
      obj["image_url"] = "image://gameimages/" + QString::number(i++);
      // obj["time"] = now - point.timestamp;
      obj["time"] = time.toString();
      obj["ago"] = QString::number(diff) + " seconds ago";
      points.append(obj);
    }

    QJsonObject obj;
    getGameImageProvider()->setImage("0", m_fbo->toImage());
    obj["image_url"] = "image://gameimages/0";
    obj["time"] = QDateTime::fromMSecsSinceEpoch(now).time().toString();
    obj["ago"] = "Just now";
    points.prepend(obj);

    manager->rewindPointsReady(points);
    manager->m_shouldGetRewindPoints = false;
  }

  if (manager->m_shouldReset) {
    if (m_core) {
      m_core->reset();
    }
    manager->m_shouldReset = false;
  }

  if (!m_core && m_gameReady) {
    m_gameData = manager->m_gameData;
    m_saveData = manager->m_saveData;
    m_corePath = manager->m_corePath;
    m_currentEntry = manager->m_currentEntry;
  }

  if (m_coreConfiguration) {
    manager->m_coreConfiguration = m_coreConfiguration;
  }

  manager->setGeometry(m_nativeWidth, m_nativeHeight, m_nativeAspectRatio);
  manager->setIsRunning(m_running);

  Renderer::synchronize(fbo);
}

QOpenGLFramebufferObject *
EmulatorRenderer::createFramebufferObject(const QSize &size) {
  if (m_nativeWidth != 0 && m_nativeHeight != 0) {
    m_fboIsNew = true;
    m_fbo =
        Renderer::createFramebufferObject(QSize(m_nativeWidth, m_nativeHeight));

    if (m_resetContextFunction) {
      m_resetContextFunction();
    }
    return m_fbo;
  }

  m_fbo = Renderer::createFramebufferObject(size);
  return m_fbo;
}

void EmulatorRenderer::render() {
  auto currentTime = QDateTime::currentMSecsSinceEpoch();
  if (!m_core && m_gameReady) {
    auto configProvider = getEmulatorConfigManager()->getCoreConfigFor(m_currentEntry.platformId, m_currentEntry.id);
    m_core = std::make_unique<libretro::Core>(m_corePath.toStdString(), configProvider);

    m_core->setVideoReceiver(this);
    m_core->setAudioReceiver(std::make_shared<AudioManager>());
    m_core->setRetropadProvider(getControllerManager());

    m_core->setSystemDirectory("./system");
    // m_core->setSaveDirectory(".");
    m_core->init();

    libretro::Game game(
      m_currentEntry.contentPath,
      vector<unsigned char>(m_gameData.begin(), m_gameData.end()));
    m_core->loadGame(&game);

    if (m_saveData.size() > 0) {
      m_core->writeMemoryData(libretro::SAVE_RAM,
                              vector(m_saveData.begin(), m_saveData.end()));
    }

    autosaveTimer.start();
    suspendPointTimer.start();

    QMetaObject::invokeMethod(
      getAchievementManager(), "loadGame", Qt::QueuedConnection,
      Q_ARG(int, m_currentEntry.platformId),
      Q_ARG(QString, QString::fromStdString(m_currentEntry.contentId)));

    update();
    return;
  }

  if (m_fbo && m_core && !m_paused) {
    m_running = true;
    m_core->run(0);
    getAchievementManager()->doFrame(m_core.get(), m_currentEntry);
    if (m_shouldSave) {
      save(false);
      m_shouldSave = false;
    }

    if (m_shouldCreateSuspendPoint) {
      if (m_fbo->size() != QSize(0, 0)) {
        // spdlog::info("Creating suspend point");
        SuspendPoint suspendPoint;
        suspendPoint.state = m_core->serializeState();
        suspendPoint.image = m_fbo->toImage();
        suspendPoint.timestamp = QDateTime::currentMSecsSinceEpoch();
        m_suspendPoints.push_front(suspendPoint);
      }

      m_shouldCreateSuspendPoint = false;
    }
  } else if (m_fboIsNew) {
    // m_fbo->bind();
    // glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // glClear(GL_COLOR_BUFFER_BIT);
    // m_fbo->release();
  }

  auto newTime = QDateTime::currentMSecsSinceEpoch();
  auto elapsed = newTime - currentTime;
  if (elapsed > 10) {
    spdlog::info("Frame took {} ms", elapsed);
  }

  update();
}

void EmulatorRenderer::receive(const void *data, unsigned width,
                               unsigned height, size_t pitch) {
  if (!m_usingHwRendering && data != nullptr) {
    QOpenGLPaintDevice paint_device;
    paint_device.setSize(m_fbo->size());
    QPainter painter(&paint_device);

    m_fbo->bind();
    const QImage image((uchar *) data, width, height, pitch, m_pixelFormat);

    // TODO: Check native size, etc. make sure we use max size and base size correctly
    painter.drawImage(QRect(0, 0, m_fbo->width(), m_fbo->height()), image,
                      image.rect());
    m_fbo->release();
  }
}
