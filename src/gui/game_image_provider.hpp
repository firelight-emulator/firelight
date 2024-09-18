#pragma once

#include "models/suspend_point_list_model.hpp"


namespace firelight::gui {
    class GameImageProvider : public QQuickImageProvider {
        Q_OBJECT

    public:
        GameImageProvider();

      ~GameImageProvider();

        QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

        void setImage(const QString &id, const QImage &image);

    private:
        QStringList m_ids;
        QMap<QString, QImage> m_images{};
        // emulation::RewindModel *m_rewindModel;
    };
} // gui
// firelight
