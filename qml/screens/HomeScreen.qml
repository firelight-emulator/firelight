import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts 1.0
import Firelight 1.0

FocusScope {
    id: root

    property bool showNowPlayingButton: false

    signal readyToStartGame()

    Keys.onEscapePressed: function (event) {
        if (root.StackView.status !== StackView.Active && !event.isAutoRepeat) {
            closeAppConfirmationDialog.open()
        }
    }

    Component {
        id: shopPage
        ShopLandingPage {
            property bool topLevel: true
            property string topLevelName: "shop"
            property string pageTitle: "Mod shop (coming soon!)"
            model: shop_item_model
        }
    }

    Component {
        id: libraryPage2
        LibraryPage2 {
            objectName: "Library Page 2"
            property bool topLevel: true
            property string topLevelName: "library"
            property string pageTitle: "Library"
            // model: library_short_model
            model: LibraryEntryModel

            onReadyToStartGame: {
                root.readyToStartGame()
            }

            onOpenDetails: function (id) {
                contentStack.push(gameDetailsPage)
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
            property string pageTitle: "Controllers"
        }
    }

    Popup {
        id: scannerPopup
        width: 180
        height: 60
        modal: false
        focus: false
        visible: LibraryScanner.scanning
        x: (parent.width - width) / 2
        y: parent.height - height - 20
        background: Rectangle {
            color: ColorPalette.neutral800
        }
        contentItem: Text {
            text: "Updating library..."
            color: "white"
            font.pixelSize: 16
            font.family: Constants.regularFontFamily
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

    }

    // Pane {
    //     id: drawer
    //     anchors.top: parent.top
    //     anchors.bottom: parent.bottom
    //     anchors.left: parent.left
    //     width: 60
    //     padding: 10
    //     background: Rectangle {
    //         color: "transparent"
    //     }
    //     // visible: false
    //     focus: true
    //
    //     ColumnLayout {
    //         anchors.fill: parent
    //         spacing: 12
    //
    //         // Item {
    //         //     Layout.fillWidth: true
    //         //     Layout.preferredHeight: 10
    //         // }
    //
    //         // Text {
    //         //     text: "Firelight"
    //         //     opacity: parent.width > 48 ? 1 : 0
    //         //     color: "#dadada"
    //         //     font.pointSize: 12
    //         //     font.weight: Font.DemiBold
    //         //     font.family: Constants.regularFontFamily
    //         //     Layout.fillWidth: true
    //         //     horizontalAlignment: Text.AlignHCenter
    //         // }
    //         //
    //         // Text {
    //         //     text: "alpha (0.5.0a)"
    //         //     opacity: parent.width > 48 ? 1 : 0
    //         //     color: "#dadada"
    //         //     font.pointSize: 8
    //         //     font.weight: Font.DemiBold
    //         //     font.family: Constants.regularFontFamily
    //         //     Layout.fillWidth: true
    //         //     horizontalAlignment: Text.AlignHCenter
    //         // }
    //         //
    //         // Text {
    //         //     Layout.fillWidth: true
    //         //     Layout.preferredHeight: 12
    //         // }
    //
    //         Button {
    //             Layout.fillWidth: true
    //             Layout.preferredHeight: width
    //
    //             hoverEnabled: true
    //
    //             background: Rectangle {
    //                 color: "white"
    //                 opacity: parent.hovered ? 0.1 : 0
    //                 radius: width / 2
    //             }
    //
    //             contentItem: Text {
    //                 text: "\ue5d2"
    //                 font.family: Constants.symbolFontFamily
    //                 font.pixelSize: 28
    //                 horizontalAlignment: Text.AlignHCenter
    //                 verticalAlignment: Text.AlignVCenter
    //                 color: "white"
    //             }
    //
    //             onClicked: {
    //                 drawer2.open()
    //             }
    //         }
    //
    //         Item {
    //             Layout.fillWidth: true
    //             Layout.fillHeight: true
    //         }
    //
    //         Button {
    //             Layout.fillWidth: true
    //             Layout.preferredHeight: width
    //
    //             z: 3
    //
    //             hoverEnabled: true
    //
    //             background: Rectangle {
    //                 color: "white"
    //                 opacity: parent.hovered ? 0.1 : 0
    //                 radius: width / 2
    //             }
    //
    //             contentItem: Text {
    //                 text: "\ue871"
    //                 font.family: Constants.symbolFontFamily
    //                 font.pixelSize: 28
    //                 opacity: parent.checked ? 1 : 0.5
    //                 horizontalAlignment: Text.AlignHCenter
    //                 verticalAlignment: Text.AlignVCenter
    //                 color: "white"
    //             }
    //
    //             checked: contentStack.currentPageName === "home"
    //         }
    //
    //         Button {
    //             Layout.fillWidth: true
    //             Layout.preferredHeight: width
    //
    //             z: 3
    //
    //             hoverEnabled: true
    //
    //             background: Rectangle {
    //                 color: "white"
    //                 opacity: parent.hovered ? 0.1 : 0
    //                 radius: width / 2
    //             }
    //
    //             contentItem: Text {
    //                 text: "\uef48"
    //                 font.family: Constants.symbolFontFamily
    //                 font.pixelSize: 28
    //                 // font.variableAxes: { "FILL": 1 }
    //                 opacity: parent.checked ? 1 : 0.5
    //                 horizontalAlignment: Text.AlignHCenter
    //                 verticalAlignment: Text.AlignVCenter
    //                 color: "white"
    //             }
    //
    //             checked: contentStack.currentItem === shopPage
    //         }
    //
    //         Button {
    //             Layout.fillWidth: true
    //             Layout.preferredHeight: width
    //
    //             z: 3
    //
    //             hoverEnabled: true
    //
    //             background: Rectangle {
    //                 color: "white"
    //                 opacity: parent.hovered ? 0.1 : 0
    //                 radius: width / 2
    //             }
    //
    //             contentItem: Text {
    //                 text: "\uf53e"
    //                 font.family: Constants.symbolFontFamily
    //                 font.pixelSize: 28
    //                 opacity: parent.checked ? 1 : 0.5
    //                 horizontalAlignment: Text.AlignHCenter
    //                 verticalAlignment: Text.AlignVCenter
    //                 color: "white"
    //             }
    //
    //             checked: contentStack.currentItem === libraryPage2
    //         }
    //
    //         Button {
    //             Layout.fillWidth: true
    //             Layout.preferredHeight: width
    //
    //             z: 3
    //
    //             hoverEnabled: true
    //
    //             background: Rectangle {
    //                 color: "white"
    //                 opacity: parent.hovered ? 0.1 : 0
    //                 radius: width / 2
    //             }
    //
    //             contentItem: Text {
    //                 text: "\uf135"
    //                 font.family: Constants.symbolFontFamily
    //                 font.pixelSize: 28
    //                 opacity: parent.checked ? 1 : 0.5
    //                 horizontalAlignment: Text.AlignHCenter
    //                 verticalAlignment: Text.AlignVCenter
    //                 color: "white"
    //             }
    //
    //             checked: contentStack.currentItem === controllerPage
    //         }
    //
    //         // NavMenuButton {
    //         //     id: homeNavButton
    //         //     KeyNavigation.down: libraryNavButton
    //         //     labelText: "Dashboard"
    //         //     labelIcon: "\ue871"
    //         //     Layout.preferredWidth: parent.width
    //         //     Layout.preferredHeight: 40
    //         //     enabled: false
    //         //
    //         //
    //         //     checked: contentStack.currentPageName === "home"
    //         //     // checked: stackview.topLevelName === "home"
    //         // }
    //         // NavMenuButton {
    //         //     id: modNavButton
    //         //     KeyNavigation.down: controllersNavButton
    //         //     labelText: "Mod Shop"
    //         //     labelIcon: "\uef48"
    //         //     Layout.preferredWidth: parent.width
    //         //     Layout.preferredHeight: 40
    //         //     // enabled: false
    //         //     checked: contentStack.currentPageName === "shop"
    //         //
    //         //     // checked: stackview.topLevelName === "mods"
    //         //
    //         //     onToggled: function () {
    //         //         // stackview.replace(null, modsPage)
    //         //         contentStack.goTo(shopPage)
    //         //     }
    //         // }
    //         // NavMenuButton {
    //         //     id: libraryNavButton
    //         //     KeyNavigation.down: modNavButton
    //         //     labelText: "My Library"
    //         //     labelIcon: "\uf53e"
    //         //     Layout.preferredWidth: parent.width
    //         //     Layout.preferredHeight: 40
    //         //     focus: true
    //         //
    //         //     // checked: stackview.topLevelName === "library"
    //         //     checked: contentStack.currentPageName === "library"
    //         //
    //         //     onToggled: function () {
    //         //         // stackview.replace(null, libraryPage)
    //         //         contentStack.goTo(libraryPage2)
    //         //     }
    //         // }
    //         // NavMenuButton {
    //         //     id: controllersNavButton
    //         //     // KeyNavigation.down: nowPlayingNavButton
    //         //     labelText: "Controllers"
    //         //     labelIcon: "\uf135"
    //         //     Layout.preferredWidth: parent.width
    //         //     Layout.preferredHeight: 40
    //         //
    //         //     // enabled: true
    //         //
    //         //     // checked: stackview.topLevelName === "controllers"
    //         //     checked: contentStack.currentPageName === "controllers"
    //         //
    //         //     onToggled: function () {
    //         //         contentStack.goTo(controllerPage)
    //         //         // stackview.replace(null, controllerPage)
    //         //     }
    //         // }
    //         Item {
    //             Layout.fillWidth: true
    //             Layout.fillHeight: true
    //         }
    //
    //         Item {
    //             Layout.fillWidth: true
    //             Layout.preferredHeight: width
    //         }
    //     }
    // }

    Pane {
        id: headerBar
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.left: parent.left
        height: 72
        padding: 16

        KeyNavigation.down: contentStack

        background: Item {
        }

        contentItem: RowLayout {
            spacing: 24

            FirelightButton {
                id: hamburger

                tooltipLabel: "Menu"
                focus: true
                flat: true

                Layout.fillHeight: true
                Layout.preferredWidth: height
                Layout.leftMargin: 12

                iconCode: "\ue5d2"

                onClicked: {
                    drawer2.open()
                }
            }

            Text {
                text: contentStack.currentItem.pageTitle
                color: ColorPalette.neutral100
                font.pixelSize: 24
                font.weight: Font.Normal
                font.family: Constants.regularFontFamily
                Layout.fillHeight: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }


            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
            }

            FirelightButton {
                id: searchButton

                enabled: false
                tooltipLabel: "Search"
                flat: true

                Layout.fillHeight: true
                Layout.preferredWidth: height
                KeyNavigation.left: hamburger

                iconCode: "\ue8b6"
            }

            FirelightButton {
                id: notificationButton

                enabled: false
                tooltipLabel: "Notifications"
                flat: true

                Layout.fillHeight: true
                Layout.preferredWidth: height
                KeyNavigation.left: searchButton

                iconCode: "\ue7f4"
            }

            FirelightButton {
                Layout.fillHeight: true
                Layout.preferredWidth: height
                tooltipLabel: "Settings"
                flat: true

                KeyNavigation.left: notificationButton

                iconCode: "\ue8b8"

                onClicked: {
                    Router.navigateTo("settings")
                }
            }

            // Button {
            //     Layout.preferredHeight: 36
            //     Layout.preferredWidth: 36
            //
            //     z: 3
            //
            //     hoverEnabled: true
            //
            //     background: Rectangle {
            //         color: "white"
            //         // opacity: parent.hovered ? 0.1 : 0
            //         radius: width / 2
            //     }
            //
            //     // contentItem: Text {
            //     //     text: "\ue8b8"
            //     //     font.family: Constants.symbolFontFamily
            //     //     font.pixelSize: 28
            //     //     horizontalAlignment: Text.AlignHCenter
            //     //     verticalAlignment: Text.AlignVCenter
            //     //     color: "white"
            //     // }
            //
            //     // onClicked: {
            //     //     Router.navigateTo("settings")
            //     // }
            // }

            Text {
                property bool drawColon: true
                Timer {
                    triggeredOnStart: true
                    interval: 1000
                    running: true
                    repeat: true
                    onTriggered: {
                        parent.text = new Date().toLocaleTimeString(Qt.locale("en_US"), Locale.ShortFormat)
                        if (!parent.drawColon) {
                            parent.text = parent.text.replace(":", " ")
                            parent.drawColon = true
                        } else {
                            parent.drawColon = false
                        }
                    }
                    // console.log("tick")
                }
                text: ""
                color: "white"
                font.pixelSize: 18
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
                Layout.rightMargin: 12
                font.weight: Font.Normal
                font.family: Constants.regularFontFamily
                Layout.preferredWidth: 85
                Layout.fillHeight: true
            }
        }
    }


    Popup {
        id: drawer2
        width: 280
        height: parent.height
        x: -width
        modal: true
        focus: true

        background: Rectangle {
            color: ColorPalette.neutral900
        }

        leftPadding: 40 + rightPadding

        contentItem: ColumnLayout {
            width: parent.width - 40
            spacing: 4

            FirelightButton {
                Layout.preferredWidth: 40
                Layout.preferredHeight: 40
                Layout.alignment: Qt.AlignRight
                KeyNavigation.down: libraryButton
                flat: true

                iconCode: "\ue5cd"

                onClicked: {
                    drawer2.close()
                }
            }

            FirelightMenuItem {
                id: libraryButton
                focus: contentStack.currentItem.topLevelName === "library"
                labelText: "Library"
                Layout.fillWidth: true
                property bool showGlobalCursor: true

                KeyNavigation.down: exploreButton
                // Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 42
                checkable: false
                // alignRight: true

                onClicked: {
                    contentStack.goTo(libraryPage2)
                    drawer2.close()
                }
            }

            FirelightMenuItem {
                id: exploreButton
                focus: contentStack.currentItem.topLevelName === "shop"
                labelText: "Mod shop"
                Layout.fillWidth: true
                property bool showGlobalCursor: true

                KeyNavigation.down: controllerButton
                // Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 42
                checkable: false
                // alignRight: true

                onClicked: {
                    contentStack.goTo(shopPage)
                    drawer2.close()
                }
            }

            FirelightMenuItem {
                id: controllerButton
                focus: contentStack.currentItem.topLevelName === "controllers"
                labelText: "Controllers"
                Layout.fillWidth: true
                property bool showGlobalCursor: true

                // KeyNavigation.down: quitButton

                // Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 42
                checkable: false

                onClicked: {
                    contentStack.goTo(controllerPage)
                    drawer2.close()
                }
                // alignRight: true
            }

            // Button {
            //     Layout.fillWidth: true
            //     Layout.preferredHeight: 50
            //
            //     z: 3
            //
            //     hoverEnabled: true
            //
            //     background: Rectangle {
            //         color: "white"
            //         opacity: parent.hovered ? 0.1 : 0
            //     }
            //
            //     contentItem: Text {
            //         text: "Library"
            //         font.family: Constants.regularFontFamily
            //         font.pixelSize: 16
            //         // opacity: parent.checked ? 1 : 0.5
            //         horizontalAlignment: Text.AlignLeft
            //         verticalAlignment: Text.AlignVCenter
            //         color: "white"
            //     }
            //
            //     checked: contentStack.currentPageName === "home"
            // }

            // Button {
            //     Layout.fillWidth: true
            //     Layout.preferredHeight: 50
            //
            //     z: 3
            //
            //     hoverEnabled: true
            //
            //     background: Rectangle {
            //         color: "white"
            //         opacity: parent.hovered ? 0.1 : 0
            //     }
            //     contentItem: Text {
            //         text: "Explore"
            //         font.family: Constants.regularFontFamily
            //         font.pixelSize: 16
            //         // opacity: parent.checked ? 1 : 0.5
            //         // font.variableAxes: { "FILL": 1 }
            //         horizontalAlignment: Text.AlignLeft
            //         verticalAlignment: Text.AlignVCenter
            //         color: "white"
            //     }
            //
            //     checked: contentStack.currentItem === shopPage
            // }
            //
            // Button {
            //     Layout.fillWidth: true
            //     Layout.preferredHeight: 50
            //
            //     z: 3
            //
            //     hoverEnabled: true
            //
            //     background: Rectangle {
            //         color: "white"
            //         opacity: parent.hovered ? 0.1 : 0
            //     }
            //
            //     contentItem: Text {
            //         text: "Controllers"
            //         font.family: Constants.regularFontFamily
            //         font.pixelSize: 16
            //         // opacity: parent.checked ? 1 : 0.5
            //         horizontalAlignment: Text.AlignLeft
            //         verticalAlignment: Text.AlignVCenter
            //         color: "white"
            //     }
            //
            //     checked: contentStack.currentItem === libraryPage2
            // }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            Text {
                Layout.fillWidth: true
                text: "Firelight is made with ❤️\nby BiscuitCakes"
                color: ColorPalette.neutral300
                font.pixelSize: 12
                font.weight: Font.Normal
                font.family: Constants.regularFontFamily
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            // Button {
            //     id: quitButton
            //     Layout.fillWidth: true
            //     Layout.preferredHeight: 42
            //     property bool showGlobalCursor: true
            //
            //     background: Rectangle {
            //         color: ColorPalette.neutral700
            //         radius: height / 2
            //     }
            //
            //     contentItem: Text {
            //         text: "Close Firelight"
            //         font.family: Constants.regularFontFamily
            //         font.pixelSize: 14
            //         // opacity: parent.checked ? 1 : 0.5
            //         horizontalAlignment: Text.AlignHCenter
            //         verticalAlignment: Text.AlignVCenter
            //         color: "white"
            //     }
            // }
        }

        Overlay.modal: Rectangle {
            color: "black"
            anchors.fill: parent
            opacity: 0.6

            Behavior on opacity {
                NumberAnimation {
                    duration: 200
                }
            }
        }

        enter: Transition {
            NumberAnimation {
                property: "x";
                to: -40
                easing.type: Easing.OutBack
                easing.overshoot: 0.6
                duration: 400
            }
        }
        exit: Transition {
            NumberAnimation {
                property: "x";
                to: -width
                easing.type: Easing.InBack
                easing.overshoot: 0.6
                duration: 400
            }
        }

        // enter: Transition {
        //     NumberAnimation {
        //         properties: "x"
        //         easing.type: Easing.OutBack
        //         duration: 200
        //     }
        // }
        //
        // exit: Transition {
        //     NumberAnimation {
        //         properties: "x"
        //         easing.type: Easing.InBack
        //         duration: 200
        //     }
        // }
    }


    StackView {
        id: contentStack
        anchors.top: headerBar.bottom
        // anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.leftMargin: 40
        anchors.rightMargin: 40
        KeyNavigation.left: hamburger

        background: Item {
        }

        Keys.onEscapePressed: function (event) {
            drawer2.open()
        }

        focus: true

        //
        objectName: "Home Content Stack View"
        // anchors.fill: parent

        property alias currentPageName: contentStack.topLevelName

        function goTo(page) {
            contentStack.replace(null, page)
            forceActiveFocus()
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