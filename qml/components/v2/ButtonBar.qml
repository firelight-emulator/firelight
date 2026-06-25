import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts 1.0
import Firelight 1.0

Pane {
    id: control
    padding: 4

    implicitHeight: 36

    background: Rectangle {
        color: Qt.darker("#1b1d27", 1.2)
        radius: 8
        border.color: Qt.lighter("#1b1d27", 1.3)
        border.width: 1
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Button {
            id: dayButton
            Layout.preferredWidth: 74
            Layout.fillHeight: true
            checkable: true
            autoExclusive: true

            hoverEnabled: true
            background: Rectangle {
                color: "white"
                opacity: dayButton.down ? 0.065 : dayButton.hovered ? 0.1 : dayButton.checked ? 0.08 : 0
                radius: 6
            }

            Text {
                anchors.centerIn: parent
                height: parent.height
                text: qsTr("Day")
                color: "white"
                opacity: dayButton.checked ? 1.0 : 0.8
                font.family: Constants.regularFontFamily
                font.pointSize: 11
                font.weight: Font.Medium

                verticalAlignment: Qt.AlignVCenter
            }
        }

        Button {
            id: weekButton
            Layout.preferredWidth: 74
            Layout.fillHeight: true
            checkable: true
            autoExclusive: true

            hoverEnabled: true
            background: Rectangle {
                color: "white"
                opacity: weekButton.down ? 0.065 : weekButton.hovered ? 0.1 : weekButton.checked ? 0.08 : 0
                radius: 6
            }

            Text {
                anchors.centerIn: parent
                height: parent.height
                text: qsTr("Week")
                color: "white"
                opacity: weekButton.checked ? 1.0 : 0.8
                font.family: Constants.regularFontFamily
                font.pointSize: 11
                font.weight: Font.Medium

                verticalAlignment: Qt.AlignVCenter
            }
        }

        Button {
            id: monthButton
            Layout.preferredWidth: 74
            Layout.fillHeight: true
            checkable: true
            autoExclusive: true

            hoverEnabled: true
            background: Rectangle {
                color: "white"
                opacity: monthButton.down ? 0.065 : monthButton.hovered ? 0.1 : monthButton.checked ? 0.08 : 0
                radius: 6
            }

            Text {
                anchors.centerIn: parent
                height: parent.height
                text: qsTr("Month")
                color: "white"
                opacity: monthButton.checked ? 1.0 : 0.8
                font.family: Constants.regularFontFamily
                font.pointSize: 11
                font.weight: Font.Medium

                verticalAlignment: Qt.AlignVCenter
            }
        }

        Button {
            id: yearButton
            Layout.preferredWidth: 74
            Layout.fillHeight: true
            checkable: true
            autoExclusive: true

            hoverEnabled: true
            background: Rectangle {
                color: "white"
                opacity: yearButton.down ? 0.065 : yearButton.hovered ? 0.1 : yearButton.checked ? 0.08 : 0
                radius: 6
            }

            Text {
                anchors.centerIn: parent
                height: parent.height
                text: qsTr("Year")
                color: "white"
                opacity: yearButton.checked ? 1.0 : 0.8
                font.family: Constants.regularFontFamily
                font.pointSize: 11
                font.weight: Font.Medium

                verticalAlignment: Qt.AlignVCenter
            }
        }
    }
}