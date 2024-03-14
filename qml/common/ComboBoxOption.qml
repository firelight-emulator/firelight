import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FirelightStyle 1.0

Option {
    id: root
    control: ComboBox {
        model: ["Windowed", "Fullscreen"]
    }
}


