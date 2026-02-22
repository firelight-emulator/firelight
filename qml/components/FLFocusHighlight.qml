import QtQuick
import QtQuick.Controls
import QtMultimedia

Item {
    id: root

    property color borderColor: "#ff9f38"
    property real borderWidth: 2
    property real bounceAmplitude: 16
    property real defaultAnchorMargins: -4
    property int animationDuration: 85
    z: 1000

    required property Item target
    required property bool usingMouse

    visible: !usingMouse

    // Track which highlight is currently "active"
    property bool useHighlightA: true

    function calculateRadius(item: Item): real {
        if (item === null) {
            return Math.abs(root.defaultAnchorMargins)
        }

        if (item.hasOwnProperty('background') && item.background && item.background.hasOwnProperty('radius')) {
            return item.background.radius + Math.abs(root.defaultAnchorMargins)
        } else if (item.hasOwnProperty('radius')) {
            return item.radius + Math.abs(root.defaultAnchorMargins)
        } else {
            return Math.abs(root.defaultAnchorMargins)
        }
    }

    function getMargins(item: Item): real {
        if (item !== null && item.hasOwnProperty('globalCursorSpacing')) {
            return -item.globalCursorSpacing
        }
        return root.defaultAnchorMargins
    }

    // Two highlights that crossfade via re-parenting
    Rectangle {
        id: highlightA
        opacity: 0
        visible: !root.usingMouse
        anchors.fill: parent
        anchors.margins: parent ? root.getMargins(parent) : root.defaultAnchorMargins
        border.color: root.borderColor
        border.width: root.borderWidth
        color: "transparent"
        radius: root.calculateRadius(parent)
        z: 1000

        Behavior on opacity { NumberAnimation { duration: root.animationDuration; easing.type: Easing.OutQuad } }
    }

    Rectangle {
        id: highlightB
        opacity: 0
        visible: !root.usingMouse
        anchors.fill: parent
        anchors.margins: parent ? root.getMargins(parent) : root.defaultAnchorMargins
        border.color: root.borderColor
        border.width: root.borderWidth
        color: "transparent"
        radius: root.calculateRadius(parent)
        z: 1000

        Behavior on opacity { NumberAnimation { duration: root.animationDuration; easing.type: Easing.OutQuad } }
    }

    onTargetChanged: {
        if (target === null) {
            highlightA.opacity = 0
            highlightB.opacity = 0
            return
        }

        if (target.hasOwnProperty('showGlobalCursor') && target.showGlobalCursor) {
            var newParent
            if (target.hasOwnProperty("globalCursorProxy") && target.globalCursorProxy) {
                newParent = target.globalCursorProxy
            } else {
                newParent = target
            }

            // Crossfade: fade out old highlight (stays with old parent), fade in new one
            if (useHighlightA) {
                highlightA.parent = newParent
                highlightA.opacity = 1
                highlightB.opacity = 0
            } else {
                highlightB.parent = newParent
                highlightB.opacity = 1
                highlightA.opacity = 0
            }

            useHighlightA = !useHighlightA
            return
        }

        // Target doesn't have showGlobalCursor
        highlightA.opacity = 0
        highlightB.opacity = 0
    }
}
