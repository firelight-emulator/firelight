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

    contentItem: Icon {
        name: "more_vert"
        color: Theme.textPrimary
        size: 21
    }
}