import QtQuick
import QtQuick.Controls

Button {
    id: root
    padding: 2

    hoverEnabled: true

    background: Rectangle {
        color: root.hovered ? "#ffffff" : "transparent"
        opacity: 0.1
        radius: height / 2
    }

    contentItem: Text {
        font.family: Constants.symbolFontFamily
        text: "\ue5d4"
        color: "white"
        font.pointSize: 16
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}