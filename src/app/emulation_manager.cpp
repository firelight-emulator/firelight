//
// Created by alexs on 12/20/2023.
//

#include "emulation_manager.hpp"
#include "../gui/controller_manager.hpp"
#include <QGuiApplication>
#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QSGImageNode>
#include <QSGTexture>
#include <qopenglcontext.h>
#include <utility>

#include "audio_manager.hpp"
#include "emulator_renderer.hpp"

#include <spdlog/spdlog.h>

constexpr int SAVE_FREQUENCY_MILLIS = 10000;

QQuickFramebufferObject::Renderer *EmulationManager::createRenderer() const {
  return new EmulatorRenderer();
  // return new EmulatorRenderer(m_entryId, m_gameData, m_saveData, m_corePath);
}

EmulationManager::EmulationManager(QQuickItem *parent)
    : QQuickFramebufferObject(parent) {
  setTextureFollowsItemSize(false);
  setMirrorVertically(true);
  setFlag(ItemHasContents);
}
EmulationManager::~EmulationManager() {
  printf("DESTROYING EMULATION MANAGER\n");
}

int EmulationManager::getEntryId() const { return m_entryId; }
QByteArray EmulationManager::getGameData() { return m_gameData; }
QByteArray EmulationManager::getSaveData() { return m_saveData; }
QString EmulationManager::getCorePath() { return m_corePath; }
QString EmulationManager::currentGameName() const {
  return QString::fromStdString(m_currentEntry.display_name);
}

void EmulationManager::loadGame(int entryId, const QByteArray &gameData,
                                const QByteArray &saveData,
                                const QString &corePath) {
  m_shouldLoadGame = true;
  m_entryId = entryId;
  m_gameData = gameData;
  m_saveData = saveData;
  m_corePath = corePath;
  update();

  printf("DOING IT\n");
}

void EmulationManager::pauseGame() {
  m_shouldPauseGame = true;
  update();
}

void EmulationManager::resumeGame() {
  m_shouldResumeGame = true;
  update();
}

void EmulationManager::startEmulation() {
  m_shouldStartEmulation = true;
  update();
  printf("starting emulation\n");
}

void EmulationManager::stopEmulation() {
  m_shouldStopEmulation = true;
  update();
}
bool EmulationManager::isRunning() { return m_isRunning; }

bool EmulationManager::takeShouldLoadGameFlag() {
  if (m_shouldLoadGame) {
    m_shouldLoadGame = false;
    return true;
  }

  return false;
}

bool EmulationManager::takeShouldPauseGameFlag() {
  if (m_shouldPauseGame) {
    m_shouldPauseGame = false;
    return true;
  }

  return false;
}
bool EmulationManager::takeShouldResumeGameFlag() {
  if (m_shouldResumeGame) {
    m_shouldResumeGame = false;
    return true;
  }

  return false;
}
bool EmulationManager::takeShouldStartEmulationFlag() {
  if (m_shouldStartEmulation) {
    m_shouldStartEmulation = false;
    return true;
  }

  return false;
}
bool EmulationManager::takeShouldStopEmulationFlag() {
  if (m_shouldStopEmulation) {
    m_shouldStopEmulation = false;
    return true;
  }

  return false;
}

void EmulationManager::setIsRunning(bool isRunning) { m_isRunning = isRunning; }

void EmulationManager::setCurrentEntry(LibEntry entry) {
  if (entry.display_name != m_currentEntry.display_name) {
    m_currentEntry = std::move(entry);
    emit currentGameNameChanged();
  }
}

// void EmulationManager::receive(const void *data, unsigned int width,
//                                unsigned int height, size_t pitch) {
//   if (data != nullptr && !usingHwRendering) {
//     if (!gameFbo) {
//       gameFbo = std::make_unique<QOpenGLFramebufferObject>(width, height);
//
//       gameTexture = QNativeInterface::QSGOpenGLTexture::fromNative(
//           gameFbo->texture(), window(), gameFbo->size());
//     }
//
//     QOpenGLPaintDevice paint_device;
//     paint_device.setSize(gameFbo->size());
//     QPainter painter(&paint_device);
//
//     gameFbo->bind();
//     const QImage image((uchar *)data, width, height, pitch,
//                        QImage::Format_RGB16);
//
//     painter.drawImage(QRect(0, 0, gameFbo->width(), gameFbo->height()),
//     image,
//                       image.rect());
//     gameFbo->release();
//   }
// }
