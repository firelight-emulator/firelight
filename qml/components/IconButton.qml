import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import Firelight 1.0

RoundButton {
    id: control
    height: 50
    width: 50

    icon.width: 32
    icon.height: 32
    icon.color: "white"

    hoverEnabled: true

    property bool showGlobalCursor: true
    property int globalCursorSpacing: 1

    flat: true

    background: Rectangle {
        color: control.hovered ? ColorPalette.neutral100 : "transparent"
        radius: control.width / 2
        opacity: control.pressed ? 0.14 : 0.23
    }

    ToolTip {
        y: control.height / 2 - height / 2 - 4
        x: control.width + 8
        visible: control.hovered || control.activeFocus

        background: Item {}
        contentItem: Text {
            text: "Sort"
            color: "white"
            font.family: Constants.regularFontFamily
            font.pixelSize: 20
            font.weight: Font.Medium
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
}
