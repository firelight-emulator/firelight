import QtQuick
import QtQuick.Controls

ToolTip {
    id: root

    horizontalPadding: 8
    verticalPadding: 6

    background: Rectangle {
        color: ColorPalette.neutral900
        radius: 4
        border.color: ColorPalette.neutral700
        border.width: 1
    }

    contentItem: Text {
        text: root.text
        font.pixelSize: 16
        font.family: Constants.mainFontFamily
        color: ColorPalette.neutral100
    }

    enter: Transition {
        NumberAnimation {
            property: "opacity"
            from: 0
            to: 1
            duration: 120
            easing.type: Easing.InOutQuad
        }
    }

    exit: Transition {
        NumberAnimation {
            property: "opacity"
            from: 1
            to: 0
            duration: 120
            easing.type: Easing.InOutQuad
        }
    }
}