import QtQuick
import QtQml
import QtQuick.Controls
import QtQuick.Layouts 1.0
import "folder_tree.js" as FolderTree

SplitView {
    id: splitView
    orientation: Qt.Horizontal
    spacing: 0

    // clip: true

    // Collapse the sidebar to an icon-only rail to reclaim grid width
    property bool sidebarCollapsed: false

    // FolderDialog {
    //     id: folderDialog
    // }

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
            // property var counts: LibraryEntryModel.countByPlatform
            // onCountsChanged: invalidate()
            // data must be typed: this proxy exposes model roles only through a
            // declared parameter type, so an untyped `data` leaves platformId
            // undefined and filters every platform out
            function filter(data: PlatformRoleData): bool {
                return true;
                // if (!data) {
                //     return false;
                // }
                // return (counts[data.platformId] || 0) > 0;
            }
        }
    }

    component PlatformRoleData: QtObject {
        property int platformId
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
                description: o.description,
                filterJson: o.filterJson,
                color: o.color,
                folderType: o.folderType,
                icon1x1SourceUrl: o.icon1x1SourceUrl,
                sortRole: o.sortRole,
                sortAscending: o.sortAscending
            });
        }
        splitView.folderRows = FolderTree.flatten(all, splitView.expandedFolders, false);
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
            required property string description
            required property string filterJson
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
            width: AppStyle.spacingSm
            x: -AppStyle.spacingXs

            HoverHandler {
                id: handleHoverHandler
            }
        }

        Rectangle {
            anchors.centerIn: parent
            color: Theme.borderStrong
            height: parent.height - AppStyle.spacingLg
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

    // LibrarySidebar {
    //     page: splitView
    //     gameView: gameView
    //     ownedPlatformsModel: ownedPlatformsModel
    //     folderDialog: folderDialog
    // }

    Pane {
        id: gamesPanel

        SplitView.fillHeight: true
        SplitView.fillWidth: true
        bottomPadding: 0
        horizontalPadding: 0
        topPadding: 0
        verticalPadding: 0

        background: Item {}

        contentItem: GameView {
            id: gameView
        }
    }
}
