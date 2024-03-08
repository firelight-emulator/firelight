import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import FirelightStyle 1.0

Menu {
    id: control

    padding: Constants.rightClickMenu_Padding
    overlap: 10

    background: Rectangle {
        id: bg
        implicitWidth: 200
        implicitHeight: control.count * Constants.rightClickMenuItem_DefaultHeight
        color: Constants.rightClickMenu_BackgroundColor
        radius: Constants.rightClickMenu_BackgroundRadius
    }

    delegate: FirelightRightClickMenuItem {
        id: delegate
        text: text
    }
}