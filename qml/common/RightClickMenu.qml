import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import FirelightStyle 1.0

Menu {
    id: control

    padding: Constants.rightClickMenu_Padding
    // overlap: 10

    // contentWidth: 260
    // contentHeight: Constants.rightClickMenuItem_DefaultHeight

    // implicitContentWidth: 260
    // // implicitContentHeight: control.count * Constants.rightClickMenuItem_DefaultHeight

    background: Rectangle {
        id: bg
        implicitWidth: 260
        implicitHeight: 48
        color: Constants.rightClickMenu_BackgroundColor
        radius: Constants.rightClickMenu_BackgroundRadius
    }

    delegate: RightClickMenuItem {
        id: delegate
        text: text
    }
}