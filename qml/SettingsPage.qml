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
                text: "Settings"
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
        width: parent.width / 3

        ColumnLayout {
            id: menu
            anchors.fill: parent
            spacing: 4

            FirelightMenuItem {
                labelText: "Appearance"
                // labelIcon: "\ue40a"
                Layout.preferredWidth: parent.width / 2
                Layout.preferredHeight: 40
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                enabled: false
            }
            FirelightMenuItem {
                labelText: "Video"
                // labelIcon: "\ue333"
                Layout.preferredWidth: parent.width / 2
                Layout.preferredHeight: 40
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                checked: true
            }
            FirelightMenuItem {
                labelText: "Sound"
                // labelIcon: "\ue050"
                Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 40
                enabled: false
            }
            FirelightMenuItem {
                labelText: "System"
                // labelIcon: "\uf522"
                Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 40
                enabled: false
            }
            Rectangle {
                Layout.preferredWidth: parent.width / 2
                Layout.preferredHeight: 1
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                opacity: 0.3
                color: "#dadada"
            }
            FirelightMenuItem {
                labelText: "Privacy"
                // labelIcon: "\ue897"
                Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 40
                enabled: false
            }
            FirelightMenuItem {
                labelText: "About"
                // labelIcon: "\ue88e"
                Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 40
                enabled: false
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }

    // Item {
    //     id: rightHalf
    //     anchors.right: parent.right
    //     anchors.top: header.bottom
    //     anchors.bottom: parent.bottom
    //     anchors.left: leftHalf.right
    //
    //
    //     Rectangle {
    //         id: preview
    //         width: 2 * parent.width / 3
    //         height: width * 9 / 16
    //         anchors.left: parent.left
    //         anchors.verticalCenter: parent.verticalCenter
    //
    //         color: "lightblue"
    //     }
    // }


}