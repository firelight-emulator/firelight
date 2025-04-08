import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import Firelight 1.0

Image {
    id: root

    required property var size
    required property string icon
    property color color: "#e8eaed"

    source: "qrc:/icons/" + root.icon

    sourceSize.width: root.size
    sourceSize.height: root.size

    layer.enabled: true
    layer.effect: MultiEffect {
        source: root
        colorization: 1
        colorizationColor: root.color
    }
}
