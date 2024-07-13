import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Option {
    id: root
    control: Slider {
        from: 0
        to: 50
        stepSize: 5
        value: 0
        snapMode: Slider.SnapAlways
    }
}