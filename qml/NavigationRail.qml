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

    FontLoader {
        id: symbols
        source: "qrc:/fonts/symbols"
    }

    ButtonGroup {
        id: navPositionGroup
        buttons: column.children.filter(function (child) {
            return child instanceof Button;
        });
    }

    ColumnLayout {
        id: column
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 160
            Layout.minimumHeight: 160
            Layout.preferredWidth: buttonWidth
            color: "transparent"
        }

        NavButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 48
            Layout.preferredWidth: buttonWidth

            checked: control.currentIndex === 0

            onClicked: {
                indexSelected(0)
            }

            labelText: "Home"
            labelIcon: "\ue88a"
        }

        NavButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 48
            Layout.preferredWidth: buttonWidth

            checked: control.currentIndex === 1

            onClicked: {
                indexSelected(1)
            }

            labelText: "Explore"
            labelIcon: "\ue8d0"
        }

        NavButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 48
            Layout.preferredWidth: buttonWidth

            checked: control.currentIndex === 2

            onClicked: {
                indexSelected(2)
            }

            labelText: "Library"
            labelIcon: "\uf53e"
        }

        NavButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 48
            Layout.preferredWidth: buttonWidth

            checked: control.currentIndex === 3

            onClicked: {
                indexSelected(3)
            }

            labelText: "Controllers"
            labelIcon: "\uf135"
        }

        NavButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 48
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

        NavButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 48
            Layout.preferredWidth: buttonWidth

            checked: control.currentIndex === 5

            onClicked: {
                indexSelected(5)
            }

            labelText: "Now Playing"
            labelIcon: "\ue037"

            visible: control.showNowRunning
        }

        NavButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 48
            Layout.preferredWidth: buttonWidth

            checkable: false

            onClicked: {
                closeGameClicked()
            }

            labelText: "Close Game"
            labelIcon: "\ue5cd"

            visible: control.showNowRunning
        }

        Item {
            Layout.fillHeight: true
        }
    }

    component NavButton: Button {
        id: navButtonComponent

        // autoExclusive: true
        checkable: true
        property string labelText
        property string labelIcon
        // radius: 12
        // icon.name: "computer"
        // icon.color: "black"

        // horizontalPadding: 12
        // verticalPadding: 10
        // display: AbstractButton.TextBesideIcon

        // icon.font.family: symbols.name
        contentItem: Item {
            anchors.fill: parent

            Text {
                id: buttonIcon
                text: navButtonComponent.labelIcon
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                leftPadding: 12
                rightPadding: 12
                // width: 24

                font.family: symbols.name
                font.pixelSize: 24
                color: navButtonComponent.checked ? Constants.colorTestTextActive : Constants.colorTestText
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                // Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            }
            Text {
                id: buttonText
                text: navButtonComponent.labelText
                anchors.left: buttonIcon.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                font.pointSize: 12
                font.family: control.fontFamily
                color: navButtonComponent.checked ? Constants.colorTestTextActive : Constants.colorTestText
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                // Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            }
        }

        // onClicked: function () {
        //     navButtonComponent.toggle()
        //     console.log(navButtonComponent.checked)
        // }

        background: Rectangle {
            color: navButtonComponent.checked ? Constants.colorTestCardActive : mouse.hovered ? Constants.colorTestCard : "transparent"
            // color: navButtonComponent.checked ? Constants.color_secondaryContainer : "transparent"
            radius: 12
            // color: "transparent"
        }

        HoverHandler {
            id: mouse
            acceptedDevices: PointerDevice.Mouse
            cursorShape: Qt.PointingHandCursor
        }
    }
}