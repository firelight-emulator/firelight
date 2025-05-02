import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0
import Firelight 1.0

Button {
    id: root
    verticalPadding: 12
    horizontalPadding: 12

    implicitHeight: 50

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

    contentItem: Item {
        FLIcon {
            id: icon
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            size: 28
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

            // opacity: parent.width >  ? 0 : 1

            elide: Text.ElideRight
            text: root.label
            font.pixelSize: 18
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
