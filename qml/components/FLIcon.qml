import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import Firelight 1.0

Image {
    id: root

    required property var size
    required property string icon
    property bool filled: true
    smooth: false

    property color color: "#e8eaed"

    source: filled ? ("qrc:/icons/" + root.icon) : ("qrc:/icons/empty/" + root.icon)
    fillMode: Image.PreserveAspectFit

    sourceSize.width: root.size
    sourceSize.height: root.size

    layer.enabled: true
    layer.effect: MultiEffect {
        source: root
        colorization: 1
        colorizationColor: root.color
    }
}
