import QtQuick
import QtQuick.Controls
import QtQuick.Effects

RoundButton {
    id: control
    height: 50
    width: 50

    icon.width: 32
    icon.height: 32
    icon.color: "white"

    property string tooltipText: ""

    property real xOffset: 0

    hoverEnabled: true

    property bool showGlobalCursor: true
    property int globalCursorSpacing: 1

    flat: true

    contentItem: Image {
        id: iconImage
        anchors.centerIn: parent
        source: control.icon.source
        sourceSize.width: control.icon.width
        sourceSize.height: control.icon.height
        fillMode: Image.PreserveAspectFit
        opacity: control.enabled ? 1 : 0.5

        layer.enabled: true
        layer.effect: MultiEffect {
            source: iconImage
            colorization: 1
            colorizationColor: "white"
        }
        // transform: Translate { x: control.xOffset }
    }

    background: Rectangle {
        color: control.pressed || control.hovered ? ColorPalette.neutral100 : "transparent"
        radius: control.width / 2
        opacity: !control.enabled ? 0 : control.pressed ? 0.14 : 0.23
    }

    FLToolTip {
        y: control.height / 2 - height / 2 - verticalPadding / 2
        x: control.width + 8
        visible: (control.hovered || (!InputMethodManager.usingMouse && activeFocus)) && control.tooltipText !== ""
        text: control.tooltipText
    }
}
