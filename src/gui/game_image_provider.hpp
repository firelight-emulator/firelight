#pragma once
#include <QQuickImageProvider>

#include "models/rewind_model.hpp"


namespace firelight::gui {
    class GameImageProvider : public QQuickImageProvider {
        Q_OBJECT

    public:
        explicit GameImageProvider(emulation::RewindModel *rewindModel);

        QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

    private:
        emulation::RewindModel *m_rewindModel;
    };
} // gui
// firelight
