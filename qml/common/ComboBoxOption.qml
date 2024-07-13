import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


Option {
    id: root
    control: MyComboBox {
        model: ["Windowed", "Fullscreen"]
    }
}


