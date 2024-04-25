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

    Component {
        id: libraryPage
        LibraryPage {
            property bool topLevel: true

            property string topLevelName: "library"
            onEntryClicked: function (id) {
                gameStartRequested(id)
            }
        }
    }

    Component {
        id: modsPage
        DiscoverPage {
            property bool topLevel: true
            property string topLevelName: "mods"
        }
    }


    Component {
        id: controllerPage
        ControllersPage {
            property bool topLevel: true
            property string topLevelName: "controllers"
        }
    }

    Component {
        id: settingsPage
        SettingsPage {
            property bool topLevel: true
            property string topLevelName: "settings"
        }
    }

    Component {
        id: nowPlayingPage
        NowPlayingPage {
            property bool topLevel: true
            property string topLevelName: "nowPlaying"
        }
    }

    Pane {
        id: drawer
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        width: 250
        background: Rectangle {
            color: "#101114"
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
                stackview.pop()
                // if (drawer.width === 250) {
                //     drawer.width = 48
                // } else {
                //     drawer.width = 250
                // }
            }

            anchors.top: parent.top
            anchors.right: parent.right
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 10
            }

            Text {
                text: "Firelight"
                opacity: parent.width > 48 ? 1 : 0
                color: "#dadada"
                font.pointSize: 12
                font.weight: Font.DemiBold
                font.family: Constants.regularFontFamily
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            Text {
                text: "alpha (0.4.0a)"
                opacity: parent.width > 48 ? 1 : 0
                color: "#dadada"
                font.pointSize: 8
                font.weight: Font.DemiBold
                font.family: Constants.regularFontFamily
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            Text {
                Layout.fillWidth: true
                Layout.preferredHeight: 12
            }
            NavMenuButton {
                id: homeNavButton
                KeyNavigation.down: libraryNavButton
                labelText: "Home"
                labelIcon: "\ue88a"
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 48
                enabled: false

                checked: stackview.topLevelName === "home"

                onToggled: function () {
                    stackview.push(thingy, {text: "Home"})
                }
            }
            NavMenuButton {
                id: modNavButton
                KeyNavigation.down: controllersNavButton
                labelText: "Market"
                labelIcon: "\uea12"
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 48

                checked: stackview.topLevelName === "mods"

                onToggled: function () {
                    stackview.push(modsPage)
                }
            }
            NavMenuButton {
                id: libraryNavButton
                KeyNavigation.down: modNavButton
                labelText: "My Library"
                labelIcon: "\uf53e"
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 48

                checked: stackview.topLevelName === "library"

                onToggled: function () {
                    stackview.push(libraryPage)
                }
            }
            // Rectangle {
            //     Layout.topMargin: 8
            //     Layout.bottomMargin: 8
            //     Layout.preferredWidth: parent.width
            //     Layout.preferredHeight: 1
            //     opacity: 0.3
            //     color: "#dadada"
            // }
            // NavMenuButton {
            //     id: nowPlayingNavButton
            //     KeyNavigation.down: settingsNavButton
            //     labelText: "Now Playing"
            //     labelIcon: "\ue037"
            //     Layout.preferredWidth: parent.width
            //     Layout.preferredHeight: 48
            //
            //     onToggled: function () {
            //         stackview.replace(nowPlayingPage)
            //     }
            // }
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
            NavMenuButton {
                id: controllersNavButton
                // KeyNavigation.down: nowPlayingNavButton
                labelText: "Controllers"
                labelIcon: "\uf135"
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 48

                checked: stackview.topLevelName === "controllers"

                onToggled: function () {
                    stackview.push(controllerPage)
                }
            }
            NavMenuButton {
                id: settingsNavButton
                labelText: "Settings"
                labelIcon: "\ue8b8"
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 48

                checked: stackview.topLevelName === "settings"

                onToggled: function () {
                    stackview.push(settingsPage)
                }
            }

            NavMenuButton {
                labelText: "Profile"
                labelIcon: "\ue853"
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 48


                checked: stackview.topLevelName === "nowPlaying"

                enabled: false

                onToggled: function () {
                    stackview.push(thingy, {text: "Profile"})
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

        ColumnLayout {
            anchors.fill: parent

            Pane {
                Layout.fillWidth: true
                Layout.preferredHeight: 48

                z: 2

                background: Item {
                }

                Button {
                    id: melol
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    // horizontalPadding: 12
                    // verticalPadding: 8

                    enabled: stackview.depth > 1

                    hoverEnabled: false

                    HoverHandler {
                        id: myHover
                        cursorShape: melol.enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
                    }

                    background: Rectangle {
                        color: enabled ? myHover.hovered ? "#4e535b" : "#3e434b" : "#3e434b"
                        radius: height / 2
                        // border.color: "#7d848c"
                    }

                    contentItem: Text {
                        text: "\ue5c4"
                        color: enabled ? "white" : "#7d848c"
                        font.pointSize: 11
                        font.family: Constants.symbolFontFamily
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        stackview.pop()
                    }
                }
            }

            StackView {
                id: stackview
                Layout.fillWidth: true
                Layout.fillHeight: true

                property string topLevelName: ""

                onCurrentItemChanged: {
                    if (currentItem) {
                        let top = stackview.find(function (item, index) {
                            return item.topLevel === true
                        })

                        stackview.topLevelName = top ? top.topLevelName : ""
                    }
                }

                initialItem: libraryPage

                pushEnter: Transition {
                    // PropertyAnimation {
                    //     property: "opacity"
                    //     from: 0
                    //     to: 1
                    //     duration: 200
                    // }
                }
                pushExit: Transition {
                    // PropertyAnimation {
                    //     property: "opacity"
                    //     from: 1
                    //     to: 0
                    //     duration: 200
                    // }
                }
                popEnter: Transition {
                    // PropertyAnimation {
                    //     property: "opacity"
                    //     from: 0
                    //     to: 1
                    //     duration: 200
                    // }
                }
                popExit: Transition {
                    // PropertyAnimation {
                    //     property: "opacity"
                    //     from: 1
                    //     to: 0
                    //     duration: 200
                    // }
                }
                replaceEnter: Transition {
                    // ParallelAnimation {
                    //     PropertyAnimation {
                    //         property: "opacity"
                    //         from: 0
                    //         to: 1
                    //         duration: 400
                    //     }
                    //     PropertyAnimation {
                    //         property: "x"
                    //         from: 20
                    //         to: 0
                    //         duration: 250
                    //     }
                    // }
                }
                replaceExit: Transition {
                }
            }

        }
    }
}
