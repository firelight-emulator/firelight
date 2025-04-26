import QtQml
import QtQuick
import QtQuick.Controls

TabBar {
    id: control

    required property int tabWidth
    required property list<string> tabs

    spacing: 8

    implicitWidth: tabWidth * tabs.length + (spacing * (tabs.length - 1))
    implicitHeight: 48

    currentIndex: 0

    background: Rectangle {
        objectName: "background"
        width: control.tabWidth
        height: 4
        radius: 2
        color: "#acacac"
        x: (control.tabWidth * control.currentIndex) + (control.spacing * control.currentIndex)
        y: control.height - 4

        Behavior on x {
            NumberAnimation {
                duration: 120
                easing.type: Easing.InOutQuad
            }
        }
    }

    Repeater {
        anchors.fill: parent
        model: control.tabs
        delegate: TabButton {
            required property var modelData
            required property var index

            objectName: "button" + index

            width: control.tabWidth
            height: control.height

            contentItem: Text {
                objectName: "text"
                height: control.height
                text: modelData
                color: parent.enabled ? parent.checked ? "#ffffff" : "#f0f0f0" : "#666666"
                font.family: Constants.regularFontFamily
                font.pixelSize: 15
                font.weight: parent.enabled && parent.checked ? Font.Bold : Font.Normal
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Item {
            }

            HoverHandler {
                objectName: "hoverHandler"
                cursorShape: Qt.PointingHandCursor
            }
        }
    }
}