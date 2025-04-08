import QtQuick
import QtQuick.Controls
import Firelight 1.0

Button {
    id: root
    verticalPadding: 6
    horizontalPadding: 8

    property bool showGlobalCursor: true
    // property Item globalCursorProxy: mask

    // containmentMask: Rectangle {
    //     id: mask
    //     color: "transparent"
    //     radius: 4
    //     x: -8
    //     width: root.width + 16
    //     y: -6
    //     height: root.height + 12
    // }

    // leftInset: -8
    // rightInset: -8
    // topInset: -6
    // bottomInset: -6
    required property string label
    required property string iconName

    background: Rectangle {
        color: "white"
        opacity: root.pressed ? 0.16 : root.activeFocus ? 0.12 : 0.08
        radius: 4
        visible: hoverHandler.hovered || root.pressed || root.activeFocus
    }

    HoverHandler {
        id: hoverHandler
        cursorShape: Qt.PointingHandCursor
    }

    contentItem: Row {
        spacing: 20

        FLIcon {
            width: parent.height
            height: parent.height
            size: parent.height
            icon: root.iconName
        }

        Text {
            text: root.label
            font.pixelSize: 17
            font.weight: Font.DemiBold
            font.family: Constants.regularFontFamily
            color: "#DDDDDD"
            height: parent.height
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }
    }


}
