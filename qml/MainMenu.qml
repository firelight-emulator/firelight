import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts 1.0
import QtQuick.Effects
import Firelight 1.0
import FirelightStyle 1.0

Item {

    signal gameStartRequested(int entryId)

    Component {
        id: thingy

        Pane {
            id: header
            property alias text: headerLabel.text

            background: Item {
            }

            Column {
                anchors.fill: parent
                spacing: 8

                Text {
                    id: headerLabel
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
    }

    LibraryPage {
        id: libraryPage
        visible: false

        onEntryClicked: function (id) {
            gameStartRequested(id)
        }
    }

    ControllersPage {
        id: controllerPage
        visible: false
    }

    SettingsPage {
        id: settingsPage
        visible: false
    }

    NowPlayingPage {
        id: nowPlayingPage
        visible: false
    }


    Pane {
        id: drawer
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        width: 48
        background: Rectangle {
            color: "#000000"
        }
        padding: 4
        KeyNavigation.right: stackview

        Behavior on width {
            NumberAnimation {
                duration: 200
                easing.type: Easing.InOutQuad
            }
        }

        Button {
            id: theButton
            width: 48
            height: 48
            hoverEnabled: true
            background: Rectangle {
                color: "#dadada"
                opacity: theButton.hovered ? 0.2 : 0
                radius: 6
            }

            onPressed: function () {
                if (drawer.width === 220) {
                    drawer.width = 48
                } else {
                    drawer.width = 220
                }
            }

            anchors.top: parent.top
            anchors.right: parent.right
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 4

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 10
            }

            Text {
                text: "Firelight"
                opacity: parent.width > 48 ? 1 : 0
                color: "#dadada"
                font.pointSize: 16
                font.family: Constants.semiboldFontFamily
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            Text {
                text: "alpha (0.4.0a)"
                opacity: parent.width > 48 ? 1 : 0
                color: "#dadada"
                font.pointSize: 10
                font.family: Constants.regularFontFamily
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            Text {
                Layout.fillWidth: true
                Layout.preferredHeight: 10
            }

            NavMenuButton {
                id: homeNavButton
                KeyNavigation.down: libraryNavButton
                labelText: "Home"
                labelIcon: "\ue88a"
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 40
                enabled: false

                onToggled: function () {
                    stackview.replace(thingy, {text: "Home"})
                }
            }
            NavMenuButton {
                id: libraryNavButton
                KeyNavigation.down: modNavButton
                labelText: "Library"
                labelIcon: "\uf53e"
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 40
                checked: true

                onToggled: function () {
                    stackview.replace(libraryPage)
                }
            }
            NavMenuButton {
                id: modNavButton
                KeyNavigation.down: controllersNavButton
                labelText: "Mods"
                labelIcon: "\uef48"
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 40

                onToggled: function () {
                    stackview.replace(thingy, {text: "Mods"})
                }

                enabled: false
            }
            NavMenuButton {
                id: controllersNavButton
                KeyNavigation.down: nowPlayingNavButton
                labelText: "Controllers"
                labelIcon: "\uf135"
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 40

                onToggled: function () {
                    stackview.replace(controllerPage)
                }
            }
            Rectangle {
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 1
                opacity: 0.3
                color: "#dadada"
            }
            NavMenuButton {
                id: nowPlayingNavButton
                KeyNavigation.down: settingsNavButton
                labelText: "Now Playing"
                labelIcon: "\ue037"
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 40

                onToggled: function () {
                    stackview.replace(nowPlayingPage)
                }
            }
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
            NavMenuButton {
                id: settingsNavButton
                labelText: "Settings"
                labelIcon: "\ue8b8"
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 40

                onToggled: function () {
                    stackview.replace(settingsPage)
                }
            }
            Rectangle {
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 1
                opacity: 0.3
                color: "#dadada"
            }
            NavMenuButton {
                labelText: "Profile"
                labelIcon: "\ue853"
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 40
                enabled: false

                onToggled: function () {
                    stackview.replace(thingy, {text: "Profile"})
                }
            }
        }
    }

    Pane {
        id: rightSide

        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.left: drawer.right

        background: Item {
        }

        StackView {
            id: stackview
            anchors.fill: parent

            initialItem: libraryPage

            pushEnter: Transition {
                PropertyAnimation {
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 200
                }
            }
            pushExit: Transition {
                PropertyAnimation {
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: 200
                }
            }
            popEnter: Transition {
                PropertyAnimation {
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 200
                }
            }
            popExit: Transition {
                PropertyAnimation {
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: 200
                }
            }
            replaceEnter: Transition {
                ParallelAnimation {
                    PropertyAnimation {
                        property: "opacity"
                        from: 0
                        to: 1
                        duration: 400
                    }
                    PropertyAnimation {
                        property: "x"
                        from: 20
                        to: 0
                        duration: 250
                    }
                }
            }
            replaceExit: Transition {
            }
        }
    }
}
