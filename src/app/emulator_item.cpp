#include "emulator_item.hpp"
#include <QQuickWindow>
#include <rhi/qrhi_platform.h>
#include <spdlog/spdlog.h>

#include "emulator_item_renderer.hpp"
#include "platform_metadata.hpp"

EmulatorItem::EmulatorItem(QQuickItem *parent) : QQuickRhiItem(parent) {
    setFlag(ItemHasContents);
}

EmulatorItem::~EmulatorItem() {
    spdlog::info("Destroying EmulatorItem");
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
        m_core = std::make_unique<libretro::Core>(m_corePath.toStdString(), configProvider);

        m_audioManager = std::make_shared<AudioManager>();
        m_core->setAudioReceiver(m_audioManager);
        m_core->setRetropadProvider(getControllerManager());

        setFixedColorBufferHeight(1);
        setFixedColorBufferWidth(1);

        m_coreBaseWidth = 1;
        m_coreBaseHeight = 1;

        emit videoWidthChanged();
        emit videoHeightChanged();
    });
}

QQuickRhiItemRenderer *EmulatorItem::createRenderer() {
    const auto renderer = new EmulatorItemRenderer(window()->rendererInterface()->graphicsApi(),
                                                   m_core.get());

    renderer->onGeometryChanged([this](int width, int height, float aspectRatio) {
        updateGeometry(width, height, aspectRatio);
    });

    return renderer;
}

void EmulatorItem::updateGeometry(int width, int height, float aspectRatio) {
    m_coreBaseWidth = width;
    m_coreBaseHeight = height;
    m_coreAspectRatio = aspectRatio;
    m_calculatedAspectRatio = static_cast<float>(m_coreBaseWidth) / static_cast<float>(m_coreBaseHeight);

    setFixedColorBufferWidth(m_coreBaseWidth);
    setFixedColorBufferHeight(m_coreBaseHeight);
    emit videoWidthChanged();
    emit videoHeightChanged();
}
