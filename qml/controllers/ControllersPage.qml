import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


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
                text: "Controllers"
                color: Constants.purpleText
                font.pointSize: 24
                font.family: Constants.semiboldFontFamily
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    RowLayout {
        id: rowLayout
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 2 * parent.height / 3
        spacing: 8

        Pane {
            Layout.preferredWidth: parent.width / 5
            Layout.preferredHeight: parent.width / 5
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

            background: Rectangle {
                color: "transparent"
                radius: 4
                opacity: 0.3
                border.color: "black"
                border.width: 1
            }

            Text {
                anchors.fill: parent
                text: "No controller connected"
                color: "black"
                opacity: 0.5
                font.pointSize: 11
                font.family: Constants.regularFontFamily
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        Pane {
            Layout.preferredWidth: parent.width / 5
            Layout.preferredHeight: parent.width / 5
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

            background: Rectangle {
                color: "transparent"
                radius: 4
                opacity: 0.3
                border.color: "black"
                border.width: 1
            }

            Text {
                anchors.fill: parent
                text: "No controller connected"
                color: "black"
                opacity: 0.5
                font.pointSize: 11
                font.family: Constants.regularFontFamily
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        Pane {
            Layout.preferredWidth: parent.width / 5
            Layout.preferredHeight: parent.width / 5
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

            background: Rectangle {
                color: "transparent"
                radius: 4
                opacity: 0.3
                border.color: "black"
                border.width: 1
            }

            Text {
                anchors.fill: parent
                text: "No controller connected"
                color: "black"
                opacity: 0.5
                font.pointSize: 11
                font.family: Constants.regularFontFamily
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        Pane {
            Layout.preferredWidth: parent.width / 5
            Layout.preferredHeight: parent.width / 5
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

            background: Rectangle {
                color: "transparent"
                radius: 4
                opacity: 0.3
                border.color: "black"
                border.width: 1
            }

            Text {
                anchors.fill: parent
                text: "No controller connected"
                color: "black"
                opacity: 0.5
                font.pointSize: 11
                font.family: Constants.regularFontFamily
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }
}