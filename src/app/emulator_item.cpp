#include "emulator_item.hpp"
#include <QQuickWindow>
#include <rhi/qrhi_platform.h>
#include <spdlog/spdlog.h>

#include "emulator_item_renderer.hpp"
#include "platform_metadata.hpp"

void EmulatorItem::mouseMoveEvent(QMouseEvent *event) {
    const auto pos = event->position();
    const auto bounds = boundingRect();

    const auto x = (pos.x() - bounds.width() / 2) / (bounds.width() / 2);
    const auto y = (pos.y() - bounds.height() / 2) / (bounds.height() / 2);

    getControllerManager()->updateMouseState(x, y, m_mousePressed);
}

EmulatorItem::EmulatorItem(QQuickItem *parent) : QQuickRhiItem(parent) {
    setFlag(ItemHasContents);
    setAcceptHoverEvents(true);
    setAcceptTouchEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);

    m_autosaveTimer.setInterval(10000);
    m_autosaveTimer.setSingleShot(false);
    connect(&m_autosaveTimer, &QTimer::timeout,
            [this] {
                if (m_renderer) {
                    m_renderer->submitCommand({
                        .type = EmulatorItemRenderer::WriteSaveFile
                    });
                    update();
                }
            });

    m_autosaveTimer.start();

    m_rewindPointTimer.setInterval(3000);
    m_rewindPointTimer.setSingleShot(false);
    connect(&m_rewindPointTimer, &QTimer::timeout,
            [this] {
                if (m_renderer) {
                    m_renderer->submitCommand({
                        .type = EmulatorItemRenderer::WriteRewindPoint
                    });
                    update();
                }
            });
    m_rewindPointTimer.start();
}


EmulatorItem::~EmulatorItem() {
    spdlog::info("Destroying EmulatorItem");
    m_autosaveTimer.stop();
}

bool EmulatorItem::paused() const {
    return m_paused;
}

void EmulatorItem::setPaused(const bool paused) {
    if (m_paused != paused) {
        m_paused = paused;
        emit pausedChanged();
        update();
    }
}

float EmulatorItem::audioBufferLevel() const {
    return m_audioManager ? m_audioManager->getBufferLevel() : 0.0f;
}

void EmulatorItem::resetGame() {
    m_renderer->submitCommand({
        .type = EmulatorItemRenderer::ResetGame
    });
    update();
}

void EmulatorItem::writeSuspendPoint(const int index) {
    m_renderer->submitCommand({
        .type = EmulatorItemRenderer::WriteSuspendPoint,
        .suspendPointIndex = index
    });
    update();
}

void EmulatorItem::loadSuspendPoint(const int index) {
    m_renderer->submitCommand({
        .type = EmulatorItemRenderer::LoadSuspendPoint,
        .suspendPointIndex = index
    });
    update();
}

void EmulatorItem::createRewindPoints() {
    m_renderer->submitCommand({
        .type = EmulatorItemRenderer::EmitRewindPoints
    });
    update();
}

void EmulatorItem::hoverMoveEvent(QHoverEvent *event) {
    const auto pos = event->position();
    const auto bounds = boundingRect();

    const auto x = (pos.x() - bounds.width() / 2) / (bounds.width() / 2);
    const auto y = (pos.y() - bounds.height() / 2) / (bounds.height() / 2);

    getControllerManager()->updateMouseState(x, y, m_mousePressed);
}

void EmulatorItem::mousePressEvent(QMouseEvent *event) {
    m_mousePressed = true;
    getControllerManager()->updateMousePressed(m_mousePressed);
}

void EmulatorItem::mouseReleaseEvent(QMouseEvent *event) {
    m_mousePressed = false;
    getControllerManager()->updateMousePressed(m_mousePressed);
}

void EmulatorItem::startGame(const QByteArray &gameData, const QByteArray &saveData, const QString &corePath,
                             const QString &contentHash,
                             const unsigned int saveSlotNumber, const unsigned int platformId,
                             const QString &contentPath) {
    m_gameData = gameData;
    m_saveData = saveData;
    m_corePath = corePath;
    m_contentHash = contentHash;
    m_saveSlotNumber = saveSlotNumber;
    m_platformId = platformId;
    m_contentPath = contentPath;

    QThreadPool::globalInstance()->start([this] {
        auto configProvider = getEmulatorConfigManager()->getCoreConfigFor(m_platformId, m_contentHash);
        auto m_core = std::make_unique<libretro::Core>(m_platformId, m_corePath.toStdString(), configProvider);

        m_audioManager = std::make_shared<AudioManager>([this] { emit audioBufferLevelChanged(); });
        m_core->setAudioReceiver(m_audioManager);
        m_core->setRetropadProvider(getControllerManager());
        m_core->setPointerInputProvider(getControllerManager());

        // Qt owns the renderer, so it will destoy it.
        m_renderer = new EmulatorItemRenderer(window()->rendererInterface()->graphicsApi(), std::move(m_core));

        m_renderer->onGeometryChanged([this](unsigned int width, unsigned int height, float aspectRatio) {
            updateGeometry(width, height, aspectRatio);
        });

        // m_core->init();

        // Setting these causes the item's geometry to be visible, and the renderer is initialized.
        // If an item is not visible, the renderer is not initialized.
        m_coreBaseWidth = 1;
        m_coreBaseHeight = 1;
        m_calculatedAspectRatio = 0.000001;
        m_coreAspectRatio = 0.000001;

        emit videoWidthChanged();
        emit videoHeightChanged();
        emit videoAspectRatioChanged();
    });
}

QQuickRhiItemRenderer *EmulatorItem::createRenderer() {
    // const auto renderer = new EmulatorItemRenderer(window()->rendererInterface()->graphicsApi(),
    //                                                m_core.get());
    //
    // renderer->onGeometryChanged([this](int width, int height, float aspectRatio) {
    //     updateGeometry(width, height, aspectRatio);
    // });

    return m_renderer;
}

void EmulatorItem::updateGeometry(unsigned int width, unsigned int height, float aspectRatio) {
    m_coreBaseWidth = width;
    m_coreBaseHeight = height;
    m_coreAspectRatio = aspectRatio;
    m_calculatedAspectRatio = static_cast<float>(m_coreBaseWidth) / static_cast<float>(m_coreBaseHeight);

    spdlog::info("width: {}, height: {}, aspectRatio: {}, calculatedAspectRatio: {}", m_coreBaseWidth, m_coreBaseHeight,
                 m_coreAspectRatio, m_calculatedAspectRatio);
    setFixedColorBufferWidth(m_coreBaseWidth);
    setFixedColorBufferHeight(m_coreBaseHeight);
    emit videoWidthChanged();
    emit videoHeightChanged();
    emit videoAspectRatioChanged();
}
