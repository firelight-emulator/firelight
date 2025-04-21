import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQml.Models
import QtQuick.Layouts 1.0
import QtNetwork
import QtMultimedia
import QtQuick.Effects
import Firelight 1.0

ApplicationWindow {
    id: window
    objectName: "Application Window"

    width: 1920
    height: 1080

    // flags: Qt.FramelessWindowHint

    visible: true
    visibility: GeneralSettings.fullscreen ? Window.FullScreen : Window.Windowed

    title: qsTr("Firelight")

    property bool blur: false

    background: FLUserBackground {
        blur: window.blur
        defaultColor: ColorPalette.neutral1000
        usingCustomBackground: AppearanceSettings.usingCustomBackground
        backgroundFile: AppearanceSettings.backgroundFile
    }

    SplitView {
        id: splitView
        anchors.fill: parent
        orientation: Qt.Horizontal

        // background: Item {}

        focus: true

        handle: Rectangle {
            id: handleDelegate
            color: handleHover.hovered ? "#8e8e8e" : "#4B4B4B"
            implicitWidth: 1

            Behavior on color {
                ColorAnimation {
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
            }

            HoverHandler {
                id: handleHover
            }

            containmentMask: Item {
                x: (handleDelegate.width - width) / 2
                width: 16
                height: splitView.height
            }
        }

        Pane {
            id: navbar
            padding: 16

            SplitView.minimumWidth: 74
            SplitView.maximumWidth: 260
            focus: true

            background: Item {
                MultiEffect {
                    source: ShaderEffectSource {
                        width: navbar.width
                        height: navbar.height
                        sourceItem: window.background
                        sourceRect: Qt.rect(navbar.x, navbar.y, navbar.width, navbar.height)
                    }

                    anchors.fill: parent
                    blurEnabled: true
                    blurMultiplier: 1.0
                    blurMax: 32
                    blur: 1.0
                }

                Rectangle {
                    color: "#1D1D1D"
                    opacity: 0.8
                    anchors.fill: parent
                }
            }

            contentItem: ColumnLayout {
                spacing: 42

                RowLayout {
                    Layout.leftMargin: 7
                    Layout.maximumHeight: 36
                    Layout.minimumHeight: 36
                    Layout.fillWidth: true
                    Image {
                        source: "qrc:/images/firelight-logo"
                        Layout.preferredHeight: parent.height
                        sourceSize.height: parent.height
                        mipmap: true
                    }
                    // Text {
                    //     color: "white"
                    //     padding: 0
                    //     opacity: parent.width > 140 ? 1 : 0
                    //     text: "Firelight"
                    //     Layout.preferredWidth: opacity > 0 ?
                    //     font.pixelSize: 20
                    //     font.weight: Font.Bold
                    //     font.family: Constants.regularFontFamily
                    //     horizontalAlignment: Text.AlignLeft
                    //     verticalAlignment: Text.AlignVCenter

                    //     Behavior on opacity {
                    //         NumberAnimation {
                    //             duration: 100
                    //             easing.type: Easing.InOutQuad
                    //         }
                    //     }
                    // }
                }
                // RowLayout {
                //     Layout.alignment: Qt.AlignCenter
                //     Layout.minimumHeight: 32
                //     Layout.maximumHeight: 32
                //     spacing: 12
                //     Image {
                //         source: "qrc:/images/firelight-logo"
                //         Layout.preferredHeight: parent.height
                //         sourceSize.height: parent.height
                //         mipmap: true
                //     }
                //     Text {
                //         color: "white"
                //         padding: 0
                //         Layout.preferredHeight: parent.height
                //         text: "Firelight"
                //         font.pixelSize: 24
                //         font.weight: Font.Bold
                //         font.family: Constants.regularFontFamily
                //         horizontalAlignment: Text.AlignLeft
                //         verticalAlignment: Text.AlignVCenter

                //     }
                // }
                FLNavSection {
                    Layout.fillWidth: true

                    focus: true

                    FLNavItem {
                        label: "Library"
                        iconName: "book"
                        checked: Router.currentRoute.startsWith("/library")
                        onClicked: {
                            Router.navigateTo("/library")
                        }
                    }

                    FLNavItem {
                        label: "Mod Shop"
                        iconName: "shopping-bag"
                        checked: Router.currentRoute.startsWith("/shop")
                        onClicked: {
                            Router.navigateTo("/shop")
                        }
                    }

                    // FLNavItem {
                    //     label: "Gallery"
                    //     iconName: "photo-library"
                    //     checked: Router.currentRoute === "/gallery"
                    //     onClicked: {
                    //         Router.navigateTo("/gallery")
                    //     }
                    // }

                    // FLNavItem {
                    //     label: "Controllers"
                    //     iconName: "controller"
                    //     checked: Router.currentRoute === "/controllers"
                    //     onClicked: {
                    //         Router.navigateTo("/controllers")
                    //     }
                    // }

                    FLNavItem {
                        label: "Settings"
                        iconName: "settings"
                        checked: Router.currentRoute.startsWith("/settings")
                        onClicked: {
                            Router.navigateTo("/settings")
                        }
                    }

                    // FLNavItem {
                    //     label: "Favorites"
                    //     iconName: "kid-star"
                    // }

                    // FLNavItem {
                    //     label: "Recent"
                    //     iconName: "history"
                    // }

                    // FLNavItem {
                    //     label: "Newest"
                    //     iconName: "add"
                    // }
                }
                // FLNavSection {
                //     Layout.fillWidth: true
                //     title: "LIBRARY"

                //     focus: true

                //     FLNavItem {
                //         label: "All games"
                //         iconName: "book"
                //         checked: Router.currentRoute === "/library"
                //         onClicked: {
                //             Router.navigateTo("/library")
                //         }
                //     }

                //     // FLNavItem {
                //     //     label: "Favorites"
                //     //     iconName: "kid-star"
                //     // }

                //     // FLNavItem {
                //     //     label: "Recent"
                //     //     iconName: "history"
                //     // }

                //     // FLNavItem {
                //     //     label: "Newest"
                //     //     iconName: "add"
                //     // }
                // }
                // FLNavSection {
                //     title: "MOD SHOP"
                //     Layout.fillWidth: true

                //     FLNavItem {
                //         label: "Browse"
                //         iconName: "browse"
                //         checked: Router.currentRoute === "/shop"
                //         onClicked: {
                //             Router.navigateTo("/shop")
                //         }
                //     }

                //     // FLNavItem {
                //     //     label: "Featured"
                //     //     iconName: "bookmark-star"
                //     // }

                //     // FLNavItem {
                //     //     label: "New Arrivals"
                //     //     iconName: "star-shine"
                //     // }
                // }
                // FLNavSection {
                //     title: "OTHER"
                //     Layout.fillWidth: true

                //     FLNavItem {
                //         label: "Settings"
                //         iconName: "settings"
                //         checked: Router.currentRoute === "/settings"
                //         onClicked: {
                //             Router.navigateTo("/settings")
                //         }
                //     }
                // }
                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }
            }



        }

        Pane {
            padding: 16
            background: Item {}
            contentItem: ColumnLayout {
                spacing: 16
                RowLayout {
                    Layout.fillWidth: true
                    Layout.maximumHeight: 42
                    Layout.minimumHeight: 42

                    spacing: 12

                    Button {
                        background: Rectangle {
                            color: "white"
                            opacity: parent.pressed ? 0.16 : 0.1
                            radius: 4
                            visible: theHoverHandler.hovered && parent.enabled
                        }
                        HoverHandler {
                            id: theHoverHandler
                            cursorShape: Qt.PointingHandCursor
                        }
                        padding: 4
                        Layout.fillHeight: true
                        Layout.preferredWidth: parent.height
                        contentItem: FLIcon {
                            icon: "arrow-back"
                            size: height
                            color: parent.enabled ? "white" : "#727272"
                        }

                        enabled: Router.historySize > 1

                        onClicked: {
                            Router.goBack()
                            // contentStack.popCurrentItem()
                        }
                    }

                    // Button {
                    //     background: Rectangle {
                    //         color: "white"
                    //         opacity: 0.1
                    //         radius: 4
                    //         visible: theHoverHandler2.hovered
                    //     }
                    //     HoverHandler {
                    //         id: theHoverHandler2
                    //         cursorShape: Qt.PointingHandCursor
                    //     }
                    //     padding: 4
                    //     Layout.fillHeight: true
                    //     Layout.preferredWidth: parent.height
                    //     contentItem: FLIcon {
                    //         icon: "arrow-forward"
                    //         size: height
                    //         color: parent.enabled ? "white" : "#727272"
                    //     }
                    //
                    //     enabled: false
                    // }

                    
                    // Text {
                    //     color: "white"
                    //     font.family: Constants.regularFontFamily
                    //     text: "Library"
                    //     font.pixelSize: 28
                    //     font.weight: Font.Bold
                    //     Layout.fillHeight: true
                    //     verticalAlignment: Text.AlignVCenter
                    // }

                    // Item {
                    //     Layout.fillWidth: true
                    //     Layout.fillHeight: true
                    //     Layout.horizontalStretchFactor: 1
                    // }

                    // Pane {
                    //     Layout.fillHeight: true
                    //     Layout.preferredWidth: 460
                    //     Layout.alignment: Qt.AlignHCenter
                    //     padding: 8
                    //     horizontalPadding: 10
                    //     background: Rectangle {
                    //         color: "#1D1D1D"
                    //         opacity: 0.8
                    //         radius: 8
                    //         border.color: "#4B4B4B"
                    //     }

                    //     contentItem: RowLayout {
                    //         TextInput {
                    //             id: searchTextInput
                    //             Layout.fillWidth: true
                    //             Layout.fillHeight: true
                    //             color: "white"
                    //             font.pixelSize: 15
                    //             font.family: Constants.regularFontFamily
                    //             horizontalAlignment: Text.AlignLeft
                    //             verticalAlignment: Text.AlignVCenter

                    //             HoverHandler {
                    //                 cursorShape: Qt.IBeamCursor
                    //             }

                    //             Text {
                    //                 text: "Search"
                    //                 anchors.fill: parent
                    //                 color: "#727272"
                    //                 font.pixelSize: 15
                    //                 font.family: Constants.regularFontFamily
                    //                 horizontalAlignment: Text.AlignLeft
                    //                 verticalAlignment: Text.AlignVCenter
                    //                 visible: (parent.length === 0) && !parent.activeFocus
                    //             }

                    //             Rectangle {
                    //                 anchors.top: parent.bottom
                    //                 width: parent.width
                    //                 height: 300
                    //                 color: "#1D1D1D"
                    //                 visible: searchTextInput.length !== 0
                    //             }
                    //         }
                    //         FLIcon {
                    //             Layout.fillHeight: true
                    //             icon: "search"
                    //             size: height
                    //             color: "#7C7C7C"

                    //             HoverHandler {
                    //                 cursorShape: Qt.PointingHandCursor
                    //             }
                    //         }
                    //     }
                    // }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.horizontalStretchFactor: 1
                    }
                }

                StackView {
                    id: contentStack
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Connections {
                        target: Router

                        function onRouteChanged(route) {
                            if (route === "/shop") {
                                contentStack.clear()
                                contentStack.pushItem(shopPage, {}, StackView.PushTransition)
                            } else if (route === "/library") {
                                contentStack.clear()
                                contentStack.pushItem(allGamesPage, {}, StackView.PushTransition)
                            } else if (route.startsWith("/library/entries/")) {
                                let things = route.split("/")
                                if (things.length === 4) {
                                    console.log("THREE: " + things[3])
                                    contentStack.pushItem(gameDetailsPage, {entryId: things[3]}, StackView.PushTransition)
                                }
                                console.log(things)
                            } else if (route.startsWith("/shop/mods/")) {
                                let things = route.split("/")
                                console.log(things)
                            } else if (route === "/settings") {
                                contentStack.push(settingsScreen)
                            } else {
                                contentStack.push(lol)
                            }
                        }
                    }

                    Component {
                        id: lol
                        Text {
                            text: "bruh"
                            color: "white"
                        }
                    }

                    pushEnter: Transition {}
                    pushExit: Transition {}
                    popEnter: Transition {}
                    popExit: Transition {}
                    replaceEnter: Transition {}
                    replaceExit: Transition {}
                }
            }
        }
    }

    FLEmulationPlayer {
        id: emulator
        anchors.fill: parent
        visible: false

        onUnloaded: {
            emulator.visible = false
        }
    }

    // Component {
    //     id: emuPage
    //     NewEmulatorPage {
    //         onRewindPointsReady: function(points) {
    //             screenStack.pushItem(rewindMenu, {
    //                 model: points,
    //                 aspectRatio: emulatorLoader.item.videoAspectRatio
    //             }, StackView.Immediate)
    //         }
    //     }
    // }
    //
    // Loader {
    //     id: emulatorLoader
    //     property var gameName
    //     property var entryId
    //     property var contentHash
    //     property bool shouldDeactivate: false
    //
    //     visible: false
    //
    //     anchors.fill: parent
    //
    //     Keys.onEscapePressed: {
    //         console.log("pressed escape")
    //     }
    //
    //     active: false
    //     sourceComponent: emuPage
    //
    //     property bool paused: !emulatorLoader.activeFocus || (emulatorLoader.item ? !emulatorLoader.item.activeFocus : false)
    //
    //     function stopGame() {
    //         shouldDeactivate = true
    //         if (emulatorLoader.StackView.status === StackView.Active) {
    //             emulatorLoader.StackView.view.popCurrentItem()
    //         }
    //     }
    //
    //     onPausedChanged: function () {
    //         if (emulatorLoader.item != null) {
    //             emulatorLoader.item.paused = emulatorLoader.paused
    //         }
    //     }
    //
    //     // onActiveChanged: {
    //     //     if (!active && emulatorLoader.StackView.status === StackView.Active) {
    //     //         emulatorLoader.StackView.view.popCurrentItem()
    //     //     }
    //     // }
    //
    //     StackView.onDeactivated: {
    //         if (shouldDeactivate) {
    //             active = false
    //             shouldDeactivate = false
    //         }
    //     }
    //
    //     StackView.onActivating: {
    //         // setSource(emuPage, {
    //         //     gameData: emulatorLoader.gameData,
    //         //     saveData: emulatorLoader.saveData,
    //         //     corePath: emulatorLoader.corePath,
    //         //     contentHash: emulatorLoader.contentHash,
    //         //     saveSlotNumber: emulatorLoader.saveSlotNumber,
    //         //     platformId: emulatorLoader.platformId,
    //         //     contentPath: emulatorLoader.contentPath
    //         // })
    //         // active = true
    //     }
    //
    //     onLoaded: function () {
    //         const emu = (emulatorLoader.item as NewEmulatorPage)
    //
    //         emu.pausedChanged.connect(function( ){
    //             console.log("game started")
    //         })
    //
    //         emu.closing.connect(function() {
    //             emulatorLoader.visible = false
    //             emulatorLoader.active = false
    //         })
    //
    //         overlay.opacity = 0
    //
    //         emu.loadGame(emulatorLoader.entryId)
    //         emu.forceActiveFocus()
    //         // emulatorLoader.item.startGame()
    //         // emulatorLoader.item.paused = emulatorLoader.paused
    //         //
    //         // emulatorLoader.gameName = emulatorLoader.item.gameName
    //         // emulatorLoader.contentHash = emulatorLoader.item.contentHash
    //         // emulatorLoader.item.startGame(gameData, saveData, corePath, contentHash, saveSlotNumber, platformId, contentPath)
    //         // emulatorLoader.item.paused = emulatorLoader.paused
    //     }
    // }

    Rectangle {
        id: overlay
        color: "black"
        anchors.fill: parent
        opacity: 0
    }

    SequentialAnimation {
        id: startGameAnimation

        property var entryId: -1
        NumberAnimation {
            target: overlay
            property: "opacity"
            from: 0
            to: 1
            duration: 400
            easing.type: Easing.InOutQuad
        }

        ScriptAction {
            script: {
                emulator.load(startGameAnimation.entryId)
                emulator.visible = true
                overlay.opacity = 0
            }
        }
    }

    // Rectangle {
    //     id: overlay
    //     color: "green"
    //     visible: true
    //     anchors.bottom: parent.bottom
    //     width: parent.width
    //     height: 100

    //     Behavior on height {
    //         NumberAnimation {
    //             property: "height"
    //             duration: 200
    //             easing.type: Easing.InOutQuad
    //         }
    //     }

    //     TapHandler {
    //         onTapped: {
    //             if (overlay.height === 100) {
    //                 overlay.height = overlay.parent.height
    //             } else {
    //                 overlay.height = 100
    //             }
    //         }
    //     }
    // }

    FLFocusHighlight {
        target: window.activeFocusItem
        usingMouse: InputMethodManager.usingMouse
    }

    Component {
        id: allGamesPage
        FocusScope {
            // ListView {
            //     id: gameNavList
            //     anchors.left: parent.left
            //     anchors.top: parent.top
            //     anchors.bottom: parent.bottom
            //     width: 250

            //     property int lastIndex: 0
            //     property bool movingDown: true
            //     // KeyNavigation.right: stack
            //     // anchors.bottom: parent.bottom
            //     // anchors.left: parent.left
            //     // anchors.top: parent.top
            //     currentIndex: 0
            //     focus: true
            //     interactive: false
            //     keyNavigationEnabled: true
            //     model: [
            //         "All games",
            //         "Recently played",
            //         "Favorites",
            //         "Newly added"
            //     ]

            //     delegate: FirelightMenuItem {
            //         required property var index
            //         required property var modelData
            //         property bool showGlobalCursor: true

            //         checked: ListView.isCurrentItem
            //         focus: ListView.isCurrentItem
            //         height: 50
            //         labelText: modelData
            //         width: ListView.view.width

            //         onToggled: {
            //             if (checked) {
            //                 ListView.view.currentIndex = index;
            //             }
            //         }
            //     }
            // }

            Text {
                id: titleText
                color: "white"
                text: "Library"
                font.pixelSize: 30
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
                verticalAlignment: Qt.AlignVCenter
            }

            ListView {
                id: theList
                // anchors.left: gameNavList.right
                anchors.top: titleText.bottom
                anchors.topMargin: 16
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                // anchors.fill: parent
                highlightMoveDuration: 80
                highlightMoveVelocity: -1
                highlightRangeMode: ListView.ApplyRange
                preferredHighlightBegin: 64
                preferredHighlightEnd: height - 64
                model: LibraryEntryModel

                clip: true
                // keyNavigationEnabled: false

                // Keys.onPressed: function(event) {
                //     // theList.highlightRangeMode = ListView.ApplyRange
                //     if (event.key === Qt.Key_Up) {
                //         if (theList.currentIndex === 0 && !event.isAutoRepeat) {
                //             theList.currentIndex = theList.count - 1
                //         } else {
                //             theList.decrementCurrentIndex()
                //         }
                //     } else if (event.key === Qt.Key_Down) {
                //         if (theList.currentIndex === theList.count - 1 && !event.isAutoRepeat) {
                //             theList.currentIndex = 0
                //         } else {
                //             theList.incrementCurrentIndex()
                //         }
                //     }
                // }

                property var icons: [
                    "https://cdn2.steamgriddb.com/thumb/5554ae4bc3d7ecf085bdc1f8b0a29f39.jpg",
                    "https://cdn2.steamgriddb.com/thumb/988fff1d881114ab5a56f0c0c69e0cfe.jpg",
                    "https://cdn2.steamgriddb.com/thumb/2fe0ac4f5a258e637671444f00d1835f.jpg",
                    "https://cdn2.steamgriddb.com/thumb/08ba346be1fd6a4a481429ba2cd48e6b.jpg",
                    "https://cdn2.steamgriddb.com/thumb/3c7a5ad35fa82f1d37d6296c21a9e52c.jpg",
                    "https://cdn2.steamgriddb.com/thumb/630b919fef718760f557be12e1cb6b4c.jpg",
                    "https://cdn2.steamgriddb.com/thumb/4bf486c3a04b32aa9dec6c542d7011ac.jpg",
                    "https://cdn2.steamgriddb.com/thumb/fdf301019dd996435263a651bd60d739.jpg",
                    "https://cdn2.steamgriddb.com/thumb/1e75612dbe09ed687c51498db305d2ed.jpg",
                    "https://cdn2.steamgriddb.com/thumb/1df5456e9760c85c7b649fd7961a9f66.jpg",
                    "https://cdn2.steamgriddb.com/thumb/e5494d23b18b5c822bcaa9fe8fd077bf.jpg",
                    "https://cdn2.steamgriddb.com/thumb/7e4c8f455b0699609758957b7c4e2f6f.jpg",
                    "https://cdn2.steamgriddb.com/thumb/7e4c8f455b0699609758957b7c4e2f6f.jpg",
                    "https://cdn2.steamgriddb.com/thumb/83754cee0fe637d0dcc1de38824a4e6d.jpg",
                    "https://cdn2.steamgriddb.com/thumb/b32d70b3e23ee6ee3eaa85623213eae3.jpg",
                    "https://cdn2.steamgriddb.com/thumb/cf68791aa992ebbe8c0ea1ac0d887e90.jpg",
                    "https://cdn2.steamgriddb.com/thumb/cedd27aa368c97ff84de86fb2b2b12c0.jpg",
                    "https://cdn2.steamgriddb.com/thumb/425d383ef78226dfbc600a00f2aa9b3b.png",
                    "https://cdn2.steamgriddb.com/thumb/372529e17424eb989b7aaf97716c2d11.jpg",
                    "https://cdn2.steamgriddb.com/thumb/425d383ef78226dfbc600a00f2aa9b3b.png",
                    "https://cdn2.steamgriddb.com/thumb/630b919fef718760f557be12e1cb6b4c.jpg",
                    "https://cdn2.steamgriddb.com/thumb/280bb2c98809304e66151313503f14fb.jpg",
                    "https://cdn2.steamgriddb.com/thumb/c52cc42f66f3aadad1b1f9170fbbcf4e.jpg",
                    "https://cdn2.steamgriddb.com/thumb/c179f2dd50f1440ed0a755468950dc66.jpg",
                    "https://cdn2.steamgriddb.com/thumb/a0b4b91a4d8b7da56ab2711853f7b588.jpg",
                    "https://cdn2.steamgriddb.com/thumb/a0b4b91a4d8b7da56ab2711853f7b588.jpg"
                ]

                Timer {
                    id: wheelTimer

                    interval: 500
                    repeat: false
                }

                WheelHandler {
                    onWheel: function(event) {
                        if (wheelTimer.running) {
                            if (theList.currentIndex === 0 && event.angleDelta.y >= 0) {
                                wheelTimer.restart()
                                return
                            } else if (theList.currentIndex === theList.count - 1 && event.angleDelta.y < 0) {
                                wheelTimer.restart()
                                return
                            }
                        }

                        if (event.angleDelta.y >= 0) {
                            // theList.highlightRangeMode = ListView.ApplyRange
                            theList.decrementCurrentIndex()
                        } else {
                            // theList.highlightRangeMode = ListView.ApplyRange
                            theList.incrementCurrentIndex()
                        }

                        wheelTimer.restart()
                    }
                }

                delegate: Button {
                    id: tttt
                    required property var model
                    required property var index
                    width: ListView.view.width

                    RightClickMenu {
                        id: rightClick

                        RightClickMenuItem {
                            text: "Play"
                            // externalLink: true
                            // onTriggered: {
                            //     Qt.openUrlExternally("https://retroachievements.org/achievement/" + row.model.achievement_id)
                            // }
                        }

                        RightClickMenuItem {
                            text: "View details"
                            onTriggered: {
                                Router.navigateTo("/library/entries/" + tttt.model.entryId)
                            }
                        }
                        
                    }


                    TapHandler {
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        onDoubleTapped: {
                            startGameAnimation.entryId = model.entryId
                            startGameAnimation.start()
                            console.log(tttt.model.entryId + " double tapped")
                        }

                        onSingleTapped: function(event, button) {
                            if (button === 2) {
                                rightClick.popupNormal()
                            }
                            if (tttt.ListView.view.currentIndex !== tttt.index) {
                                // theList.highlightRangeMode = ListView.NoHighlightRange
                                tttt.ListView.view.currentIndex = tttt.index
                            }
                            console.log(tttt.model.entryId + " tapped with " + button)
                        }

                        onLongPressed: {
                            console.log(tttt.model.entryId + " long pressed")
                        }
                    }

                    HoverHandler {
                        cursorShape: Qt.PointingHandCursor
                    }

                    height: 64
                    padding: 6
                    horizontalPadding: 12

                    background: Rectangle {
                        color: "white"
                        radius: 2
                        visible: tttt.ListView.isCurrentItem
                    }

                    contentItem: RowLayout {
                        property real random: Math.random() * 50
                        spacing: 16
                        // FLIcon {
                        //     icon: "star"
                        //     color: "grey"
                        //     Layout.preferredHeight: 32
                        //     Layout.preferredWidth: 32
                        //     Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                        //     size: height
                        // }
                        // Image {
                        //     source: tttt.index < theList.icons.length ? theList.icons[tttt.index] : "https://cdn2.steamgriddb.com/thumb/2c323abe873b4f9fa8a72f45785df5f0.jpg"
                        //     Layout.preferredHeight: parent.height
                        //     Layout.preferredWidth: parent.height
                        //     Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                        //     sourceSize.width: parent.height
                        //     sourceSize.height: parent.height
                        //     smooth: false
                        //     fillMode: Image.PreserveAspectFit
                        // }
                        Text {
                            text: tttt.model.displayName
                            font.pixelSize: 17
                            font.weight: Font.DemiBold
                            font.family: Constants.regularFontFamily
                            color: tttt.ListView.isCurrentItem ? "black" : "white"
                            Layout.fillHeight: true
                            Layout.preferredWidth: 600
                            elide: Text.ElideRight
                            maximumLineCount: 1
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }
                        FLIcon {
                            icon: platform_model.getPlatformIconName(tttt.model.platformId)
                            color: tttt.ListView.isCurrentItem ? "#2A2A2A" : "#d3d3d3"
                            Layout.preferredHeight: 40
                            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                            size: height
                        }
                        Text {
                            text: platform_model.getPlatformDisplayName(tttt.model.platformId)
                            font.pixelSize: 17
                            font.weight: Font.Medium
                            font.family: Constants.regularFontFamily
                            color: tttt.ListView.isCurrentItem ? "#2A2A2A" : "#d3d3d3"
                            Layout.fillHeight: true
                            Layout.preferredWidth: 300
                            elide: Text.ElideRight
                            maximumLineCount: 1
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }
                        Item {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }
        
    }

    Component {
        id: libraryPage2
        LibraryPage2 {
            id: libPage
            objectName: "Library Page 2"
            property bool topLevel: true
            property string topLevelName: "library"
            property string pageTitle: "Library"
            // model: library_short_model
            model: LibraryEntryModel


            // onEntryClicked: function (id) {
            //     emulatorScreen.loadGame(id)
            // }
        }
    }

    Component {
        id: gameDetailsPage
        FLGameDetailsPanel {

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
        id: shopItemPage
        ShopItemPage {
        }
    }

    Component {
        id: settingsScreen

        SettingsScreen {
        }
    }
}
