import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts 1.0
import Firelight 1.0

FocusScope {
    id: root

    signal startGame(entryId: int)

    property bool showNowPlayingButton: false

    Keys.onEscapePressed: function (event) {
        if (root.StackView.status !== StackView.Active && !event.isAutoRepeat) {
            closeAppConfirmationDialog.open()
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
        id: libraryPage2
        LibraryPage2 {
            objectName: "Library Page 2"
            property bool topLevel: true
            property string topLevelName: "library"
            model: library_short_model

            onOpenDetails: function (id) {
                contentStack.push(gameDetailsPage)
            }

            Component.onCompleted: {
                startGame.connect(root.startGame)
            }

            Component {
                id: gameDetailsPage
                GameDetailsPage {
                    entryId: 115
                }
            }

            // onEntryClicked: function (id) {
            //     emulatorScreen.loadGame(id)
            // }
        }
    }


    Component {
        id: controllerPage
        ControllersPage {
            property bool topLevel: true
            property string topLevelName: "controllers"
        }
    }

    Pane {
        id: drawer
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        width: 200
        background: Rectangle {
            color: "#101114"
        }
        padding: 4
        focus: true

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
                text: "alpha (0.5.0a)"
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
                labelText: "Dashboard"
                labelIcon: "\ue871"
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 48
                enabled: false


                checked: contentStack.currentPageName === "home"
                // checked: stackview.topLevelName === "home"
            }
            NavMenuButton {
                id: modNavButton
                KeyNavigation.down: controllersNavButton
                labelText: "Mod Shop"
                labelIcon: "\uef48"
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 48
                enabled: false
                checked: contentStack.currentPageName === "mods"

                // checked: stackview.topLevelName === "mods"

                onToggled: function () {
                    // stackview.replace(null, modsPage)
                    contentStack.goTo(modsPage)
                }
            }
            NavMenuButton {
                id: libraryNavButton
                KeyNavigation.down: modNavButton
                labelText: "My Library"
                labelIcon: "\uf53e"
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 48
                focus: true

                // checked: stackview.topLevelName === "library"
                checked: contentStack.currentPageName === "library"

                onToggled: function () {
                    // stackview.replace(null, libraryPage)
                    contentStack.goTo(libraryPage2)
                }
            }
            NavMenuButton {
                id: controllersNavButton
                // KeyNavigation.down: nowPlayingNavButton
                labelText: "Controllers"
                labelIcon: "\uf135"
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 48

                // enabled: true

                // checked: stackview.topLevelName === "controllers"
                checked: contentStack.currentPageName === "controllers"

                onToggled: function () {
                    contentStack.goTo(controllerPage)
                    // stackview.replace(null, controllerPage)
                }
            }
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
            NavMenuButton {
                id: nowPlayingNavButton
                labelText: "Back to game"
                labelIcon: "\ue037"
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 48

                visible: root.showNowPlayingButton
                // visible: emulator.running

                checkable: false

                onClicked: function () {
                    root.StackView.view.pop()
                }
            }
            Rectangle {
                Layout.fillWidth: true
                Layout.topMargin: 8
                Layout.bottomMargin: 4
                Layout.preferredHeight: 1
                color: "#404143"
            }

            RowLayout {
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 48
                Layout.maximumHeight: 48
                spacing: 8

                Button {
                    background: Rectangle {
                        color: "transparent"
                        radius: 4
                    }
                    Layout.preferredHeight: 42
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                    checkable: false
                }

                Button {
                    id: me
                    background: Rectangle {
                        color: enabled ? (me.hovered ? "#404143" : "transparent") : "transparent"
                        radius: 4
                    }
                    Layout.preferredHeight: 42
                    Layout.preferredWidth: 42
                    Layout.alignment: Qt.AlignCenter

                    hoverEnabled: true

                    contentItem: Text {
                        text: "\ue8b8"
                        font.family: Constants.symbolFontFamily
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: 26
                        color: "#c3c3c3"
                    }

                    checkable: false

                    onClicked: {
                        Router.navigateTo("settings")
                    }
                }
            }


        }
    }

    StackView {
        id: contentStack
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.left: drawer.right
        KeyNavigation.left: drawer

        //
        objectName: "Home Content Stack View"
        // anchors.fill: parent

        property alias currentPageName: contentStack.topLevelName

        function goTo(page) {
            contentStack.replace(null, page)
        }

        property string topLevelName: ""

        onCurrentItemChanged: {
            if (currentItem) {
                let top = contentStack.find(function (item, index) {
                    return item.topLevel === true
                })

                contentStack.topLevelName = top ? top.topLevelName : ""
            }
        }

        // Pane {
        //     width: 48
        //     height: 48
        //
        //     z: 2
        //
        //     background: Item {
        //     }
        //
        //     Button {
        //         id: melol
        //         anchors.left: parent.left
        //         anchors.verticalCenter: parent.verticalCenter
        //         // horizontalPadding: 12
        //         // verticalPadding: 8
        //
        //         enabled: stackview.depth > 1
        //
        //         hoverEnabled: false
        //
        //         HoverHandler {
        //             id: myHover
        //             cursorShape: melol.enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
        //         }
        //
        //         background: Rectangle {
        //             color: enabled ? myHover.hovered ? "#4e535b" : "#3e434b" : "#3e434b"
        //             radius: height / 2
        //         }
        //
        //         contentItem: Text {
        //             text: "\ue5c4"
        //             color: enabled ? "white" : "#7d848c"
        //             font.pointSize: 11
        //             font.family: Constants.symbolFontFamily
        //             horizontalAlignment: Text.AlignHCenter
        //             verticalAlignment: Text.AlignVCenter
        //         }
        //
        //         onClicked: {
        //             stackview.pop()
        //         }
        //     }
        // }

        initialItem: libraryPage2

        pushEnter: Transition {
        }
        pushExit: Transition {
        }
        popEnter: Transition {
        }
        popExit: Transition {
        }
        replaceEnter: Transition {
        }
        replaceExit: Transition {
        }
    }

    // HomeContentPane {
    //     id: homeContentPane
    //     anchors.top: parent.top
    //     anchors.right: parent.right
    //     anchors.bottom: parent.bottom
    //     anchors.left: drawer.right
    //
    //     KeyNavigation.left: drawer
    // }


    FirelightDialog {
        id: closeAppConfirmationDialog
        text: "Are you sure you want to close Firelight?"

        onAccepted: {
            Qt.callLater(Qt.quit)
        }
    }
}