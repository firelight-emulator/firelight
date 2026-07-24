import QtQuick
import QtQml
import QtQuick.Controls
import QtQuick.Layouts 1.0

Item {
    id: root

    // --- Scope (the base set — one at a time, chosen in the sidebar) ---
    property int filterPlatformId: -1
    // A folder scope matches the folder AND all its descendants. The sidebar
    // supplies the descendant ids split by kind (manual = membership, smart =
    // computed), so selecting a parent shows everything nested under it
    property int filterFolderId: -1
    property var filterManualIds: []
    property var filterSmartIds: []
    // Human label + breadcrumb for the current scope, supplied by the sidebar
    property string scopeLabel: "All games"
    property var scopeCrumb: []
    // TODO
    // Icon + description for the current scope, drawn in the header identity block.
    // scopePlatformId >= 0 draws the platform logo; else custom art (scopeIconUrl)
    // when set, else the scopeIconName glyph
    property string scopeIconUrl: ""
    property string scopeIconName: "browse"
    property color scopeIconColor: Theme.textPrimary
    property int scopePlatformId: -1
    property string scopeDescription: ""

    // --- Refine (stack on top of the scope; the proxy ANDs them) ---
    property bool showOnlyFavorites: false
    property bool showOnlyUnplayed: false
    property string filterText: ""
    // Advanced refine filters (from the Filters popup)
    property bool filterHasAchievements: false
    property bool filterCompleted: false
    property string filterPlayTime: "any" // any / never / short / medium / long
    property int filterDecade: 0          // 0 = any; else the decade's start year
    property string filterGenre: ""

    readonly property var playTimeOptions: [
        {
            label: "Any play time",
            value: "any"
        },
        {
            label: "Never played",
            value: "never"
        },
        {
            label: "Under 1 hour",
            value: "short"
        },
        {
            label: "1–10 hours",
            value: "medium"
        },
        {
            label: "Over 10 hours",
            value: "long"
        }
    ]
    readonly property var decadeOptions: [
        {
            label: "Any decade",
            value: 0
        },
        {
            label: "1970s",
            value: 1970
        },
        {
            label: "1980s",
            value: 1980
        },
        {
            label: "1990s",
            value: 1990
        },
        {
            label: "2000s",
            value: 2000
        },
        {
            label: "2010s",
            value: 2010
        },
        {
            label: "2020s",
            value: 2020
        }
    ]
    readonly property bool anyAdvancedFilter: filterHasAchievements || filterCompleted || filterPlayTime !== "any" || filterDecade !== 0 || filterGenre.trim().length > 0
    // TODO
    // Every filter reachable from the Filters popup (advanced + the favorites/unplayed toggles)
    readonly property bool anyPopupFilter: anyAdvancedFilter || showOnlyFavorites || showOnlyUnplayed
    function playTimeLabel(v) {
        for (var i = 0; i < playTimeOptions.length; i++) {
            if (playTimeOptions[i].value === v) {
                return playTimeOptions[i].label;
            }
        }
        return "";
    }
    function clearAdvancedFilters() {
        filterHasAchievements = false;
        filterCompleted = false;
        filterPlayTime = "any";
        filterDecade = 0;
        filterGenre = "";
    }
    // TODO
    // Clears every filter reachable from the Filters popup
    function clearPopupFilters() {
        clearAdvancedFilters();
        showOnlyFavorites = false;
        showOnlyUnplayed = false;
    }

    // --- Sort / view ---
    property string sortRole: "displayName"
    property bool sortAscending: true
    property string viewMode: "grid"
    readonly property int gameCount: gameMirror.count

    // --- Multi-select (drives bulk actions; independent of scope/refine) ---
    // entryId -> true. Reassigned wholesale on every change so bindings refresh
    property var selectedIds: ({})
    property int selectedCount: 0
    property int selectionAnchorRow: -1

    // The scoped folder, but only when it's a manual folder we can remove from
    // (a smart folder's membership is computed, so "remove" is meaningless)
    readonly property int removableFolderId: filterManualIds.indexOf(filterFolderId) !== -1 ? filterFolderId : -1

    readonly property var sortOptions: [
        {
            label: "Name",
            role: "displayName"
        },
        {
            label: "Last played",
            role: "lastPlayedAt"
        },
        {
            label: "Playtime",
            role: "numSecondsPlayed"
        },
        {
            label: "Achievements",
            role: "achievementsEarned"
        },
        {
            label: "Date added",
            role: "createdAt"
        }
    ]

    // TODO
    // The active sort role's label, shown on the Display button
    readonly property string currentSortLabel: {
        for (var i = 0; i < sortOptions.length; i++) {
            if (sortOptions[i].role === sortRole) {
                return sortOptions[i].label;
            }
        }
        return "";
    }

    readonly property bool isDirty: filterPlatformId !== -1 || filterFolderId !== -1 || showOnlyFavorites || showOnlyUnplayed || filterText !== "" || anyAdvancedFilter

    signal folderCrumbClicked(int folderId)

    property bool _applyingFolderSort: false

    function applyFolderSort(sr, asc) {
        if (sr && sr.length > 0) {
            _applyingFolderSort = true;
            root.sortRole = sr;
            root.sortAscending = asc;
            _applyingFolderSort = false;
        }
    }

    function persistFolderSort() {
        if (!_applyingFolderSort && filterFolderId !== -1) {
            LibraryFolderModel.setFolderSort(filterFolderId, sortRole, sortAscending);
        }
    }

    // The sidebar picks a scope; it clears only the other scope axis, never the
    // refine toggles — so "SNES" + "Favorites" combine instead of replacing
    function setScopeAll() {
        filterPlatformId = -1;
        filterFolderId = -1;
        filterManualIds = [];
        filterSmartIds = [];
        scopeLabel = "All games";
        scopeCrumb = [];
        scopeIconUrl = "";
        scopeIconName = "browse";
        scopeIconColor = Theme.textPrimary;
        scopePlatformId = -1;
        scopeDescription = "";
        clearSelection();
    }

    function setScopePlatform(platformId, label) {
        filterFolderId = -1;
        filterManualIds = [];
        filterSmartIds = [];
        filterPlatformId = platformId;
        scopeLabel = label;
        scopeCrumb = [];
        scopeIconUrl = "";
        scopeIconName = "";
        scopeIconColor = Theme.textPrimary;
        scopePlatformId = platformId;
        scopeDescription = "";
        clearSelection();
    }

    function setScopeFolder(folderId, label, crumb, sr, asc, manualIds, smartIds, iconUrl, folderType, color, description) {
        filterPlatformId = -1;
        filterFolderId = folderId;
        filterManualIds = manualIds ? manualIds : [];
        filterSmartIds = smartIds ? smartIds : [];
        scopeLabel = label;
        scopeCrumb = crumb ? crumb : [];
        scopeIconUrl = iconUrl ? iconUrl : "";
        scopeIconName = folderType === 1 ? "bookmark-star" : "folder";
        scopeIconColor = color && color !== "" ? color : Theme.textPrimary;
        scopePlatformId = -1;
        scopeDescription = description ? description : "";
        applyFolderSort(sr, asc);
        clearSelection();
    }

    function clearAll() {
        setScopeAll();
        showOnlyFavorites = false;
        showOnlyUnplayed = false;
        filterText = "";
        clearAdvancedFilters();
    }

    function openAddToFolder() {
        addToFolderDialog.openFor(selectedIdList());
    }

    // --- Selection helpers ---
    function _selectionRecount() {
        var n = 0;
        for (var k in root.selectedIds) {
            if (root.selectedIds[k]) {
                n++;
            }
        }
        root.selectedCount = n;
    }
    function isSelected(entryId) {
        return root.selectedIds[entryId] === true;
    }
    function selectedIdList() {
        var out = [];
        for (var k in root.selectedIds) {
            if (root.selectedIds[k]) {
                out.push(parseInt(k));
            }
        }
        return out;
    }
    function selectOnly(entryId, rowIndex) {
        var s = {};
        s[entryId] = true;
        root.selectedIds = s;
        root.selectionAnchorRow = rowIndex;
        root._selectionRecount();
    }
    function toggleSelect(entryId, rowIndex) {
        var s = {};
        for (var k in root.selectedIds) {
            if (root.selectedIds[k]) {
                s[k] = true;
            }
        }
        if (s[entryId]) {
            delete s[entryId];
        } else
            s[entryId] = true;
        root.selectedIds = s;
        root.selectionAnchorRow = rowIndex;
        root._selectionRecount();
    }
    function rangeSelectTo(rowIndex) {
        if (root.selectionAnchorRow < 0) {
            root.selectionAnchorRow = rowIndex;
        }
        var lo = Math.min(root.selectionAnchorRow, rowIndex);
        var hi = Math.max(root.selectionAnchorRow, rowIndex);
        var s = {};
        for (var i = lo; i <= hi && i < gameMirror.count; i++) {
            var o = gameMirror.objectAt(i);
            if (o) {
                s[o.entryId] = true;
            }
        }
        root.selectedIds = s;
        root._selectionRecount();
    }
    function clearSelection() {
        root.selectedIds = ({});
        root.selectedCount = 0;
        root.selectionAnchorRow = -1;
    }
    // Modifier-aware click from a tile / row
    function handleGameClick(entryId, rowIndex, modifiers) {
        if (modifiers & Qt.ShiftModifier) {
            root.rangeSelectTo(rowIndex);
        } else if (modifiers & Qt.ControlModifier) {
            root.toggleSelect(entryId, rowIndex);
        } else
            root.selectOnly(entryId, rowIndex);
    }
    function bulkFavorite(fav) {
        var t = root.selectedIdList();
        for (var i = 0; i < t.length; i++) {
            LibraryEntryModel.setEntryFavorite(t[i], fav);
        }
    }
    function bulkRemoveFromFolder() {
        if (root.removableFolderId === -1) {
            return;
        }
        var t = root.selectedIdList();
        for (var i = 0; i < t.length; i++) {
            LibraryEntryModel.removeEntryFromFolder(t[i], root.removableFolderId);
        }
    }

    // --- Detail panel (right dock) ---
    property var detailData: null
    property bool detailOpen: false
    // Favoriting from the panel: persist + keep the snapshot in sync (reassign a
    // copy so the panel's bindings refresh)
    function setDetailFavorite(entryId, fav) {
        LibraryEntryModel.setEntryFavorite(entryId, fav);
        if (root.detailData && root.detailData.entryId === entryId) {
            var d = Object.assign({}, root.detailData);
            d.favorite = fav;
            root.detailData = d;
        }
    }

    SortFilterProxyModel {
        id: gameModel
        model: LibraryEntryModel

        filters: [
            ValueFilter {
                roleName: "favorite"
                value: true
                enabled: root.showOnlyFavorites
            },
            ValueFilter {
                roleName: "lastPlayedAt"
                value: 0
                enabled: root.showOnlyUnplayed
            },
            ValueFilter {
                roleName: "platformId"
                value: root.filterPlatformId
                enabled: root.filterPlatformId !== -1
            },
            FunctionFilter {
                property var manualIds: root.filterManualIds
                property var smartIds: root.filterSmartIds
                enabled: root.filterFolderId !== -1
                function filter(data: RoleData): bool {
                    for (var i = 0; i < manualIds.length; i++) {
                        if (data.folderIds.includes(manualIds[i])) {
                            return true;
                        }
                    }
                    for (var j = 0; j < smartIds.length; j++) {
                        if (LibraryEntryModel.matchesSmartFolder(smartIds[j], data.entryId)) {
                            return true;
                        }
                    }
                    return false;
                }
                onManualIdsChanged: invalidate()
                onSmartIdsChanged: invalidate()
            },
            FunctionFilter {
                property string filterText: root.filterText
                function filter(data: RoleData): bool {
                    return data.displayName.toLowerCase().indexOf(root.filterText.toLowerCase()) !== -1;
                }
                onFilterTextChanged: invalidate()
            },
            FunctionFilter {
                property bool on: root.filterHasAchievements
                enabled: root.filterHasAchievements
                function filter(data: RoleData): bool {
                    return data.achievementsTotal > 0;
                }
                onOnChanged: invalidate()
            },
            FunctionFilter {
                property bool on: root.filterCompleted
                enabled: root.filterCompleted
                function filter(data: RoleData): bool {
                    return data.achievementsTotal > 0 && data.achievementsEarned >= data.achievementsTotal;
                }
                onOnChanged: invalidate()
            },
            FunctionFilter {
                property string bucket: root.filterPlayTime
                enabled: root.filterPlayTime !== "any"
                function filter(data: RoleData): bool {
                    var s = data.numSecondsPlayed;
                    if (bucket === "never") {
                        return !s || s <= 0;
                    }
                    if (bucket === "short") {
                        return s > 0 && s < 3600;
                    }
                    if (bucket === "medium") {
                        return s >= 3600 && s < 36000;
                    }
                    if (bucket === "long") {
                        return s >= 36000;
                    }
                    return true;
                }
                onBucketChanged: invalidate()
            },
            FunctionFilter {
                property int decade: root.filterDecade
                enabled: root.filterDecade !== 0
                function filter(data: RoleData): bool {
                    return data.releaseYear >= decade && data.releaseYear < decade + 10;
                }
                onDecadeChanged: invalidate()
            },
            FunctionFilter {
                property string g: root.filterGenre
                enabled: root.filterGenre.trim().length > 0
                function filter(data: RoleData): bool {
                    return data.genres.toLowerCase().indexOf(g.toLowerCase().trim()) !== -1;
                }
                onGChanged: invalidate()
            }
        ]
        sorters: [
            RoleSorter {
                roleName: root.sortRole
                sortOrder: root.sortAscending ? Qt.AscendingOrder : Qt.DescendingOrder
            }
        ]
    }

    // A lightweight, ordered mirror of the filtered/sorted rows. Qt's tech-preview
    // SortFilterProxyModel exposes neither get(row) nor count, so this Instantiator
    // is how QML reads the result set in order — it backs the live count and
    // shift-range selection (and later, group-by sections)
    Instantiator {
        id: gameMirror
        model: gameModel
        delegate: QtObject {
            required property int entryId
        }
    }

    // Shared, single-instance dialogs raised by the grid/list context menus and
    // the bulk bar
    GameArtPickerDialog {
        id: artPicker
    }
    AddToFolderDialog {
        id: addToFolderDialog
    }

    ColumnLayout {
        id: mainColumn
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: detailDock.left
        spacing: 0

        // ---- Header / toolbar ----
        ColumnLayout {
            Layout.fillWidth: true
            Layout.leftMargin: AppStyle.spacingLg
            Layout.rightMargin: AppStyle.spacingLg
            Layout.topMargin: AppStyle.spacingSm
            Layout.bottomMargin: AppStyle.spacingSm
            spacing: AppStyle.spacingSm

            GameViewHeader {
                Layout.fillWidth: true
                view: root
            }

            // Toolbar: search, quick filters, Filters/Display popups, details toggle
            GameToolbar {
                Layout.fillWidth: true
                view: root
            }

            // Active advanced-filter chips (removable)
            Flow {
                Layout.fillWidth: true
                visible: root.anyAdvancedFilter
                spacing: AppStyle.spacingXs

                RefineChip {
                    visible: root.filterHasAchievements
                    label: "Has achievements"
                    onCleared: root.filterHasAchievements = false
                }
                RefineChip {
                    visible: root.filterCompleted
                    label: "Completed"
                    onCleared: root.filterCompleted = false
                }
                RefineChip {
                    visible: root.filterPlayTime !== "any"
                    label: root.playTimeLabel(root.filterPlayTime)
                    onCleared: root.filterPlayTime = "any"
                }
                RefineChip {
                    visible: root.filterDecade !== 0
                    label: root.filterDecade + "s"
                    onCleared: root.filterDecade = 0
                }
                RefineChip {
                    visible: root.filterGenre.trim().length > 0
                    label: "Genre: " + root.filterGenre
                    onCleared: root.filterGenre = ""
                }
            }
        }

        FLDivider {}

        Loader {
            id: viewLoader
            clip: true
            Layout.fillHeight: true
            Layout.fillWidth: true
            sourceComponent: root.viewMode === "grid" ? gridView : listView
        }
    }

    // Right dock: details for the last-clicked game. Width animates 0 -> open so
    // the grid reflows to make room; the panel keeps a fixed content width and is
    // clipped, so it slides rather than reflowing while animating
    Item {
        id: detailDock
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        readonly property int openWidth: AppStyle.detailPanelWidth
        width: root.detailOpen ? openWidth : 0
        clip: true
        Behavior on width {
            NumberAnimation {
                duration: 180
                easing.type: Easing.InOutQuad
            }
        }

        GameDetailPanel {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: detailDock.openWidth
            gameData: root.detailData
            onRequestPlay: entryId => EmulationService.loadEntry(entryId)
            onRequestFavorite: (entryId, fav) => root.setDetailFavorite(entryId, fav)
            onRequestAddToFolder: entryId => addToFolderDialog.openFor([entryId])
        }
    }

    Component {
        id: listView
        GameListView {
            model: gameModel
            selectedIds: root.selectedIds
            onSortRoleChanged: {
                root.sortRole = sortRole;
                root.persistFolderSort();
            }
            onSortAscendingChanged: {
                root.sortAscending = sortAscending;
                root.persistFolderSort();
            }
            onGameClicked: (entryId, rowIndex, modifiers) => root.handleGameClick(entryId, rowIndex, modifiers)
            onGameFocused: data => root.detailData = data
            onRequestAddToFolder: entryIds => addToFolderDialog.openFor(entryIds)
            onRequestChangeArt: (hash, name, platformId) => artPicker.openFor(hash, name, platformId)
        }
    }

    Component {
        id: gridView
        GameGridView {
            model: gameModel
            selectedIds: root.selectedIds
            onGameClicked: (entryId, rowIndex, modifiers) => root.handleGameClick(entryId, rowIndex, modifiers)
            onGameFocused: data => root.detailData = data
            onRequestAddToFolder: entryIds => addToFolderDialog.openFor(entryIds)
            onRequestChangeArt: (hash, name, platformId) => artPicker.openFor(hash, name, platformId)
        }
    }

    component RoleData: QtObject {
        property int entryId
        property string displayName
        property list<int> folderIds
        property int achievementsEarned
        property int achievementsTotal
        property var numSecondsPlayed
        property int releaseYear
        property string genres
    }

    // A removable pill for an active refine filter
    component RefineChip: Rectangle {
        id: chip
        property string label: ""
        signal cleared
        implicitHeight: Math.round(24 * AppStyle.scale)
        implicitWidth: chipRow.implicitWidth + AppStyle.spacingMd
        radius: implicitHeight / 2
        color: Theme.surfaceElevated
        border.color: Theme.border
        border.width: 1

        RowLayout {
            id: chipRow
            anchors.centerIn: parent
            spacing: AppStyle.spacingXs
            Text {
                text: chip.label
                color: Theme.textPrimary
                font.family: AppStyle.fontFamily
                font.pixelSize: AppStyle.fontSizeSmall
            }
            Item {
                implicitWidth: AppStyle.minTarget
                implicitHeight: AppStyle.minTarget
                Icon {
                    anchors.centerIn: parent
                    name: "close"
                    size: AppStyle.iconSizeSm
                    color: Theme.textMuted
                }
                TapHandler {
                    onTapped: chip.cleared()
                }
                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }
            }
        }
    }
}
