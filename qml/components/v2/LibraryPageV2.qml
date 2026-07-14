import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Layouts 1.0
import QtQuick.Effects
import Firelight 1.0

SplitView {
    id: splitView
    orientation: Qt.Horizontal
    spacing: 0

    clip: true

    CreateFolderDialog {
        id: createFolderDialog
        onAccepted: LibraryFolderModel.addFolder(createFolderDialog.folderName)
    }

    SmartFolderDialog {
        id: smartFolderDialog
    }

    handle: Item {
        SplitView.fillHeight: true
        implicitWidth: 0

        containmentMask: Item {
            height: splitView.height
            width: 8
            x: -4

            HoverHandler {
                id: handleHoverHandler
            }
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
            color: Theme.glass
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
            spacing: 0
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
                        Layout.preferredHeight: 48
                        Layout.rightMargin: 12
                        spacing: 12

                        Text {
                            Layout.fillHeight: true
                            text: "Library"
                            color: "#ffffff"
                            font.family: Constants.regularFontFamily
                            font.pixelSize: AppStyle.fontSizeMedium
                            font.weight: Font.DemiBold
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        Item {
                            Layout.fillWidth: true
                            implicitHeight: 1
                        }

                        FLIcon {
                            id: addFolderIcon
                            Layout.fillHeight: true
                            icon: "add"
                            size: 26
                            color: "#ffffff"
                            opacity: newFolderHover.hovered ? 1.0 : 0.8
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                            HoverHandler {
                                id: newFolderHover
                            }

                            TapHandler {
                                onTapped: newFolderMenu.popup()
                            }

                            Menu {
                                id: newFolderMenu

                                MenuItem {
                                    text: "New folder"
                                    onTriggered: createFolderDialog.open()
                                }
                                MenuItem {
                                    text: "New smart folder"
                                    onTriggered: {
                                        smartFolderDialog.editFolderId = -1;
                                        smartFolderDialog.open();
                                    }
                                }
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: 1
                        color: "#123123"
                    }
                }
                // Rectangle {
                //     anchors.left: folderList.left
                //     anchors.top: folderList.top
                //     anchors.right: folderList.right
                //     height: 40
                //     gradient: Gradient {
                //         orientation: Gradient.Vertical
                //         GradientStop {
                //             position: 0.0; color: Theme.surface
                //         }
                //         GradientStop {
                //             position: 1.0; color: "transparent"
                //         }
                //     }
                //     z: 5
                //     opacity: folderList.contentY > 16 ? 1 : 0
                //     Behavior on opacity {
                //         NumberAnimation {
                //             duration: 160
                //             easing.type: Easing.InOutQuad
                //         }
                //     }
                //     // contentItem: Rectangle {
                //     //     color: "#ffffff"
                //     //     radius: 4
                //     //     opacity: 0.12
                //     // }
                // }

                Flickable {
                    id: folderList
                    anchors.top: staticMenuColumn.bottom
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
                            id: libraryMenuSection
                            Layout.fillWidth: true

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
                                accentColor: model.color
                                numberOfItems: {
                                    var num = LibraryEntryModel.countByFolderId[model.folderId]
                                    return num !== undefined ? num : 0
                                }

                                containsDrag: dropArea.containsDrag
                                width: ListView.view.width
                                ButtonGroup.group: libraryButtonGroup

                                onCheckedChanged: {
                                    if (folderMenuItem.checked) {
                                        gameView.filterByFolderId(model.folderId, model.folderType === 1, model.sortRole, model.sortAscending);
                                    }
                                }

                                // Smart folders compute membership from criteria,
                                // so dropping a game into one wouldn't stick.
                                DropArea {
                                    id: dropArea

                                    anchors.fill: parent
                                    enabled: model.folderType !== 1

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
            color: Theme.glassElevated
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
            color: Theme.glassElevated
            topRightRadius: 8
            bottomRightRadius: 8
        }
    }
}