import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0
import Firelight 1.0

Button {
    id: root
    verticalPadding: 6
    horizontalPadding: 6

    implicitHeight: 38

    autoExclusive: true
    property bool showGlobalCursor: true
    checkable: true
    
    required property string label
    required property string iconName

    background: Rectangle {
        color: "white"
        opacity: root.pressed ? 0.16 : root.checked ? 0.12 : 0.08
        radius: 4
        visible: hoverHandler.hovered || root.pressed || root.checked
    }

    HoverHandler {
        id: hoverHandler
        cursorShape: Qt.PointingHandCursor
    }

    FLToolTip {
        id: tooltip
        text: root.label
        visible: (hoverHandler.hovered || root.activeFocus) && parent.width <= 38
        delay: 0
        y: parent.height / 2 - tooltip.height / 2
        x: parent.width + 8
    }

    contentItem: Item {
        FLIcon {
            id: icon
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            size: 26
            icon: root.iconName
            filled: root.checked
            color: root.checked ? "#FFFFFF" : "#c3c3c3"
        }

        Text {
            id: labelText
            anchors.leftMargin: 12
            anchors.left: icon.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right

            opacity: parent.width > 38 ? 1 : 0

            elide: Text.ElideRight
            text: root.label
            font.pixelSize: 16
            font.weight: Font.DemiBold
            font.family: Constants.regularFontFamily
            color: root.checked ? "#FFFFFF" : "#c3c3c3"
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter

            Behavior on opacity {
                NumberAnimation {
                    duration: 100
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }


}
