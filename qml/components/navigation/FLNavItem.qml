import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0
import Firelight 1.0

Button {
    id: root
    verticalPadding: 6
    horizontalPadding: 6

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

    contentItem: RowLayout {
        spacing: 12

        FLIcon {
            Layout.preferredWidth: parent.height
            Layout.preferredHeight: parent.height
            size: parent.height
            icon: root.iconName
            filled: root.checked
            color: root.checked ? "#FFFFFF" : "#a8a8a8"
        }

        Text {
            id: labelText
            opacity: parent.width > 140 ? 1 : 0
            text: root.label
            font.pixelSize: 17
            font.weight: Font.DemiBold
            font.family: Constants.regularFontFamily
            color: root.checked ? "#FFFFFF" : "#a8a8a8"
            Layout.fillHeight: true
            Layout.fillWidth: true
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
