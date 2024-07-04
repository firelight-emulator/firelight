import QtQml
import QtQuick
import QtQuick.Controls

TabBar {
    id: control

    required property int tabWidth
    required property list<string> tabs

    currentIndex: 0

    background: Rectangle {
        width: control.tabWidth
        height: 2
        radius: 1
        color: "white"
        x: control.tabWidth * control.currentIndex
        y: control.height

        Behavior on x {
            NumberAnimation {
                duration: 120
                easing.type: Easing.InOutQuad
            }
        }
    }

    Repeater {
        model: control.tabs
        delegate: TabButton {
            required property var modelData

            width: control.tabWidth
            contentItem: Text {
                text: modelData
                color: parent.enabled ? "#ffffff" : "#666666"
                font.family: Constants.regularFontFamily
                font.pointSize: 11
                font.weight: parent.enabled && parent.checked ? Font.Bold : Font.Normal
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Item {
            }

            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
        }
    }
}