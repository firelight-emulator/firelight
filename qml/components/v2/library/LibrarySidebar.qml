import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0

// TODO
// The library nav sidebar: collapse rail, All games, Platforms, and the folder
// tree (context menus, color, drag-drop reorder/nest). Scopes the GameView passed
// as `gameView`; folder ops use the dialogs + models passed in
Pane {
    id: libraryNavigationPane

    // Injected by the page so the sidebar stays decoupled: the SplitView root
    // (page), the games view it scopes, the owned-platforms model, and the
    // folder dialogs it opens
    property LibraryPageV2 page
    property GameView gameView
    property var ownedPlatformsModel
    property FolderDialog folderDialog

    readonly property int fullWidth: AppStyle.sidebarWidth
    readonly property int railWidth: AppStyle.sidebarRailWidth
    // Animating a local property (not SplitView.preferredWidth directly, which
    // can't take a Behavior) makes the collapse glide
    property real animatedWidth: page.sidebarCollapsed ? railWidth : fullWidth
    Behavior on animatedWidth {
        NumberAnimation {
            duration: 180
            easing.type: Easing.InOutQuad
        }
    }

    SplitView.fillHeight: true
    SplitView.minimumWidth: railWidth
    SplitView.maximumWidth: fullWidth
    SplitView.preferredWidth: animatedWidth

    background: Rectangle {
        color: Theme.glassElevated
        topLeftRadius: AppStyle.radiusMd
        bottomLeftRadius: AppStyle.radiusMd
    }
    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

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
                    Layout.rightMargin: AppStyle.spacingMd
                    Layout.leftMargin: AppStyle.spacingMd
                    spacing: AppStyle.spacingMd

                    FLIconButton {
                        iconName: page.sidebarCollapsed ? "chevron-forward" : "chevron-back"
                        compact: true
                        tooltipText: page.sidebarCollapsed ? "Expand sidebar" : "Collapse sidebar"
                        Layout.alignment: Qt.AlignVCenter
                        onClicked: page.sidebarCollapsed = !page.sidebarCollapsed
                    }

                    Text {
                        visible: !page.sidebarCollapsed
                        Layout.fillHeight: true
                        text: "Library"
                        color: Theme.textPrimary
                        font.family: AppStyle.fontFamily
                        font.pixelSize: AppStyle.fontSizeMedium
                        font.weight: Font.DemiBold
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                    }

                    Item {
                        visible: !page.sidebarCollapsed
                        Layout.fillWidth: true
                        implicitHeight: 1
                    }

                    FLIconButton {
                        visible: !page.sidebarCollapsed
                        iconName: "add"
                        Layout.topMargin: 2
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        onClicked: newFolderMenu.open()

                        RightClickMenu {
                            id: newFolderMenu

                            RightClickMenuItem {
                                text: "New folder"
                                onTriggered: folderDialog.openForCreate(false, -1)
                            }
                            RightClickMenuItem {
                                text: "New smart folder"
                                onTriggered: folderDialog.openForCreate(true, -1)
                            }
                        }
                    }
                }

                FLDivider {}
            }

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
                    width: AppStyle.spacingSm
                }

                ColumnLayout {
                    id: libraryNavColumn
                    spacing: 0

                    anchors.fill: parent

                    LibraryNavigationMenu {
                        id: libraryMenuSection
                        Layout.fillWidth: true

                        title: ""
                        model: [
                            {
                                displayName: "All games",
                                iconName: "browse"
                            }
                        ]
                        focus: true

                        onActiveFocusChanged: {
                            if (activeFocus) {
                                platformMenuSection.currentIndex = 0;
                                folderMenuSection.currentIndex = 0;
                            }
                        }

                        collapsible: false

                        KeyNavigation.down: folderMenuSection.collapsed ? (platformMenuSection.collapsed ? null : platformMenuSection) : folderMenuSection

                        delegate: LibraryNavigationMenuItem {
                            id: menuItem
                            required property var model

                            iconSource: "qrc:/icons/" + model.iconName
                            displayText: model.displayName
                            numberOfItems: model.displayName === "All games" ? LibraryEntryModel.rowCount() : 0

                            width: ListView.view.width
                            ButtonGroup.group: libraryButtonGroup

                            onCheckedChanged: {
                                if (menuItem.checked && model.displayName === "All games") {
                                    gameView.setScopeAll();
                                }
                            }
                        }
                    }

                    LibraryNavigationMenu {
                        id: folderMenuSection
                        Layout.fillWidth: true
                        title: "Folders"
                        model: page.folderRows

                        KeyNavigation.up: libraryMenuSection
                        KeyNavigation.down: platformMenuSection.collapsed ? null : platformMenuSection

                        onActiveFocusChanged: {
                            if (activeFocus) {
                                platformMenuSection.currentIndex = platformMenuSection.count - 1;
                                libraryMenuSection.currentIndex = libraryMenuSection.count - 1;
                            }
                        }

                        delegate: LibraryNavigationMenuItem {
                            id: folderMenuItem
                            required property var modelData
                            focus: true

                            treeItem: true
                            depth: modelData.depth
                            hasChildren: modelData.hasChildren
                            expanded: modelData.expanded
                            onToggleExpanded: page.toggleFolderExpanded(modelData.folderId)

                            ContextMenu.menu: RightClickMenu {
                                RightClickMenuItem {
                                    text: qsTr("New subfolder")
                                    onTriggered: folderDialog.openForCreate(false, folderMenuItem.modelData.folderId)
                                }
                                RightClickMenuItem {
                                    text: qsTr("Edit")
                                    onTriggered: folderDialog.loadForEdit(folderMenuItem.modelData.folderId, folderMenuItem.modelData.folderType === 1, folderMenuItem.modelData.displayName, folderMenuItem.modelData.color, folderMenuItem.modelData.icon1x1SourceUrl, folderMenuItem.modelData.description, folderMenuItem.modelData.filterJson, folderMenuItem.modelData.sortRole, folderMenuItem.modelData.sortAscending)
                                }
                                RightClickMenu {
                                    id: colorMenu
                                    title: qsTr("Color")
                                    function apply(c) {
                                        LibraryFolderModel.setFolderColor(folderMenuItem.modelData.folderId, c);
                                    }
                                    RightClickMenuItem {
                                        text: qsTr("Default")
                                        onTriggered: colorMenu.apply("")
                                    }
                                    RightClickMenuItem {
                                        text: qsTr("Red")
                                        onTriggered: colorMenu.apply(Theme.folderColors[0])
                                    }
                                    RightClickMenuItem {
                                        text: qsTr("Orange")
                                        onTriggered: colorMenu.apply(Theme.folderColors[1])
                                    }
                                    RightClickMenuItem {
                                        text: qsTr("Yellow")
                                        onTriggered: colorMenu.apply(Theme.folderColors[2])
                                    }
                                    RightClickMenuItem {
                                        text: qsTr("Green")
                                        onTriggered: colorMenu.apply(Theme.folderColors[3])
                                    }
                                    RightClickMenuItem {
                                        text: qsTr("Blue")
                                        onTriggered: colorMenu.apply(Theme.folderColors[4])
                                    }
                                    RightClickMenuItem {
                                        text: qsTr("Purple")
                                        onTriggered: colorMenu.apply(Theme.folderColors[5])
                                    }
                                    RightClickMenuItem {
                                        text: qsTr("Pink")
                                        onTriggered: colorMenu.apply(Theme.folderColors[6])
                                    }
                                }
                                RightClickMenuItem {
                                    text: qsTr("Delete folder")
                                    dangerous: true
                                    onTriggered: LibraryFolderModel.deleteFolder(folderMenuItem.modelData.folderId)
                                }
                            }

                            iconName: modelData.folderType === 1 ? "bookmark-star" : "folder"
                            customIconUrl: modelData.icon1x1SourceUrl
                            iconColor: modelData.color !== "" ? modelData.color : Theme.textPrimary
                            displayText: modelData.displayName !== "" ? modelData.displayName : qsTr("Untitled folder")
                            accentColor: modelData.color
                            numberOfItems: {
                                var num = LibraryEntryModel.countByFolderId[modelData.folderId];
                                return num !== undefined ? num : 0;
                            }

                            containsDrag: dropArea.dropZone === "into"
                            width: ListView.view.width
                            ButtonGroup.group: libraryButtonGroup

                            // Drag a folder onto another to re-nest it, or onto a
                            // row's top/bottom edge to reorder. mimeData + active
                            // are set imperatively in the grab callback (a
                            // declarative Drag.active binding never starts the drag)
                            Drag.dragType: Drag.Automatic
                            Drag.supportedActions: Qt.MoveAction

                            DragHandler {
                                id: folderDrag
                                target: null
                                onActiveChanged: {
                                    if (active) {
                                        folderMenuItem.grabToImage(function (result) {
                                            folderMenuItem.Drag.imageSource = result.url;
                                            folderMenuItem.Drag.mimeData = {
                                                "application/x-fl-folder": "" + folderMenuItem.modelData.folderId
                                            };
                                            folderMenuItem.Drag.active = true;
                                        });
                                    } else {
                                        folderMenuItem.Drag.active = false;
                                    }
                                }
                            }

                            onCheckedChanged: {
                                if (folderMenuItem.checked) {
                                    var sub = page.collectFolderSubtree(modelData.folderId);
                                    gameView.setScopeFolder(modelData.folderId, modelData.displayName, [
                                        {
                                            label: modelData.displayName,
                                            folderId: modelData.folderId
                                        }
                                    ], modelData.sortRole, modelData.sortAscending, sub.manual, sub.smart, modelData.icon1x1SourceUrl, modelData.folderType, modelData.color, modelData.description);
                                }
                            }

                            // Folder drops: top/bottom edge reorders next to this row,
                            // middle nests into it (cycle-guarded). Game drops add to
                            // this folder (manual folders only). dropZone tracks the
                            // hover position live so the row can show nest vs reorder
                            DropArea {
                                id: dropArea

                                anchors.fill: parent

                                // "top"/"bottom" = reorder before/after, "into" = nest,
                                // "none" = not hovering
                                property string dropZone: "none"

                                function updateZone(drag) {
                                    if (drag.formats.indexOf("application/x-fl-folder") === -1) {
                                        dropZone = "into";
                                        return;
                                    }
                                    var h = dropArea.height;
                                    dropZone = drag.y < h * 0.30 ? "top" : drag.y > h * 0.70 ? "bottom" : "into";
                                }

                                onEntered: function (drag) {
                                    updateZone(drag);
                                }
                                onPositionChanged: function (drag) {
                                    updateZone(drag);
                                }
                                onExited: dropZone = "none"

                                onDropped: function (event) {
                                    var zone = dropArea.dropZone;
                                    dropArea.dropZone = "none";
                                    if (event.formats.indexOf("application/x-fl-folder") !== -1) {
                                        var draggedId = parseInt(event.getDataAsString("application/x-fl-folder"));
                                        if (draggedId === modelData.folderId) {
                                            return;
                                        }
                                        var sub = page.collectFolderSubtree(draggedId);
                                        if (sub.manual.concat(sub.smart).indexOf(modelData.folderId) !== -1) {
                                            return;
                                        }
                                        if (zone === "top") {
                                            page.reorderFolderTo(draggedId, modelData.folderId, true);
                                        } else if (zone === "bottom") {
                                            page.reorderFolderTo(draggedId, modelData.folderId, false);
                                        } else if (modelData.folderType !== 1) {
                                            LibraryFolderModel.setFolderParent(draggedId, modelData.folderId);
                                        }
                                    } else if (modelData.folderType !== 1) {
                                        LibraryEntryModel.addEntryToFolder(event.text, modelData.folderId);
                                    }
                                }
                            }

                            // Insertion indicator when reordering (nesting uses the
                            // row highlight instead)
                            Rectangle {
                                z: 3
                                visible: dropArea.dropZone === "top"
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: parent.top
                                height: 2
                                color: Theme.accent
                            }
                            Rectangle {
                                z: 3
                                visible: dropArea.dropZone === "bottom"
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.bottom: parent.bottom
                                height: 2
                                color: Theme.accent
                            }
                        }
                    }

                    LibraryNavigationMenu {
                        id: platformMenuSection
                        Layout.fillWidth: true
                        title: "Platforms"
                        model: ownedPlatformsModel
                        focus: true

                        KeyNavigation.up: folderMenuSection.collapsed ? libraryMenuSection : folderMenuSection

                        onActiveFocusChanged: {
                            if (activeFocus) {
                                folderMenuSection.currentIndex = folderMenuSection.count - 1;
                                libraryMenuSection.currentIndex = libraryMenuSection.count - 1;
                            }
                        }

                        delegate: LibraryNavigationMenuItem {
                            id: platformMenuItem
                            required property var model

                            platformId: model.platformId
                            displayText: model.displayName
                            numberOfItems: LibraryEntryModel.countByPlatform[model.platformId] ?? 0

                            width: ListView.view.width
                            ButtonGroup.group: libraryButtonGroup

                            onCheckedChanged: {
                                if (platformMenuItem.checked) {
                                    gameView.setScopePlatform(model.platformId, model.displayName);
                                }
                            }
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        Layout.leftMargin: AppStyle.spacingLg
                        Layout.topMargin: 2
                        Layout.bottomMargin: AppStyle.spacingSm
                        visible: !platformMenuSection.collapsed
                        text: page.showAllPlatforms ? qsTr("Show fewer") : qsTr("Show all platforms")
                        color: Theme.textMuted
                        font.family: AppStyle.fontFamily
                        font.pixelSize: AppStyle.fontSizeSmall
                        font.weight: Font.DemiBold
                        TapHandler {
                            onTapped: page.showAllPlatforms = !page.showAllPlatforms
                        }
                        HoverHandler {
                            cursorShape: Qt.PointingHandCursor
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
