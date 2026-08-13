import QtQuick

Item {
    id: root

    property url source: ""
    property real radius: 0

    // TODO
    // Each corner follows radius until it is given its own, the way Rectangle's do
    property real topLeftRadius: radius
    property real topRightRadius: radius
    property real bottomLeftRadius: radius
    property real bottomRightRadius: radius
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

        // TODO
        // Any corner being rounded is reason enough to mask, so squaring off radius while
        // leaving one corner set still clips
        layer.enabled: root.topLeftRadius > 0 || root.topRightRadius > 0 || root.bottomLeftRadius > 0 || root.bottomRightRadius > 0
        layer.smooth: true
        layer.effect: FLRoundedMask {
            radius: root.radius
            topLeftRadius: root.topLeftRadius
            topRightRadius: root.topRightRadius
            bottomLeftRadius: root.bottomLeftRadius
            bottomRightRadius: root.bottomRightRadius
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
