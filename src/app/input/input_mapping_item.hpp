#pragma once

#include <QObject>
#include <QVariantMap>
#include <firelight/libretro/retropad.hpp>

namespace firelight::input {
    class InputMappingItem : public QObject {
        Q_OBJECT
        Q_PROPERTY(QVariantMap inputMappings READ getInputMappings WRITE setInputMappings NOTIFY inputMappingsChanged)

    public:
        QVariantMap getInputMappings() const;

        void setInputMappings(const QVariantMap &inputMappings);

        Q_INVOKABLE void setButtonMapping(int retropadInput, int sdlButton);

    signals:
        void inputMappingsChanged();

    private:
        QVariantMap m_inputMappings{
            {QString::number(libretro::IRetroPad::Input::NorthFace), 1},
            {QString::number(libretro::IRetroPad::Input::EastFace), 2},
            {QString::number(libretro::IRetroPad::Input::SouthFace), 3},
            {QString::number(libretro::IRetroPad::Input::WestFace), 0},
            {QString::number(libretro::IRetroPad::Input::Select), 8},
            {QString::number(libretro::IRetroPad::Input::Start), 9},
            {QString::number(libretro::IRetroPad::Input::LeftBumper), 4},
            {QString::number(libretro::IRetroPad::Input::RightBumper), 5},
            {QString::number(libretro::IRetroPad::Input::LeftTrigger), 6},
            {QString::number(libretro::IRetroPad::Input::RightTrigger), 7},
            {QString::number(libretro::IRetroPad::Input::DpadUp), 12},
            {QString::number(libretro::IRetroPad::Input::DpadDown), 13},
            {QString::number(libretro::IRetroPad::Input::DpadLeft), 14},
            {QString::number(libretro::IRetroPad::Input::DpadRight), 15}
        };
    };
} // input
// firelight
