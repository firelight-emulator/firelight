import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Layouts 1.0
import QtQuick.Effects
import Firelight 1.0

ApplicationWindow {
    id: window

    objectName: "Application Window"
    title: qsTr("Firelight")
    visibility: Window.Windowed
    visible: true

    width: GeneralSettings.mainWindowWidth
    height: GeneralSettings.mainWindowHeight
    x: GeneralSettings.mainWindowX
    y: GeneralSettings.mainWindowY

    Component.onCompleted: {
        WindowFrame.setWindow(window)
    }

    minimumWidth: 800
    minimumHeight: 600

    property real previousX: x
    property real previousY: y
    property real previousWidth: width
    property real previousHeight: height

    background: FLUserBackground {
        // blur: window.blur
        blurAmount: 0
        dimAmount: 0
        defaultColor: "#12131A"
        usingCustomBackground: false
        backgroundFile: AppearanceSettings.backgroundFile
    }

    onHeightChanged: {
        GeneralSettings.mainWindowHeight = height;
    }

    FLFocusHighlight {
        target: window.activeFocusItem
        usingMouse: InputMethodManager.usingMouse
    }

    onWidthChanged: {
        GeneralSettings.mainWindowWidth = width;
    }
    onXChanged: {
        GeneralSettings.mainWindowX = x;
    }
    onYChanged: {
        GeneralSettings.mainWindowY = y;
    }

    Popup {
        id: settingsModal

        anchors.centerIn: parent
        height: window.height - 120
        modal: true
        padding: 0
        parent: window
        width: window.width - 160

        Overlay.modal: Rectangle {
            color: "black"
            opacity: visible ? 0.7 : 0

            Behavior on opacity {
                NumberAnimation {
                    duration: 160
                    easing.type: Easing.InOutQuad
                }
            }
        }
        background: Rectangle {
            color: "#1e1e1e"
            radius: 8
        }
        contentItem: Item {
            Pane {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.top: parent.top
                bottomPadding: 0
                padding: 12
                width: 260

                background: Rectangle {
                    bottomLeftRadius: 8
                    color: Qt.darker("#1e1e1e", 1.25)
                    topLeftRadius: 8
                }
                contentItem: ColumnLayout {
                    Rectangle {
                        Layout.fillWidth: true
                        color: "red"
                        implicitHeight: 72
                    }
                    ListView {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        boundsBehavior: ListView.StopAtBounds
                        clip: true
                        model: 20

                        delegate: Button {
                            height: 40
                            hoverEnabled: true
                            padding: 8
                            width: ListView.view.width

                            background: Rectangle {
                                color: hovered ? "#FFFFFF" : "transparent"
                                opacity: hovered ? 0.08 : 0
                                radius: 4
                            }
                            contentItem: Text {
                                color: "#bababa"
                                font.family: Constants.regularFontFamily
                                font.pointSize: 12
                                font.weight: Font.DemiBold
                                text: "Setting " + (index + 1)
                            }
                        }
                    }
                }
            }
        }
        enter: Transition {
            NumberAnimation {
                duration: 160
                easing.type: Easing.InOutQuad
                from: 0.0
                property: "opacity"
                to: 1.0
            }
            NumberAnimation {
                duration: 160
                easing.type: Easing.InOutQuad
                from: 1.0
                property: "scale"
                to: 1.03
            }
        }
        exit: Transition {
            NumberAnimation {
                duration: 160
                easing.type: Easing.InOutQuad
                from: 1.0
                property: "opacity"
                to: 0.0
            }
            NumberAnimation {
                duration: 160
                easing.type: Easing.InOutQuad
                from: 1.03
                property: "scale"
                to: 1.0
            }
        }
    }

    // Pane {
    //     id: topBar
    //     anchors.top: parent.top
    //     anchors.right: parent.right
    //     anchors.left: parent.left
    //     height: 48
    //     padding: 8
    //
    //     // background: Item {
    //     //     Rectangle {
    //     //         id: theMask
    //     //         visible: false
    //     //         anchors.fill: parent
    //     //         color: "#000000"
    //     //         radius: 0
    //     //         layer.enabled: true
    //     //         layer.smooth: true
    //     //     }
    //     //
    //     //     MultiEffect {
    //     //         source: ShaderEffectSource {
    //     //             width: topBar.width
    //     //             height: topBar.height
    //     //             sourceItem: window.background
    //     //             sourceRect: Qt.rect(topBar.x, topBar.y, topBar.width, topBar.height)
    //     //         }
    //     //
    //     //         maskEnabled: true
    //     //         maskSource: theMask
    //     //         maskThresholdMin: 0.5
    //     //         maskSpreadAtMin: 1.0
    //     //         // maskInverted: true
    //     //         // paddingRect: Qt.rect(-2, -2, -2, -2)
    //     //         autoPaddingEnabled: false
    //     //         anchors.fill: parent
    //     //         blurEnabled: true
    //     //         blurMultiplier: 1.0
    //     //         blurMax: 48
    //     //         blur: 1.0
    //     //     }
    //     //
    //     //     Rectangle {
    //     //         anchors.fill: parent
    //     //         color: "#000000"
    //     //         opacity: 0.1
    //     //         radius: 0
    //     //     }
    //     //
    //     //     Rectangle {
    //     //         anchors.fill: parent
    //     //         anchors.topMargin: -1
    //     //         anchors.leftMargin: -1
    //     //         anchors.rightMargin: -1
    //     //         color: "transparent"
    //     //         opacity: 0.08
    //     //
    //     //
    //     //         border.width: 1
    //     //         border.color: "white"
    //     //
    //     //         radius: 0
    //     //     }
    //     // }
    //
    //     background: Item {}
    //
    //     MainTopBar {
    //         id: mainBar
    //         anchors.fill: parent
    //     }
    //
    //     // Rectangle {
    //     //     width: parent.width
    //     //     height: 1
    //     //     color: "white"
    //     //     opacity: 0.14
    //     //     anchors.top: parent.bottom
    //     // }
    // }

    // IconButton {
    //     id: menuButton
    //     anchors.left: parent.left
    //     anchors.top: parent.top
    //     anchors.margins: 12
    //     z: 10
    //     icon.source: "qrc:/icons/menu"
    //
    //     onClicked: {
    //         if (navBar.anchors.leftMargin === -240) {
    //             navBar.anchors.leftMargin = 0
    //         } else {
    //             navBar.anchors.leftMargin = -240
    //         }
    //     }
    // }

    // Item {
    //     anchors.top: parent.top
    //     anchors.left: parent.left
    //     anchors.bottom: parent.bottom
    //     width: 16
    //     z: 10
    //
    //     HoverHandler {
    //         id: grabbyHoverHandler
    //     }
    //
    //     Rectangle {
    //         height: 80
    //         width: 6
    //         radius: width / 2
    //         color: "white"
    //         opacity: grabbyHoverHandler.hovered ? 0.2 : 0.12
    //         anchors.right: parent.right
    //         anchors.verticalCenter: parent.verticalCenter
    //         scale: grabbyHoverHandler.hovered ? 1.15 : 1
    //
    //         Behavior on scale {
    //             NumberAnimation {
    //                 duration: 160
    //                 easing.type: Easing.InOutQuad
    //             }
    //         }
    //
    //         Behavior on opacity {
    //             NumberAnimation {
    //                 duration: 160
    //                 easing.type: Easing.InOutQuad
    //             }
    //         }
    //     }
    //
    //     TapHandler {
    //         onTapped: {
    //             navBarModal.open()
    //         }
    //     }
    // }



    // Pane {
    //     id: navBar
    //
    //     anchors.bottom: parent.bottom
    //     anchors.left: parent.left
    //     anchors.top: parent.top
    //     anchors.leftMargin: -240
    //     width: 240
    //
    //     Behavior on width {
    //         NumberAnimation {
    //             duration: 160
    //             easing.type: Easing.InOutQuad
    //         }
    //     }
    //
    //     TapHandler {
    //         onTapped: {
    //             if (navBar.width === 240) {
    //                 navBar.width = 42 + (navBar.padding * 2)
    //             } else {
    //                 navBar.width = 240
    //             }
    //         }
    //     }
    //
    //     // background: Item {
    //     //     Rectangle {
    //     //         id: theMask
    //     //         visible: false
    //     //         anchors.fill: parent
    //     //         color: "#000000"
    //     //         radius: 8
    //     //         layer.enabled: true
    //     //         layer.smooth: true
    //     //     }
    //     //
    //     //     MultiEffect {
    //     //         source: ShaderEffectSource {
    //     //             width: navBar.width
    //     //             height: navBar.height
    //     //             sourceItem: window.background
    //     //             sourceRect: Qt.rect(navBar.x, navBar.y, navBar.width, navBar.height)
    //     //         }
    //     //
    //     //         maskEnabled: true
    //     //         maskSource: theMask
    //     //         maskThresholdMin: 0.5
    //     //             maskSpreadAtMin: 1.0
    //     //         // maskInverted: true
    //     //         // paddingRect: Qt.rect(-2, -2, -2, -2)
    //     //         autoPaddingEnabled: false
    //     //         anchors.fill: parent
    //     //         blurEnabled: true
    //     //         blurMultiplier: 1.0
    //     //         blurMax: 48
    //     //         blur: 1.0
    //     //     }
    //     //
    //     //     Rectangle {
    //     //         anchors.fill: parent
    //     //         color: "#000000"
    //     //         opacity: 0.1
    //     //         radius: 8
    //     //     }
    //     //
    //     //     Rectangle {
    //     //         anchors.fill: parent
    //     //         color: "transparent"
    //     //         opacity: 0.14
    //     //
    //     //
    //     //         border.width: 1
    //     //         border.color: "white"
    //     //
    //     //         radius: 8
    //     //     }
    //     // }
    //
    //     background: Item {
    //         Rectangle {
    //             anchors.leftMargin: -70
    //             anchors.bottom: parent.bottom
    //             anchors.left: parent.left
    //             anchors.top: parent.top
    //             anchors.right: parent.right
    //             gradient: Gradient {
    //                 orientation: Gradient.Horizontal
    //                 GradientStop {
    //                     position: 0.0; color: "black"
    //                 }
    //                 GradientStop {
    //                     position: 1.0; color: "transparent"
    //                 }
    //             }
    //         }
    //     }
    //
    //     LeftNavigationBar2 {
    //         anchors.fill: parent
    //     }
    // }
    //
    // Pane {
    //     id: topBar
    //
    //     anchors.left: navBar.right
    //     anchors.right: parent.right
    //     anchors.top: parent.top
    //     height: 48
    //
    //     background: Item {}
    //
    //     contentItem: RowLayout {
    //         // Text {
    //         //     Layout.fillHeight: true
    //         //     color: "#FFFFFF"
    //         //     font.family: Constants.regularFontFamily
    //         //     font.pointSize: 14
    //         //     font.weight: Font.DemiBold
    //         //     horizontalAlignment: Text.AlignHCenter
    //         //     text: "Library"
    //         //     verticalAlignment: Text.AlignVCenter
    //         // }
    //     }
    // }

    // Pane {
    //     id: gameGrid
    //     anchors.left: navBar.right
    //     anchors.right: parent.right
    //     anchors.top: topBar.bottom
    //     anchors.bottom: parent.bottom
    //     anchors.margins: 6
    //
    //     background: Item {
    //         Rectangle {
    //             id: theMask3
    //             visible: false
    //             anchors.fill: parent
    //             color: "#000000"
    //             radius: 8
    //             layer.enabled: true
    //             layer.smooth: true
    //         }
    //
    //         MultiEffect {
    //             source: ShaderEffectSource {
    //                 width: gameGrid.width
    //                 height: gameGrid.height
    //                 sourceItem: window.background
    //                 sourceRect: Qt.rect(gameGrid.x, gameGrid.y, gameGrid.width, gameGrid.height)
    //             }
    //
    //             maskEnabled: true
    //             maskSource: theMask3
    //             maskThresholdMin: 0.5
    //                 maskSpreadAtMin: 1.0
    //             // maskInverted: true
    //             // paddingRect: Qt.rect(-2, -2, -2, -2)
    //             autoPaddingEnabled: false
    //             anchors.fill: parent
    //             blurEnabled: true
    //             blurMultiplier: 1.0
    //             blurMax: 48
    //             blur: 1.0
    //         }
    //
    //         Rectangle {
    //             anchors.fill: parent
    //             color: "#000000"
    //             opacity: 0.1
    //             radius: 8
    //         }
    //
    //         Rectangle {
    //             anchors.fill: parent
    //             color: "transparent"
    //             opacity: 0.14
    //
    //
    //             border.width: 1
    //             border.color: "white"
    //
    //             radius: 8
    //         }
    //     }
    // }

    Pane {
        id: topBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: AppStyle.topBarHeight
        padding: AppStyle.topBarPadding

        background: Rectangle {
            color: "transparent"
            anchors.fill: parent
        }

        MainTopBar {
            id: mainBar
            anchors.fill: parent

            onMaximizeClicked: {
                if (window.visibility === Window.Maximized) {
                    window.showNormal()
                    window.x = window.previousX
                    window.y = window.previousY
                    window.width = window.previousWidth
                    window.height = window.previousHeight
                } else {
                    window.previousX = window.x
                    window.previousY = window.y
                    window.previousWidth = window.width
                    window.previousHeight = window.height
                    window.showMaximized()
                }
            }
            onMinimizeClicked: window.showMinimized()
            onCloseClicked: window.close()
        }

    }

    Pane {
        id: nowPlayingBar
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        height: 0

        background: Rectangle {
            color: "transparent"
        }

        Text {
            anchors.centerIn: parent
            color: "#FFFFFF"
            font.family: Constants.regularFontFamily
            font.pointSize: 14
            text: "Now playing or something idk"
        }
    }

    Rectangle {
        anchors.fill: splitView
        color: "#1b1d27"
        radius: 8
    }

    SplitView {
        id: splitView

        anchors.bottom: nowPlayingBar.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: topBar.bottom
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        anchors.bottomMargin: 8
        anchors.topMargin: 2
        orientation: Qt.Horizontal
        spacing: 0

        clip: true

        handle: Item {
            SplitView.fillHeight: true
            implicitWidth: 8

            HoverHandler {
                id: handleHoverHandler
            }

            Rectangle {
                anchors.centerIn: parent
                color: "#FFFFFF"
                height: parent.height - 16
                opacity: handleHoverHandler.hovered ? 0.25 : 0
                width: 2

                Behavior on opacity {
                    NumberAnimation {
                        duration: 160
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        }

        Pane {
            id: libraryNavigationPane
            SplitView.fillHeight: true
            SplitView.maximumWidth: 280
            SplitView.minimumWidth: 60
            SplitView.preferredWidth: 280

            background: Rectangle {
                color: Qt.lighter("#1b1d27", 1.15)
                topLeftRadius: 8
                bottomLeftRadius: 8
            }
            topPadding: 0
            bottomPadding: 0
            leftPadding: 0
            rightPadding: 0
            // topPadding: 8
            // clip: true

            // padding: 0
            //

            ButtonGroup {
                id: libraryButtonGroup
                exclusive: true
            }

            contentItem: ColumnLayout {
                FocusScope {
                    id: folderListContainer
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    // clip: true

                    ColumnLayout {
                        id: staticMenuColumn
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right

                        spacing: 0

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 32
                            Layout.rightMargin: 12
                            spacing: 12

                            Text {
                                Layout.fillHeight: true
                                Layout.topMargin: 10
                                Layout.leftMargin: 16
                                text: "Your stuff"
                                color: "#ffffff"
                                font.family: Constants.regularFontFamily
                                font.pointSize: 12
                                font.weight: Font.DemiBold
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                            }

                            Item {
                                Layout.fillWidth: true
                                implicitHeight: 1
                            }

                            FLIcon {
                                Layout.fillHeight: true
                                Layout.topMargin: 13
                                icon: "add"
                                size: 26
                                color: "#ffffff"
                                opacity: 0.8
                                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                            }
                        }

                        LibraryNavigationMenuSection {
                            id: libraryMenuSection
                            Layout.fillWidth: true
                            Layout.topMargin: 8

                            title: ""
                            model: [
                                { displayName: "All games", iconName: "browse" },
                                { displayName: "Favorites", iconName: "favorite" }
                            ]
                            focus: true

                            onActiveFocusChanged: {
                                if (activeFocus) {
                                    platformMenuSection.currentIndex = 0
                                    folderMenuSection.currentIndex = 0
                                }
                            }

                            collapsible: false

                            KeyNavigation.down: platformMenuSection.collapsed ? folderMenuSection.collapsed ? null : folderMenuSection : platformMenuSection

                            delegate: LibraryNavigationMenuItem {
                                id: menuItem
                                required property var model

                                iconSource: "qrc:/icons/" + model.iconName
                                displayText: model.displayName
                                numberOfItems: {
                                    if (model.displayName === "All games") {
                                        return LibraryEntryModel.rowCount()
                                    } else if (model.displayName === "Favorites") {
                                        return LibraryEntryModel.numFavorites
                                    } else {
                                        return 0
                                    }
                                }

                                width: ListView.view.width
                                ButtonGroup.group: libraryButtonGroup

                                onCheckedChanged: {
                                    if (menuItem.checked) {
                                        if (model.displayName === "All games") {
                                            gameView.clearFilters();
                                        } else if (model.displayName === "Favorites") {
                                            gameView.filterByFavorites();
                                        }
                                    }
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.leftMargin: 8
                            Layout.rightMargin: 8
                            implicitHeight: 36
                            radius: 4
                            color: Qt.lighter("#1b1d27", 1.6)
                        }
                    }
                    Rectangle {
                        anchors.left: folderList.left
                        anchors.top: folderList.top
                        anchors.right: folderList.right
                        height: 40
                        gradient: Gradient {
                            orientation: Gradient.Vertical
                            GradientStop {
                                position: 0.0; color: Qt.darker("#1b1d27", 1.2)
                            }
                            GradientStop {
                                position: 1.0; color: "transparent"
                            }
                        }
                        z: 5
                        opacity: folderList.contentY > 16 ? 1 : 0
                        Behavior on opacity {
                            NumberAnimation {
                                duration: 160
                                easing.type: Easing.InOutQuad
                            }
                        }
                        // contentItem: Rectangle {
                        //     color: "#ffffff"
                        //     radius: 4
                        //     opacity: 0.12
                        // }
                    }

                    Flickable {
                        id: folderList
                        anchors.top: staticMenuColumn.bottom
                        anchors.topMargin: 16
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        clip: true
                        contentHeight: libraryNavColumn.implicitHeight
                        boundsBehavior: Flickable.StopAtBounds

                        ScrollBar.vertical: FLScrollBar {
                            anchors.right: parent.right
                            anchors.rightMargin: -4
                            width: 0
                         }

                        ColumnLayout {
                            id: libraryNavColumn
                            spacing: 0

                            anchors.fill: parent

                            LibraryNavigationMenuSection {
                                id: platformMenuSection
                                Layout.fillWidth: true
                                title: "Platforms"
                                model: PlatformModel
                                focus: true

                                KeyNavigation.down: folderMenuSection.collapsed ? null : folderMenuSection

                                onActiveFocusChanged: {
                                    if (activeFocus) {
                                        folderMenuSection.currentIndex = 0
                                        libraryMenuSection.currentIndex = libraryMenuSection.count - 1
                                    }
                                }

                                delegate: LibraryNavigationMenuItem {
                                    id: platformMenuItem
                                    required property var model

                                    iconSource: "qrc:/icons/" + model.iconName
                                    displayText: model.displayName
                                    numberOfItems: LibraryEntryModel.countByPlatform[model.platformId]

                                    width: ListView.view.width
                                    ButtonGroup.group: libraryButtonGroup

                                    onCheckedChanged: {
                                        if (platformMenuItem.checked) {
                                            gameView.filterByPlatform(model.platformId);
                                        }
                                    }
                                }
                            }

                            LibraryNavigationMenuSection {
                                id: folderMenuSection
                                Layout.fillWidth: true
                                title: "Folders"
                                model: LibraryFolderModel

                                KeyNavigation.up: platformMenuSection.collapsed ? libraryMenuSection : platformMenuSection

                                onActiveFocusChanged: {
                                    if (activeFocus) {
                                        platformMenuSection.currentIndex = platformMenuSection.count - 1
                                        libraryMenuSection.currentIndex = libraryMenuSection.count - 1
                                    }
                                }

                                delegate: LibraryNavigationMenuItem {
                                    id: folderMenuItem
                                    required property var model
                                    focus: true

                                    iconSource: model.icon1x1SourceUrl
                                    displayText: model.displayName
                                    numberOfItems: {
                                        var num = LibraryEntryModel.countByFolderId[model.folderId]
                                        return num !== undefined ? num : 0
                                    }

                                    containsDrag: dropArea.containsDrag
                                    width: ListView.view.width
                                    ButtonGroup.group: libraryButtonGroup

                                    onCheckedChanged: {
                                        if (folderMenuItem.checked) {
                                            gameView.filterByFolderId(model.folderId);
                                        }
                                    }

                                    DropArea {
                                        id: dropArea

                                        anchors.fill: parent

                                        onDropped: function (event) {
                                            var entryId = event.text
                                            LibraryEntryModel.addEntryToFolder(entryId, model.folderId)
                                        }
                                    }
                                }
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

        Pane {
            id: gamesPanel

            property real achievementColumnWidth: 0
            property real lastPlayedColumnWidth: 0
            property real timePlayedColumnWidth: 0
            property real titleColumnWidth: 0

            SplitView.fillHeight: true
            SplitView.fillWidth: true
            bottomPadding: 0
            horizontalPadding: 24
            topPadding: 0
            verticalPadding: 0
            // clip: true

            background: Rectangle {
                color: "#1b1d27"
                topRightRadius: detailsPanel.width > 0 ? 0 : 8
                bottomRightRadius: detailsPanel.width > 0 ? 0 : 8
            }

            contentItem: GameView {
                id: gameView
            }
        }

        Pane {
            id: detailsPanel

            SplitView.fillHeight: true
            SplitView.fillWidth: true

            onWidthChanged: {
                console.log("Details panel width: " + width)
            }

            padding: 8
            clip: true

            background: Rectangle {
                color: Qt.darker("#1b1d27", 1.2)
                topRightRadius: 8
                bottomRightRadius: 8
            }
        }
    }

    // Resize edges — z:100 so they sit above all content
    Item {
        anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
        width: 6; z: 100
        HoverHandler { cursorShape: Qt.SizeHorCursor }
        DragHandler { target: null; onActiveChanged: if (active) window.startSystemResize(Qt.LeftEdge) }
    }
    Item {
        anchors { right: parent.right; top: parent.top; bottom: parent.bottom }
        width: 6; z: 100
        HoverHandler { cursorShape: Qt.SizeHorCursor }
        DragHandler { target: null; onActiveChanged: if (active) window.startSystemResize(Qt.RightEdge) }
    }
    Item {
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
        height: 6; z: 100
        HoverHandler { cursorShape: Qt.SizeVerCursor }
        DragHandler { target: null; onActiveChanged: if (active) window.startSystemResize(Qt.BottomEdge) }
    }
    Item {
        anchors { left: parent.left; bottom: parent.bottom }
        width: 12; height: 12; z: 100
        HoverHandler { cursorShape: Qt.SizeBDiagCursor }
        DragHandler { target: null; onActiveChanged: if (active) window.startSystemResize(Qt.LeftEdge | Qt.BottomEdge) }
    }
    Item {
        anchors { right: parent.right; bottom: parent.bottom }
        width: 12; height: 12; z: 100
        HoverHandler { cursorShape: Qt.SizeFDiagCursor }
        DragHandler { target: null; onActiveChanged: if (active) window.startSystemResize(Qt.RightEdge | Qt.BottomEdge) }
    }

    component RoleData: QtObject {
        property string displayName
    }
}
