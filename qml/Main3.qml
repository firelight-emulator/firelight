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

    onActiveFocusItemChanged: {
        console.log("Active focus item changed to", window.activeFocusItem)
    }

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

        focus: true

        handle: Rectangle{
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

            SplitView.minimumWidth: 230
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
                spacing: 36
                Row {
                    Layout.alignment: Qt.AlignCenter
                    padding: 8
                    Layout.minimumHeight: 32
                    Layout.maximumHeight: 32
                    spacing: 12
                    Image {
                        source: "qrc:/images/firelight-logo"
                        height: parent.height
                        sourceSize.height: parent.height
                        mipmap: true
                    }
                    Text {
                        color: "white"
                        padding: 0
                        height: parent.height
                        text: "Firelight"
                        font.pixelSize: 24
                        font.weight: Font.Bold
                        font.family: Constants.regularFontFamily
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter

                    }
                }
                FLNavSection {
                    Layout.fillWidth: true
                    title: "LIBRARY"

                    focus: true

                    FLNavItem {
                        label: "All games"
                        iconName: "book"
                        onClicked: {
                            contentStack.pushItem(allGamesPage, {}, StackView.PushTransition)
                        }
                    }

                    FLNavItem {
                        label: "Favorites"
                        iconName: "kid-star"
                    }

                    FLNavItem {
                        label: "Recent"
                        iconName: "history"
                    }

                    FLNavItem {
                        label: "Newest"
                        iconName: "add"
                    }
                }
                FLNavSection {
                    title: "MOD SHOP"
                    Layout.fillWidth: true

                    FLNavItem {
                        label: "Browse"
                        iconName: "browse"
                        onClicked: {
                            contentStack.pushItem(shopPage, {}, StackView.PushTransition)
                        }
                    }

                    FLNavItem {
                        label: "Featured"
                        iconName: "bookmark-star"
                    }

                    FLNavItem {
                        label: "New Arrivals"
                        iconName: "star-shine"
                    }
                }
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

                        enabled: contentStack.depth > 1

                        onClicked: {
                            contentStack.popCurrentItem()
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

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.horizontalStretchFactor: 1
                    }

                    Pane {
                        Layout.fillHeight: true
                        Layout.preferredWidth: 460
                        Layout.alignment: Qt.AlignHCenter
                        padding: 8
                        horizontalPadding: 10
                        background: Rectangle {
                            color: "#1D1D1D"
                            opacity: 0.8
                            radius: 8
                            border.color: "#4B4B4B"
                        }

                        contentItem: RowLayout {
                            TextInput {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                color: "white"
                                font.pixelSize: 15
                                font.family: Constants.regularFontFamily
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter

                                HoverHandler {
                                    cursorShape: Qt.IBeamCursor
                                }

                                Text {
                                    text: "Search"
                                    anchors.fill: parent
                                    color: "#727272"
                                    font.pixelSize: 15
                                    font.family: Constants.regularFontFamily
                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter
                                    visible: (parent.length === 0) && !parent.activeFocus
                                }
                            }
                            FLIcon {
                                Layout.fillHeight: true
                                icon: "search"
                                size: height
                                color: "#7C7C7C"

                                HoverHandler {
                                    cursorShape: Qt.PointingHandCursor
                                }
                            }
                        }
                    }

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

    FLFocusHighlight {
        target: window.activeFocusItem
        usingMouse: InputMethodManager.usingMouse
    }

    // Pane {
    //     parent: Overlay.overlay
    //     z: 2
    //
    //     anchors.top: parent.top
    //     anchors.right: parent.right
    //     height: AppStyle.mainHeaderHeight
    //     padding: AppStyle.mainHeaderPadding
    //
    //     background: Item {
    //     }
    //
    //     RowLayout {
    //         spacing: 16
    //         // FirelightButton {
    //         //     iconCode: "\ue8b8"
    //         //     tooltipLabel: "Settings"
    //         //     flat: true
    //         //     circle: true
    //         //     Layout.fillHeight: true
    //         //     Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
    //         //
    //         //     onClicked: {
    //         //         quickMenuBar.visible = false
    //         //         screenStack.pushItem(settingsScreen, {}, StackView.PushTransition)
    //         //     }
    //         // }
    //
    //         Text {
    //             property bool drawColon: true
    //             Timer {
    //                 triggeredOnStart: true
    //                 interval: 1000
    //                 running: true
    //                 repeat: true
    //                 onTriggered: {
    //                     parent.text = new Date().toLocaleTimeString(Qt.locale("en_US"), Locale.ShortFormat)
    //                     if (!parent.drawColon) {
    //                         parent.text = parent.text.replace(":", " ")
    //                         parent.drawColon = true
    //                     } else {
    //                         parent.drawColon = false
    //                     }
    //                 }
    //                 // console.log("tick")
    //             }
    //             text: ""
    //             color: "white"
    //             font.pixelSize: 18
    //             horizontalAlignment: Text.AlignRight
    //             verticalAlignment: Text.AlignVCenter
    //             Layout.rightMargin: 12
    //             font.weight: Font.Normal
    //             font.family: Constants.regularFontFamily
    //             Layout.preferredWidth: 85
    //             Layout.fillHeight: true
    //         }
    //     }
    // }
    //
    // StackView {
    //     id: screenStack
    //     anchors.fill: parent
    //     anchors.topMargin: AppStyle.mainHeaderHeight
    //     anchors.bottomMargin: inputGuideBar.height
    //     padding: AppStyle.windowPadding
    //     focus: true
    //     // initialItem: FLTwoColumnMenu {
    //     //     menuItems: ["Home", "Controllers"]
    //     //     pages: [homeScreen, controllerPage]
    //     // }
    //     // initialItem: GeneralSettings.showNewUserFlow ? newUserFlow : homeScreen
    //
    //     initialItem: FLThing{
    //         libraryPage: libraryPage2
    //     }
    //
    //     property real blurAmount: window.blur ? 0.5 : 0
    //
    //     Behavior on blurAmount {
    //         NumberAnimation {
    //             easing.type: Easing.InOutQuad
    //             duration: 250
    //         }
    //     }
    //
    //     layer.enabled: blurAmount !== 0
    //     layer.effect: MultiEffect {
    //         // enabled: root.blurAmount !== 0
    //         source: screenStack
    //         anchors.fill: screenStack
    //         blurEnabled: true
    //         blurMultiplier: 0
    //         blurMax: 64
    //         blur: screenStack.blurAmount
    //     }
    //
    //     Keys.onPressed: function (event) {
    //         if (event.key === Qt.Key_Escape) {
    //             if (screenStack.depth > 1) {
    //                 screenStack.popCurrentItem()
    //             } else {
    //                 quickMenuBar.visible = true
    //             }
    //         } else {
    //             inputGuideBar.hidden = !inputGuideBar.hidden
    //         }
    //     }
    //
    //     pushEnter: Transition {
    //         ParallelAnimation {
    //             PropertyAnimation {
    //                 property: "scale"
    //                 from: 1.03
    //                 to: 1
    //                 duration: 250
    //                 easing.type: Easing.InOutQuad
    //             }
    //             PropertyAnimation {
    //                 property: "opacity"
    //                 from: 0
    //                 to: 1
    //                 duration: 250
    //                 easing.type: Easing.InOutQuad
    //             }
    //         }
    //     }
    //     pushExit: Transition {
    //         ParallelAnimation {
    //             PropertyAnimation {
    //                 property: "opacity"
    //                 from: 1
    //                 to: 0
    //                 duration: 250
    //                 easing.type: Easing.InOutQuad
    //             }
    //             PropertyAnimation {
    //                 property: "scale"
    //                 from: 1
    //                 to: 0.97
    //                 duration: 250
    //                 easing.type: Easing.OutQuad
    //             }
    //         }
    //     }
    //     popEnter: Transition {
    //         ParallelAnimation {
    //             PropertyAnimation {
    //                 property: "scale"
    //                 from: 0.97
    //                 to: 1
    //                 duration: 250
    //                 easing.type: Easing.InQuad
    //             }
    //             PropertyAnimation {
    //                 property: "opacity"
    //                 from: 0
    //                 to: 1
    //                 duration: 250
    //                 easing.type: Easing.InOutQuad
    //             }
    //         }
    //     }
    //     popExit: Transition {
    //         ParallelAnimation {
    //             PropertyAnimation {
    //                 property: "opacity"
    //                 from: 1
    //                 to: 0
    //                 duration: 250
    //                 easing.type: Easing.InOutQuad
    //             }
    //             PropertyAnimation {
    //                 property: "scale"
    //                 from: 1
    //                 to: 1.03
    //                 duration: 250
    //                 easing.type: Easing.InOutQuad
    //             }
    //         }
    //     }
    //
    //     replaceEnter: Transition {
    //         PropertyAnimation {
    //             property: "opacity"
    //             from: 0
    //             to: 1
    //             duration: 200
    //             easing.type: Easing.InOutQuad
    //         }
    //     }
    //     replaceExit: Transition {
    //         PropertyAnimation {
    //             property: "opacity"
    //             from: 1
    //             to: 0
    //             duration: 200
    //             easing.type: Easing.InOutQuad
    //         }
    //     }
    // }
    //
    // FLInputGuideBar {
    //     id: inputGuideBar
    //     target: window.activeFocusItem
    //     parent: Overlay.overlay
    //     width: parent.width
    //     anchors.bottom: parent.bottom
    //     z: 1
    // }
    //
    // Popup {
    //     id: quickMenuBar
    //     parent: Overlay.overlay
    //     modal: true
    //     focus: true
    //
    //     padding: AppStyle.windowPadding
    //
    //     closePolicy: Popup.NoAutoClose
    //
    //     width: 240
    //     height: parent.height - inputGuideBar.height
    //     // height: parent.height
    //
    //     onAboutToShow: function () {
    //         sfx_player.play("quickopen")
    //         window.blur = true
    //         if (window.gameRunning && screenStack.currentItem === emulatorLoader) {
    //             quickMenuStack.pushItem(nowPlayingPage, {
    //                 entryId: currentEntryId,
    //                 contentHash: currentContentHash,
    //                 undoEnabled: false
    //             }, StackView.PushTransition)
    //             quickMenuStack.forceActiveFocus()
    //         } else {
    //             menuBar.forceActiveFocus()
    //         }
    //     }
    //
    //     onAboutToHide: function () {
    //         window.blur = false
    //     }
    //
    //     Overlay.modal: Rectangle {
    //         color: "#131313"
    //         anchors.fill: parent
    //         opacity: 0.7
    //         Behavior on opacity {
    //             NumberAnimation {
    //                 duration: 200
    //             }
    //         }
    //     }
    //
    //     enter: Transition {
    //         ParallelAnimation {
    //             PropertyAnimation {
    //                 property: "opacity"
    //                 from: 0
    //                 to: 1
    //                 duration: 250
    //                 easing.type: Easing.InOutQuad
    //             }
    //             PropertyAnimation {
    //                 property: "x"
    //                 from: -20
    //                 to: 0
    //                 duration: 250
    //                 easing.type: Easing.InOutQuad
    //             }
    //         }
    //     }
    //
    //     exit: Transition {
    //         ParallelAnimation {
    //             PropertyAnimation {
    //                 property: "opacity"
    //                 from: 1
    //                 to: 0
    //                 duration: 250
    //                 easing.type: Easing.InOutQuad
    //             }
    //             PropertyAnimation {
    //                 property: "x"
    //                 from: 0
    //                 to: -20
    //                 duration: 250
    //                 easing.type: Easing.InOutQuad
    //             }
    //         }
    //     }
    //
    //     // background: Rectangle {
    //     //     color: ColorPalette.neutral900
    //     //     opacity: 0.96
    //     // }
    //
    //     background: Item {}
    //
    //     contentItem: FocusScope {
    //         focus: true
    //         id: contentThing
    //
    //         Keys.onEscapePressed: function (event) {
    //             sfx_player.play("quickclose")
    //             quickMenuBar.close()
    //         }
    //
    //         Keys.onBackPressed: function (event) {
    //             sfx_player.play("quickclose")
    //             quickMenuBar.close()
    //         }
    //
    //         ColumnLayout {
    //             width: contentThing.width
    //             anchors.top: parent.top
    //             anchors.left: parent.left
    //             anchors.bottom: parent.bottom
    //             // anchors.bottomMargin: 24
    //
    //             spacing: 16
    //
    //             FirelightButton {
    //                 id: closeButton
    //                 tooltipLabel: "Close menu"
    //                 flat: true
    //                 circle: true
    //                 tooltipOnRight: true
    //
    //                 iconCode: "\ue5cd"
    //
    //                 onClicked: {
    //                     sfx_player.play("quickclose")
    //                     quickMenuBar.close()
    //                 }
    //             }
    //
    //             FirelightButton {
    //                 tooltipLabel: "Library"
    //                 tooltipOnRight: true
    //                 circle: true
    //                 iconCode: "\ue88a"
    //                 onClicked: {
    //                     quickMenuBar.visible = false
    //                     screenStack.clear()
    //                     screenStack.pushItem(libraryPage2, {}, StackView.ReplaceTransition)
    //                 }
    //             }
    //
    //             FirelightButton {
    //                 tooltipLabel: "Mop Shop"
    //                 tooltipOnRight: true
    //                 circle: true
    //                 iconCode: "\uf1cc"
    //                 onClicked: {
    //                     quickMenuBar.visible = false
    //                     screenStack.clear()
    //                     screenStack.pushItem(shopPage, {}, StackView.ReplaceTransition)
    //                 }
    //             }
    //
    //             FirelightButton {
    //                 tooltipLabel: "Controllers"
    //                 tooltipOnRight: true
    //                 circle: true
    //                 iconCode: "\uf135"
    //             }
    //
    //             FirelightButton {
    //                 tooltipLabel: "Settings"
    //                 tooltipOnRight: true
    //                 circle: true
    //                 iconCode: "\ue8b8"
    //                 onClicked: {
    //                     quickMenuBar.visible = false
    //                     screenStack.clear()
    //                     screenStack.pushItem(settingsScreen, {}, StackView.ReplaceTransition)
    //                 }
    //             }
    //
    //             FirelightButton {
    //                 tooltipLabel: "Power"
    //                 tooltipOnRight: true
    //                 circle: true
    //                 iconCode: "\ue8ac"
    //             }
    //
    //             Item {
    //                 Layout.fillHeight: true
    //                 Layout.fillWidth: true
    //             }
    //         }
    //
    //         // ListView {
    //         //     id: menuBar
    //         //     // height: (spacing * 4) + 5 * 42
    //         //     height: parent.height
    //         //     focus: true
    //         //     width: contentThing.width
    //         //     // anchors.verticalCenter: parent.verticalCenter
    //         //     anchors.top: parent.top
    //         //     anchors.topMargin: 72
    //         //     orientation: ListView.Vertical
    //         //     currentIndex: 0
    //         //     interactive: false
    //         //
    //         //     keyNavigationEnabled: true
    //         //
    //         //     model: ListModel {
    //         //         ListElement {
    //         //             icon: "\ue88a"; label: "Home"
    //         //         }
    //         //         ListElement {
    //         //             icon: "\uf1cc"; label: "Mop Shop"
    //         //         }
    //         //         ListElement {
    //         //             icon: "\uf135"; label: "Controllers"
    //         //         }
    //         //         ListElement {
    //         //             icon: "\ue8b8"; label: "Settings"
    //         //         }
    //         //         ListElement {
    //         //             icon: "\ue8ac"; label: "Power"
    //         //         }
    //         //     }
    //         //     spacing: 12
    //         //     delegate: FirelightButton {
    //         //         id: theButton
    //         //         focus: true
    //         //         required property var model
    //         //         required property var index
    //         //         tooltipLabel: model.label
    //         //         tooltipOnRight: true
    //         //         circle: true
    //         //         flat: true
    //         //         iconCode: model.icon
    //         //     }
    //         //     // delegate: Text {
    //         //     //     text: model.label
    //         //     //     font.pixelSize: 18
    //         //     //     width: ListView.view.width
    //         //     //     height: 36
    //         //     //     color: "white"
    //         //     //     font.family: Constants.regularFontFamily
    //         //     //     horizontalAlignment: Text.AlignLeft
    //         //     //     verticalAlignment: Text.AlignVCenter
    //         //     //
    //         //     // }
    //         // }
    //     }
    // }

    Component {
        id: allGamesPage
        ListView {
            model: LibraryEntryModel
            delegate: Button {
                required property var model
                required property var index

                height: 60
                padding: 12

                background: Item {}
                contentItem: RowLayout {

                    FLIcon {
                        icon: "star"
                        color: "grey"
                        Layout.preferredHeight: 32
                        Layout.preferredWidth: 32
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                        size: height
                    }
                    Rectangle {
                        color: "white"
                        Layout.preferredHeight: 48
                        Layout.preferredWidth: 48
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    }
                    Text {
                        text: model.displayName
                        font.pixelSize: 17
                        font.weight: Font.Bold
                        font.family: Constants.regularFontFamily
                        color: "white"
                        Layout.fillHeight: true
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
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
        id: shopPage
        ShopLandingPage {
            property bool topLevel: true
            property string topLevelName: "shop"
            property string pageTitle: "Mod shop (coming soon!)"
            model: shop_item_model
        }
    }

    Component {
        id: settingsScreen

        SettingsScreen {
        }
    }
}
