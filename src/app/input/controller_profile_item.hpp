#pragma once

#include <QObject>

namespace firelight::input {
    class ControllerProfileItem : public QObject {
        Q_OBJECT
        Q_PROPERTY(int profileId)
        Q_PROPERTY(QString displayName)
    };
} // input
// firelight
