import QtQuick
import QtQuick.Controls
import QtMultimedia

Rectangle {
    id: activeFocusHighlight

    property color borderColor: "#ff9f38"
    property real borderWidth: 2
    property real bounceAmplitude: 16
    // property MediaPlayer bumpSfx: undefined

    required property Item target
    required property bool usingMouse

    visible: !usingMouse && parent !== null

    anchors.fill: parent
    anchors.margins: -4
    border.color: borderColor
    border.width: borderWidth
    color: "transparent"

    radius: {
        if (activeFocusHighlight.parent === null || activeFocusHighlight.parent === undefined) {
            return Math.abs(anchors.margins)
        }

        if (activeFocusHighlight.parent.hasOwnProperty('background')) {
            return parent.background.radius + (Math.abs(anchors.margins))
        } else if (activeFocusHighlight.parent.hasOwnProperty('radius')) {
            return parent.radius + (Math.abs(anchors.margins))
        } else {
            return Math.abs(anchors.margins)
        }
    }

    onTargetChanged: {
        if (target === null) {
            activeFocusHighlight.parent = null
            return
        }

        if (target.hasOwnProperty('showGlobalCursor') && target.showGlobalCursor) {
            if (target.hasOwnProperty("globalCursorProxy") && target.globalCursorProxy) {
                activeFocusHighlight.parent = target.globalCursorProxy
            } else {
                activeFocusHighlight.parent = target
            }
            return
        }

        activeFocusHighlight.parent = null
    }
}
