import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FirelightStyle 1.0

Item {
    id: control
    property string fontFamily: "Arial"
    readonly property int buttonWidth: 200

    property int currentIndex: 0
    property bool showNowRunning: true

    signal indexSelected(int index)

    signal closeGameClicked()

    ButtonGroup {
        id: navPositionGroup
        buttons: column.children.filter(function (child) {
            return child instanceof Button;
        });
    }

    ColumnLayout {
        id: column
        anchors.fill: parent
        spacing: 2

        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 160
            Layout.minimumHeight: 160
            Layout.preferredWidth: buttonWidth
            color: "transparent"
        }

        FirelightMenuItem {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 40
            Layout.preferredWidth: buttonWidth

            checked: control.currentIndex === 0

            onClicked: {
                indexSelected(0)
            }

            labelText: "Home"
            labelIcon: "\ue88a"
        }

        FirelightMenuItem {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 40
            Layout.preferredWidth: buttonWidth

            checked: control.currentIndex === 1

            onClicked: {
                indexSelected(1)
            }

            labelText: "Explore"
            labelIcon: "\ue8d0"
        }

        FirelightMenuItem {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 40
            Layout.preferredWidth: buttonWidth

            checked: control.currentIndex === 2

            onClicked: {
                indexSelected(2)
            }

            labelText: "Library"
            labelIcon: "\uf53e"
        }

        FirelightMenuItem {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 40
            Layout.preferredWidth: buttonWidth

            checked: control.currentIndex === 3

            onClicked: {
                indexSelected(3)
            }

            labelText: "Controllers"
            labelIcon: "\uf135"
        }

        FirelightMenuItem {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 40
            Layout.preferredWidth: buttonWidth

            checked: control.currentIndex === 4

            onClicked: {
                indexSelected(4)
            }

            labelText: "Settings"
            labelIcon: "\ue8b8"
        }

        Rectangle {
            id: spacerOne
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 12
            Layout.minimumHeight: 12
            Layout.maximumHeight: 12
            Layout.fillWidth: true
            color: "transparent"

            visible: control.showNowRunning
        }

        Rectangle {
            id: horizontalRule
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 1
            Layout.fillWidth: true
            color: Constants.colorTestTextMuted

            visible: control.showNowRunning
        }

        Rectangle {
            id: spacerTwo
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 12
            Layout.minimumHeight: 12
            Layout.maximumHeight: 12
            Layout.fillWidth: true
            color: "transparent"

            visible: control.showNowRunning
        }

        FirelightMenuItem {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 40
            Layout.preferredWidth: buttonWidth

            checked: control.currentIndex === 5

            onClicked: {
                indexSelected(5)
            }

            labelText: "Now Playing"
            labelIcon: "\ue037"

            visible: control.showNowRunning
        }

        Item {
            Layout.fillHeight: true
        }
    }
}