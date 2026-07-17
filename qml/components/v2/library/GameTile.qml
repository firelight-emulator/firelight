import QtQuick
import QtQuick.Effects
import Firelight 1.0

// A game boxart thumbnail. `source` is the art url (empty or still-loading
// shows the placeholder frame); `size` sets the tile edge and drives the decode
// resolution so a scaled-up tile stays crisp without over-decoding a small one.
Item {
    id: root

    property url source: ""
    property int size: 140
    property real radius: AppStyle.radiusMd

    implicitWidth: size
    implicitHeight: size

    // Placeholder / letterbox behind the art.
    Rectangle {
        anchors.fill: parent
        radius: root.radius
        color: Theme.surfaceElevated
    }

    Image {
        id: img
        anchors.fill: parent
        source: root.source
        visible: status === Image.Ready
        fillMode: Image.PreserveAspectFit
        sourceSize.width: Math.min(512, root.size * 2)
        sourceSize.height: Math.min(512, root.size * 2)
        mipmap: true
        smooth: true
        layer.enabled: root.radius > 0
        layer.effect: MultiEffect {
            maskEnabled: true
            maskSource: mask
        }
    }

    Rectangle {
        id: mask
        anchors.fill: parent
        radius: root.radius
        visible: false
        layer.enabled: true
    }
}
