import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FirelightStyle 1.0

Item {
    id: root

    Pane {
        id: header
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        background: Item {
        }

        Column {
            anchors.fill: parent
            spacing: 8

            Text {
                text: "Now Playing"
                color: "#dadada"
                font.pointSize: 24
                font.family: Constants.semiboldFontFamily
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }

            Rectangle {
                width: parent.width
                height: 1
                opacity: 0.3
                color: "#dadada"
            }
        }
    }

    Item {
        id: leftHalf
        anchors.left: parent.left
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        width: parent.width / 2

        ColumnLayout {
            id: menu
            anchors.fill: parent
            spacing: 4

            FirelightMenuItem {
                labelText: "Resume Game"
                Layout.preferredWidth: parent.width / 2
                Layout.preferredHeight: 40
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            }
            FirelightMenuItem {
                labelText: "Restart Game"
                Layout.preferredWidth: parent.width / 2
                Layout.preferredHeight: 40
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            }
            FirelightMenuItem {
                labelText: "Rewind"
                Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 40
            }
            Rectangle {
                Layout.preferredWidth: parent.width / 2
                Layout.preferredHeight: 1
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                opacity: 0.3
                color: "#dadada"
            }
            FirelightMenuItem {
                labelText: "Create Suspend Point"
                Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 40
            }
            FirelightMenuItem {
                labelText: "Load Suspend Point"
                Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 40
            }
            FirelightMenuItem {
                labelText: "Undo Last Load"
                Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 40
            }
            Rectangle {
                Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 1
                opacity: 0.3
                color: "#dadada"
            }
            FirelightMenuItem {
                labelText: "Quit Game"
                Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 40
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }

    Item {
        id: rightHalf
        anchors.right: parent.right
        anchors.top: header.bottom
        anchors.bottom: parent.bottom
        anchors.left: leftHalf.right


        Image {
            id: preview
            anchors.centerIn: parent
            smooth: false

            source: "file:pmscreenshot.jpg"
            // fillMode: Image.PreserveAspectFit
        }
    }


}