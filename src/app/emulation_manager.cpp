#include "emulation_manager.hpp"
#include "emulator_renderer.hpp"
#include <QGuiApplication>
#include <QOpenGLPaintDevice>
#include <spdlog/spdlog.h>
#include <utility>

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

int EmulationManager::getEntryId() const { return m_entryId; }
QByteArray EmulationManager::getGameData() { return m_gameData; }
QByteArray EmulationManager::getSaveData() { return m_saveData; }
QString EmulationManager::getCorePath() { return m_corePath; }
QString EmulationManager::currentGameName() const {
  return QString::fromStdString(m_currentEntry.display_name);
}
int EmulationManager::nativeWidth() const { return m_nativeWidth; }
int EmulationManager::nativeHeight() const { return m_nativeHeight; }
float EmulationManager::nativeAspectRatio() const {
  return m_nativeAspectRatio;
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
}

void EmulationManager::stopEmulation() {
  m_shouldStopEmulation = true;
  update();
}

void EmulationManager::resetEmulation() {
  m_shouldResetEmulation = true;
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

bool EmulationManager::takeShouldResetEmulationFlag() {
  if (m_shouldResetEmulation) {
    m_shouldResetEmulation = false;
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
void EmulationManager::setNativeWidth(int nativeWidth) {
  if (m_nativeWidth != nativeWidth) {
    emit nativeWidthChanged();
  }

  m_nativeWidth = nativeWidth;
}
void EmulationManager::setNativeHeight(int nativeHeight) {
  if (m_nativeHeight != nativeHeight) {
    emit nativeHeightChanged();
  }

  m_nativeHeight = nativeHeight;
}
void EmulationManager::setNativeAspectRatio(float nativeAspectRatio) {
  if (m_nativeAspectRatio != nativeAspectRatio) {
    emit nativeAspectRatioChanged();
  }

  m_nativeAspectRatio = nativeAspectRatio;
}
