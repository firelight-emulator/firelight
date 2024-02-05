import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FirelightStyle 1.0

Item {
    id: control
    property string fontFamily: "Arial"
    readonly property int buttonWidth: 188

    signal homeClicked()

    signal exploreClicked()

    signal libraryClicked()

    signal controllersClicked()

    signal settingsClicked()

    FontLoader {
        id: symbols
        source: "qrc:/fonts/symbols"
    }

    ButtonGroup {
        id: navPositionGroup
        buttons: column.children
    }

    Rectangle {
        id: activeIndicator
        x: column.x + navPositionGroup.checkedButton.x
        y: column.y + navPositionGroup.checkedButton.y
        width: navPositionGroup.checkedButton.width
        height: navPositionGroup.checkedButton.height

        // Behavior on x {
        //     NumberAnimation {
        //         duration: 100
        //         easing.type: Easing.InOutQuad;
        //     }
        // }
        //
        // Behavior on y {
        //     NumberAnimation {
        //         duration: 100
        //         easing.type: Easing.InOutQuad;
        //     }
        // }

        radius: 12

        color: Constants.colorTestCardActive
    }

    ColumnLayout {
        id: column
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 0

        NavButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 48
            Layout.preferredWidth: buttonWidth

            checked: true

            onClicked: {
                homeClicked()
            }

            labelText: "Home"
            labelIcon: "\ue88a"
        }

        NavButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 48
            Layout.preferredWidth: buttonWidth

            onClicked: {
                exploreClicked()
            }

            labelText: "Explore"
            labelIcon: "\ue8d0"
        }

        NavButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 48
            Layout.preferredWidth: buttonWidth

            onClicked: {
                libraryClicked()
            }

            labelText: "Library"
            labelIcon: "\uf53e"
        }

        NavButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 48
            Layout.preferredWidth: buttonWidth

            onClicked: {
                controllersClicked()
            }

            labelText: "Controllers"
            labelIcon: "\uf135"
        }

        NavButton {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 48
            Layout.preferredWidth: buttonWidth

            onClicked: {
                settingsClicked()
            }

            labelText: "Settings"
            labelIcon: "\ue8b8"
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
            color: navButtonComponent.checked ? "transparent" : mouse.hovered ? Constants.colorTestCard : "transparent"
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