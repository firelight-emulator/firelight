import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects


Menu {
    id: control

    padding: 4
    // overlap: 10

    // contentWidth: 260
    // contentHeight: Constants.rightClickMenuItem_DefaultHeight

    // implicitContentWidth: 260
    // // implicitContentHeight: control.count * Constants.rightClickMenuItem_DefaultHeight

    background: Rectangle {
        id: bg
        implicitWidth: 260
        implicitHeight: 60
        color: "#1d1e22"
        radius: 2
    }

    delegate: RightClickMenuItem {
        id: delegate
        text: text
    }
}