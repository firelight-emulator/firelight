import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


Option {
    id: root
    control: ComboBox {
        model: ["Windowed", "Fullscreen"]
    }
}


