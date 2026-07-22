import QtQuick

Item {
    id: root

    property url source: ""
    property real radius: 0
    property int fillMode: Image.PreserveAspectFit
    property color background: "transparent"
    property alias status: img.status
    // TODO
    // Max decode size; non-zero and independent of layout so the image is never
    // decoded at full native resolution before the item is sized
    property int sourceSizePx: 512

    Item {
        id: content
        anchors.fill: parent

        layer.enabled: root.radius > 0
        layer.smooth: true
        layer.effect: FLRoundedMask {
            radius: root.radius
        }

        Rectangle {
            anchors.fill: parent
            color: root.background
        }

        Image {
            id: img
            anchors.fill: parent
            source: root.source
            fillMode: root.fillMode
            mipmap: true
            smooth: true
            asynchronous: true
            sourceSize.width: root.sourceSizePx
            sourceSize.height: root.sourceSizePx
        }
    }
}
