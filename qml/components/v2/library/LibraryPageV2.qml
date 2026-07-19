import QtQuick
import QtQml
import QtQuick.Controls
import QtQuick.Layouts 1.0

SplitView {
    id: splitView
    orientation: Qt.Horizontal
    spacing: 0

    clip: true

    // Collapse the sidebar to an icon-only rail to reclaim grid width
    property bool sidebarCollapsed: false

    CreateFolderDialog {
        id: createFolderDialog
        property int targetParentId: -1
        onAccepted: LibraryFolderModel.createFolder(createFolderDialog.folderName, createFolderDialog.targetParentId)
    }

    CreateFolderDialog {
        id: renameFolderDialog
        property int targetFolderId: -1
        headerText: "Rename folder"
        onAccepted: LibraryFolderModel.setFolderName(renameFolderDialog.targetFolderId, renameFolderDialog.folderName)
    }

    SmartFolderDialog {
        id: smartFolderDialog
    }

    // Owned platforms first: hide platforms with no games until "Show all"
    property bool showAllPlatforms: false
    SortFilterProxyModel {
        id: ownedPlatformsModel
        model: PlatformModel
        filters: FunctionFilter {
            enabled: !splitView.showAllPlatforms
            // Counts populate after entries finish loading; binding the map here
            // re-invalidates the filter when it changes, so owned platforms
            // appear once the library is ready instead of staying empty
            property var counts: LibraryEntryModel.countByPlatform
            onCountsChanged: invalidate()
            function filter(data): bool {
                return (counts[data.platformId] || 0) > 0;
            }
        }
    }

    // Folder tree: the folder model is flat, so flatten its parentId/position
    // into a pre-ordered, depth-tagged list gated by an expanded set. Collapsed
    // folders (id present with value false) hide their descendants
    property var expandedFolders: ({})
    property var folderRows: []

    function toggleFolderExpanded(folderId) {
        var e = splitView.expandedFolders;
        e[folderId] = (e[folderId] === false);
        splitView.expandedFolders = e;
        rebuildFolderTree();
    }

    function rebuildFolderTree() {
        var all = [];
        for (var i = 0; i < folderCollector.count; i++) {
            var o = folderCollector.objectAt(i);
            if (!o) {
                continue;
            }
            all.push({
                folderId: o.folderId,
                parentId: o.parentId,
                position: o.position,
                displayName: o.displayName,
                color: o.color,
                folderType: o.folderType,
                icon1x1SourceUrl: o.icon1x1SourceUrl,
                sortRole: o.sortRole,
                sortAscending: o.sortAscending
            });
        }
        var byParent = {};
        all.forEach(function (f) {
            (byParent[f.parentId] = byParent[f.parentId] || []).push(f);
        });
        Object.keys(byParent).forEach(function (k) {
            byParent[k].sort(function (a, b) {
                return a.position - b.position;
            });
        });
        var out = [];
        function walk(pid, depth) {
            (byParent[pid] || []).forEach(function (f) {
                var kids = byParent[f.folderId] || [];
                var expanded = splitView.expandedFolders[f.folderId] !== false;
                var row = {};
                for (var key in f) {
                    row[key] = f[key];
                }
                row.depth = depth;
                row.hasChildren = kids.length > 0;
                row.expanded = expanded;
                out.push(row);
                if (expanded) {
                    walk(f.folderId, depth + 1);
                }
            });
        }
        walk(-1, 0);
        splitView.folderRows = out;
    }

    // A folder + every folder nested under it, split by kind (manual vs smart),
    // so selecting a parent scope shows all of its descendants' games too
    function collectFolderSubtree(folderId) {
        var kids = {};
        var typeOf = {};
        for (var i = 0; i < folderCollector.count; i++) {
            var o = folderCollector.objectAt(i);
            if (!o) {
                continue;
            }
            (kids[o.parentId] = kids[o.parentId] || []).push(o.folderId);
            typeOf[o.folderId] = o.folderType;
        }
        var manual = [], smart = [];
        function walk(id) {
            if (typeOf[id] === 1) {
                smart.push(id);
            } else
                manual.push(id);
            (kids[id] || []).forEach(walk);
        }
        walk(folderId);
        return {
            manual: manual,
            smart: smart
        };
    }

    // Moves `draggedId` next to `targetFolderId` (into the target's parent scope,
    // just before or after it) and renumbers that scope's positions
    function reorderFolderTo(draggedId, targetFolderId, before) {
        var parentOf = {};
        var byParent = {};
        for (var i = 0; i < folderCollector.count; i++) {
            var o = folderCollector.objectAt(i);
            if (!o) {
                continue;
            }
            parentOf[o.folderId] = o.parentId;
            (byParent[o.parentId] = byParent[o.parentId] || []).push({
                id: o.folderId,
                pos: o.position
            });
        }
        var targetParent = parentOf[targetFolderId];
        if (targetParent === undefined) {
            return;
        }
        if (parentOf[draggedId] !== targetParent) {
            LibraryFolderModel.setFolderParent(draggedId, targetParent);
        }
        var sibs = (byParent[targetParent] || []).slice().sort(function (a, b) {
            return a.pos - b.pos;
        }).map(function (s) {
            return s.id;
        }).filter(function (id) {
            return id !== draggedId;
        });
        var idx = sibs.indexOf(targetFolderId);
        if (idx === -1) {
            idx = sibs.length - 1;
        }
        sibs.splice(before ? idx : idx + 1, 0, draggedId);
        LibraryFolderModel.reorderFolders(targetParent, sibs);
    }

    Instantiator {
        id: folderCollector
        model: LibraryFolderModel
        delegate: QtObject {
            required property int folderId
            required property int parentId
            required property int position
            required property string displayName
            required property string color
            required property int folderType
            required property string icon1x1SourceUrl
            required property string sortRole
            required property bool sortAscending
        }
        onObjectAdded: Qt.callLater(splitView.rebuildFolderTree)
        onObjectRemoved: Qt.callLater(splitView.rebuildFolderTree)
    }
    Connections {
        target: LibraryFolderModel
        function onDataChanged() {
            Qt.callLater(splitView.rebuildFolderTree);
        }
        function onModelReset() {
            Qt.callLater(splitView.rebuildFolderTree);
        }
        function onLayoutChanged() {
            Qt.callLater(splitView.rebuildFolderTree);
        }
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

        readonly property int fullWidth: Math.round(280 * AppStyle.scale)
        readonly property int railWidth: Math.round(56 * AppStyle.scale)
        // Animating a local property (not SplitView.preferredWidth directly, which
        // can't take a Behavior) makes the collapse glide
        property real animatedWidth: splitView.sidebarCollapsed ? railWidth : fullWidth
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
            color: Theme.glass
            topLeftRadius: 8
            bottomLeftRadius: 8
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
                            iconName: splitView.sidebarCollapsed ? "chevron-forward" : "chevron-back"
                            size: "sm"
                            tooltipText: splitView.sidebarCollapsed ? "Expand sidebar" : "Collapse sidebar"
                            Layout.alignment: Qt.AlignVCenter
                            onClicked: splitView.sidebarCollapsed = !splitView.sidebarCollapsed
                        }

                        Text {
                            visible: !splitView.sidebarCollapsed
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
                            visible: !splitView.sidebarCollapsed
                            Layout.fillWidth: true
                            implicitHeight: 1
                        }

                        FLIconButton {
                            visible: !splitView.sidebarCollapsed
                            iconName: "add"
                            Layout.topMargin: 2
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                            onClicked: newFolderMenu.open()

                            RightClickMenu {
                                id: newFolderMenu

                                RightClickMenuItem {
                                    text: "New folder"
                                    onTriggered: {
                                        createFolderDialog.targetParentId = -1;
                                        createFolderDialog.open();
                                    }
                                }
                                RightClickMenuItem {
                                    text: "New smart folder"
                                    onTriggered: {
                                        smartFolderDialog.editFolderId = -1;
                                        smartFolderDialog.open();
                                    }
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

                            KeyNavigation.down: platformMenuSection.collapsed ? folderMenuSection.collapsed ? null : folderMenuSection : platformMenuSection

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

                        LibraryNavigationMenuSection {
                            id: platformMenuSection
                            Layout.fillWidth: true
                            title: "Platforms"
                            model: ownedPlatformsModel
                            focus: true

                            KeyNavigation.down: folderMenuSection.collapsed ? null : folderMenuSection

                            onActiveFocusChanged: {
                                if (activeFocus) {
                                    folderMenuSection.currentIndex = 0;
                                    libraryMenuSection.currentIndex = libraryMenuSection.count - 1;
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
                            text: splitView.showAllPlatforms ? qsTr("Show fewer") : qsTr("Show all platforms")
                            color: Theme.textMuted
                            font.family: Constants.regularFontFamily
                            font.pixelSize: AppStyle.fontSizeSmall
                            font.weight: Font.DemiBold
                            TapHandler {
                                onTapped: splitView.showAllPlatforms = !splitView.showAllPlatforms
                            }
                            HoverHandler {
                                cursorShape: Qt.PointingHandCursor
                            }
                        }

                        LibraryNavigationMenuSection {
                            id: folderMenuSection
                            Layout.fillWidth: true
                            title: "Folders"
                            model: splitView.folderRows

                            KeyNavigation.up: platformMenuSection.collapsed ? libraryMenuSection : platformMenuSection

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
                                onToggleExpanded: splitView.toggleFolderExpanded(modelData.folderId)

                                ContextMenu.menu: RightClickMenu {
                                    RightClickMenuItem {
                                        text: qsTr("New subfolder")
                                        onTriggered: {
                                            createFolderDialog.targetParentId = folderMenuItem.modelData.folderId;
                                            createFolderDialog.open();
                                        }
                                    }
                                    RightClickMenuItem {
                                        text: qsTr("Rename")
                                        onTriggered: {
                                            renameFolderDialog.targetFolderId = folderMenuItem.modelData.folderId;
                                            renameFolderDialog.initialText = folderMenuItem.modelData.displayName;
                                            renameFolderDialog.open();
                                        }
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
                                            onTriggered: colorMenu.apply("#e5484d")
                                        }
                                        RightClickMenuItem {
                                            text: qsTr("Orange")
                                            onTriggered: colorMenu.apply("#f76b15")
                                        }
                                        RightClickMenuItem {
                                            text: qsTr("Yellow")
                                            onTriggered: colorMenu.apply("#f5d90a")
                                        }
                                        RightClickMenuItem {
                                            text: qsTr("Green")
                                            onTriggered: colorMenu.apply("#46a758")
                                        }
                                        RightClickMenuItem {
                                            text: qsTr("Blue")
                                            onTriggered: colorMenu.apply("#0091ff")
                                        }
                                        RightClickMenuItem {
                                            text: qsTr("Purple")
                                            onTriggered: colorMenu.apply("#8e4ec6")
                                        }
                                        RightClickMenuItem {
                                            text: qsTr("Pink")
                                            onTriggered: colorMenu.apply("#e93d82")
                                        }
                                    }
                                    RightClickMenuItem {
                                        text: qsTr("Delete folder")
                                        dangerous: true
                                        onTriggered: LibraryFolderModel.deleteFolder(folderMenuItem.modelData.folderId)
                                    }
                                }

                                iconName: "folder"
                                iconColor: modelData.color !== "" ? modelData.color : Theme.textPrimary
                                displayText: modelData.displayName
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
                                        var sub = splitView.collectFolderSubtree(modelData.folderId);
                                        gameView.setScopeFolder(modelData.folderId, modelData.displayName, [
                                            {
                                                label: modelData.displayName,
                                                folderId: modelData.folderId
                                            }
                                        ], modelData.sortRole, modelData.sortAscending, sub.manual, sub.smart);
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
                                            var sub = splitView.collectFolderSubtree(draggedId);
                                            if (sub.manual.concat(sub.smart).indexOf(modelData.folderId) !== -1) {
                                                return;
                                            }
                                            if (zone === "top") {
                                                splitView.reorderFolderTo(draggedId, modelData.folderId, true);
                                            } else if (zone === "bottom") {
                                                splitView.reorderFolderTo(draggedId, modelData.folderId, false);
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
        horizontalPadding: 0 // 24
        topPadding: 0
        verticalPadding: 0
        // clip: true

        background: Rectangle {
            color: Theme.glassElevated
            topRightRadius: 8
            bottomRightRadius: 8
        }

        contentItem: GameView {
            id: gameView
        }
    }
}
