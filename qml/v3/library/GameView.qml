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

    // The advanced-filter bar under the toolbar is expanded
    property bool filtersExpanded: false


    property string _pendingSortRole: ""
    property string _pendingViewType: ""



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
    // Section grouping: "none" | "platform" | "decade" | "year" | "genre" | "title"
    property string groupBy: "none"
    readonly property int gameCount: gameMirror.count

    readonly property var groupOptions: [
        {
            label: "None",
            value: "none"
        },
        {
            label: "Platform",
            value: "platform"
        },
        {
            label: "Decade",
            value: "decade"
        },
        {
            label: "Year",
            value: "year"
        },
        {
            label: "Genre",
            value: "genre"
        },
        {
            label: "Title (A–Z)",
            value: "title"
        }
    ]

    // Drives the model's derived groupKey role that the views section on
    Binding {
        target: LibraryEntryModel
        property: "groupMode"
        value: root.groupBy
    }

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
            // Primary sort by group so sections stay contiguous. When grouping is
            // off the key is empty for every row, so this is a no-op and the
            // sort role below decides the order
            RoleSorter {
                roleName: "groupKey"
                sortOrder: Qt.AscendingOrder
            },
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
            required property string groupKey
        }
        onObjectAdded: Qt.callLater(root.rebuildGroupKeys)
        onObjectRemoved: Qt.callLater(root.rebuildGroupKeys)
    }

    // The distinct group-header labels in display order, derived from the sorted
    // mirror (the proxy is sorted by groupKey first). Feeds the grid's sectioned
    // layout; the list view uses native ListView.section instead
    property var groupKeys: []
    // key -> count, so the grid can give each section a known height up front and
    // the outer list only realizes on-screen sections (not all of them at once)
    property var groupCounts: ({})
    onGroupByChanged: Qt.callLater(root.rebuildGroupKeys)
    function rebuildGroupKeys() {
        if (root.groupBy === "none") {
            root.groupKeys = [];
            root.groupCounts = ({});
            return;
        }
        var keys = [];
        var counts = {};
        var last = null;
        for (var i = 0; i < gameMirror.count; i++) {
            var o = gameMirror.objectAt(i);
            if (!o) {
                continue;
            }
            var k = o.groupKey;
            counts[k] = (counts[k] || 0) + 1;
            if (k !== last) {
                keys.push(k);
                last = k;
            }
        }
        root.groupKeys = keys;
        root.groupCounts = counts;
    }

    // Shared, single-instance dialogs raised by the grid/list context menus and
    // the bulk bar
    // GameArtPickerDialog {
    //     id: artPicker
    // }
    // AddToFolderDialog {
    //     id: addToFolderDialog
    // }
    // FLGameEditDialog {
    //     id: editGameDialog
    // }

    FLColumnLayout {
        id: toolbarColumn
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: AppStyle.spacingMd
        anchors.bottom: parent.bottom
        width: 72
        spacing: AppStyle.spacingSm

        KeyNavigation.right: mainColumn

        FLIconButton {
            Layout.alignment: Qt.AlignHCenter
            iconName: "add"
            tooltipText: "Add or create"
            compact: false
            onClicked: addPopup.opened ? addPopup.close() : addPopup.open()

            // RightClickMenu {
            //     id: addPopup
            //     x: parent.width + AppStyle.spacingXs
            //     y: 0
            //
            //     RightClickMenuItem {
            //         text: "Add game"
            //         onTriggered: {
            //             // GameAddDialog.openForExisting();
            //         }
            //     }
            //
            //     RightClickMenuItem {
            //         text: "Create folder"
            //         onTriggered: {
            //             // GameAddDialog.openForExisting();
            //         }
            //     }
            //
            //     enter: Transition {
            //         NumberAnimation {
            //             property: "opacity"
            //             from: 0
            //             to: 1
            //             duration: AppStyle.durationFast
            //             easing.type: Easing.InOutQuad
            //         }
            //         NumberAnimation {
            //             property: "x"
            //             from: gameSortPopup.x - 8
            //             to: gameSortPopup.x
            //             duration: AppStyle.durationFast
            //             easing.type: Easing.InOutQuad
            //         }
            //     }
            // }
        }

        FLIconButton {
            Layout.alignment: Qt.AlignHCenter
            iconName: "search"
            tooltipText: "Search"
            compact: false
        }

        FLIconButton {
            id: filterButton
            Layout.alignment: Qt.AlignHCenter
            iconName: "filter"
            tooltipText: "Filter"
            compact: false
        }

        FLIconButton {
            id: sortButton
            Layout.alignment: Qt.AlignHCenter
            iconName: "sort"
            tooltipText: "Sort"
            compact: false
            onClicked: gameSortPopup.opened ? gameSortPopup.close() : gameSortPopup.open()

            FLPopup {
                id: gameSortPopup
                x: sortButton.width + AppStyle.spacingXs
                minWidth: 240

                // TODO
                // The choice shown in the list, which leads the applied sort by
                // the confirm beat
                property string chosenRole: root.sortRole

                onAboutToShow: gameSortPopup.chosenRole = root.sortRole

                // TODO
                // The grid transition starts once the popup is gone, so the two
                // motions read as one sequence instead of overlapping
                onClosed: {
                    // FocusCursor.blink(AppStyle.durationSlow)
                    confirmTimer.stop();
                    if (gameSortPopup.chosenRole !== root.sortRole) {
                        root._pendingSortRole = gameSortPopup.chosenRole;
                    } else {
                        FocusCursor.blink(AppStyle.durationBase);
                    }
                }

                Timer {
                    id: confirmTimer
                    interval: InputMethodManager.usingMouse ? 0 : AppStyle.confirmPause
                    onTriggered: gameSortPopup.close()
                }

                contentItem: FLRadioGroup {
                    model: root.sortOptions
                    valueRole: "role"
                    currentValue: gameSortPopup.chosenRole
                    onActivated: value => {
                        gameSortPopup.chosenRole = value;
                        confirmTimer.restart();
                    }
                }
            }

            // GameSortPopup {
            //     id: gameSortPopup
            //     y: 0
            //     x: sortButton.width + AppStyle.spacingXs
            //     view: root
            //
            //     enter: Transition {
            //         NumberAnimation {
            //             property: "opacity"
            //             from: 0
            //             to: 1
            //             duration: AppStyle.durationFast
            //             easing.type: Easing.InOutQuad
            //         }
            //         NumberAnimation {
            //             property: "x"
            //             from: gameSortPopup.x - 8
            //             to: gameSortPopup.x
            //             duration: AppStyle.durationFast
            //             easing.type: Easing.InOutQuad
            //         }
            //     }
            // }
        }

        FLIconButton {
            id: viewAsButton
            Layout.alignment: Qt.AlignHCenter
            iconName: root.viewMode === "grid" ? "grid_view" : "view_list"
            tooltipText: "View as"
            compact: false
            onClicked: displayPopup.opened ? displayPopup.close() : displayPopup.open()

            FLPopup {
                id: displayPopup
                x: viewAsButton.width + AppStyle.spacingXs
                minWidth: 240

                // TODO
                // The choice shown in the list, which leads the applied sort by
                // the confirm beat
                property string chosenViewMode: root.viewMode

                onAboutToShow: displayPopup.chosenViewMode = root.viewMode

                // TODO
                // The grid transition starts once the popup is gone, so the two
                // motions read as one sequence instead of overlapping
                onClosed: {
                    // FocusCursor.blink(AppStyle.durationSlow)
                    viewModeConfirmTimer.stop();
                    if (displayPopup.chosenViewMode !== root.viewMode) {
                        root._pendingViewType = displayPopup.chosenViewMode;
                    } else {
                        FocusCursor.blink(AppStyle.durationBase);
                    }
                }

                Timer {
                    id: viewModeConfirmTimer
                    interval: InputMethodManager.usingMouse ? 0 : AppStyle.confirmPause
                    onTriggered: displayPopup.close()
                }

                contentItem: FLRadioGroup {
                    model: [{
                        label: "Grid",
                        value: "grid"
                    },
                    {
                        label: "List",
                        value: "list"
                    }]
                    currentValue: displayPopup.chosenViewMode
                    onActivated: value => {
                        displayPopup.chosenViewMode = value;
                        viewModeConfirmTimer.restart();
                    }
                }
            }

            // GameDisplayTypePopup {
            //     id: displayPopup
            //     y: 0
            //     x: viewAsButton.width + AppStyle.spacingXs
            //     view: root
            //
            //     enter: Transition {
            //         NumberAnimation {
            //             property: "opacity"
            //             from: 0
            //             to: 1
            //             duration: AppStyle.durationFast
            //             easing.type: Easing.InOutQuad
            //         }
            //         NumberAnimation {
            //             property: "x"
            //             from: viewAsButton.width + AppStyle.spacingXs - 12
            //             to: viewAsButton.width + AppStyle.spacingXs
            //             duration: AppStyle.durationFast
            //             easing.type: Easing.InOutQuad
            //         }
            //     }
            // }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }

    ColumnLayout {
        id: mainColumn
        anchors.left: toolbarColumn.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.rightMargin: 72
        spacing: 0

        // ---- Header / toolbar ----
        // ColumnLayout {
        //     Layout.fillWidth: true
        //     Layout.leftMargin: AppStyle.spacingLg
        //     Layout.rightMargin: AppStyle.spacingLg
        //     Layout.topMargin: AppStyle.spacingSm
        //     Layout.bottomMargin: AppStyle.spacingSm
        //     spacing: AppStyle.spacingSm
        //
        //     GameViewHeader {
        //         Layout.fillWidth: true
        //         view: root
        //     }
        //
        //     // Toolbar: search, quick filters, Filters/Display popups, details toggle
        //     GameToolbar {
        //         Layout.fillWidth: true
        //         view: root
        //     }
        //
        //     // Advanced filters expand inline under the toolbar
        //     GameFilterBar {
        //         view: root
        //         expanded: root.filtersExpanded
        //     }
        //
        //     // TODO
        //     // Bulk-action bar for multi-select. Slides in under the filter bar so
        //     // selecting adds a little header height rather than shifting anything
        //     Rectangle {
        //         id: selectionBar
        //         Layout.fillWidth: true
        //         Layout.preferredHeight: root.selectedCount > 0 ? selectionRow.implicitHeight + AppStyle.spacingSm * 2 : 0
        //         visible: Layout.preferredHeight > 0
        //         clip: true
        //         radius: AppStyle.radiusMd
        //         color: Theme.backgroundInset
        //
        //         Behavior on Layout.preferredHeight {
        //             NumberAnimation {
        //                 duration: AppStyle.durationFast
        //                 easing.type: Easing.InOutQuad
        //             }
        //         }
        //
        //         RowLayout {
        //             id: selectionRow
        //             anchors.left: parent.left
        //             anchors.right: parent.right
        //             anchors.verticalCenter: parent.verticalCenter
        //             anchors.leftMargin: AppStyle.spacingMd
        //             anchors.rightMargin: AppStyle.spacingMd
        //             spacing: AppStyle.spacingSm
        //
        //             Text {
        //                 text: root.selectedCount + " selected"
        //                 color: Theme.accent
        //                 font.family: AppStyle.fontFamily
        //                 font.pixelSize: AppStyle.fontSizeMedium
        //                 font.weight: Font.DemiBold
        //             }
        //             FLButton {
        //                 compact: true
        //                 variant: "default"
        //                 iconName: "favorite"
        //                 text: "Favorite"
        //                 onClicked: root.bulkFavorite(true)
        //             }
        //             FLButton {
        //                 compact: true
        //                 variant: "default"
        //                 text: "Add to folder…"
        //                 onClicked: root.openAddToFolder()
        //             }
        //             FLButton {
        //                 compact: true
        //                 variant: "default"
        //                 text: "Remove from folder"
        //                 visible: root.removableFolderId !== -1
        //                 onClicked: root.bulkRemoveFromFolder()
        //             }
        //             Item {
        //                 Layout.fillWidth: true
        //             }
        //             FLButton {
        //                 compact: true
        //                 variant: "subtle"
        //                 text: "Clear selection"
        //                 onClicked: root.clearSelection()
        //             }
        //         }
        //     }
        // }
        //
        // FLDivider {}

        Loader {
            id: viewLoader
            // clip: true
            Layout.fillHeight: true
            Layout.fillWidth: true
            sourceComponent: root.viewMode === "grid" ? gridView : listView
        }
    }

    on_PendingSortRoleChanged: {
        sortAnimation.start()
    }

    on_PendingViewTypeChanged: {
        changeViewAnimation.start()
    }

    SequentialAnimation {
        id: sortAnimation
        running: false
        ScriptAction {
            script: {
                FocusCursor.startBlink()
            }
        }
        ParallelAnimation {
            NumberAnimation {
                target: viewLoader
                property: "opacity"
                from: 1
                to: 0
                duration: AppStyle.durationSlow
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: viewLoader
                property: "y"
                from: 0
                to: 12
                duration: AppStyle.durationBase
                easing.type: Easing.InOutQuad
            }
        }

        ScriptAction {
            script: {
                root.sortRole = root._pendingSortRole;
                viewLoader.item.contentY = 0;
                // root._pendingSortRole = "";
            }
        }

        ParallelAnimation {
            NumberAnimation {
                target: viewLoader
                property: "opacity"
                from: 0
                to: 1
                duration: AppStyle.durationSlow
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: viewLoader
                property: "y"
                from: 12
                to: 0
                duration: AppStyle.durationBase
                easing.type: Easing.InOutQuad
            }
        }
        ScriptAction {
            script: {
                FocusCursor.endBlink()
            }
        }
    }

    SequentialAnimation {
        id: changeViewAnimation
        running: false
        ScriptAction {
            script: {
                FocusCursor.startBlink()
            }
        }
        ParallelAnimation {
            NumberAnimation {
                target: viewLoader
                property: "opacity"
                from: 1
                to: 0
                duration: AppStyle.durationSlow
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: viewLoader
                property: "y"
                from: 0
                to: 12
                duration: AppStyle.durationBase
                easing.type: Easing.InOutQuad
            }
        }

        ScriptAction {
            script: {
                root.viewMode = root._pendingViewType;
                viewLoader.item.contentY = 0;
                // root._pendingSortRole = "";
            }
        }

        ParallelAnimation {
            NumberAnimation {
                target: viewLoader
                property: "opacity"
                from: 0
                to: 1
                duration: AppStyle.durationSlow
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: viewLoader
                property: "y"
                from: 12
                to: 0
                duration: AppStyle.durationBase
                easing.type: Easing.InOutQuad
            }
        }
        ScriptAction {
            script: {
                FocusCursor.endBlink()
            }
        }
    }

    // Right dock: details for the last-clicked game. Width animates 0 -> open so
    // the grid reflows to make room; the panel keeps a fixed content width and is
    // clipped, so it slides rather than reflowing while animating
    // Item {
    //     id: detailDock
    //     anchors.right: parent.right
    //     anchors.top: parent.top
    //     anchors.bottom: parent.bottom
    //     readonly property int openWidth: AppStyle.detailPanelWidth
    //     width: root.detailOpen ? openWidth : 0
    //     clip: true
    //     Behavior on width {
    //         NumberAnimation {
    //             duration: 180
    //             easing.type: Easing.InOutQuad
    //         }
    //     }
    //
    //     GameDetailPanel {
    //         anchors.right: parent.right
    //         anchors.top: parent.top
    //         anchors.bottom: parent.bottom
    //         width: detailDock.openWidth
    //         gameData: root.detailData
    //         onRequestPlay: entryId => EmulationService.loadEntry(entryId)
    //         onRequestFavorite: (entryId, fav) => root.setDetailFavorite(entryId, fav)
    //         onRequestAddToFolder: entryId => addToFolderDialog.openFor([entryId])
    //     }
    // }

    Component {
        id: listView
        GameListView {
            model: gameModel
            selectedIds: root.selectedIds
            groupBy: root.groupBy
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
            onRequestEditGame: (id, hash, platformId) => editGameDialog.loadAndOpen(id, hash, platformId)

            header: GameViewHeader {
                width: ListView.view.width
                verticalPadding: AppStyle.spacingSm
                horizontalPadding: 0
            }
        }
    }

    Component {
        id: gridView
        GameGridView {
            model: gameModel
            currentSortLabel: root.currentSortLabel
            sortAscending: root.sortAscending
            selectedIds: root.selectedIds
            groupBy: root.groupBy
            groupKeys: root.groupKeys
            groupCounts: root.groupCounts
            onGameClicked: (entryId, rowIndex, modifiers) => root.handleGameClick(entryId, rowIndex, modifiers)
            onGameFocused: data => root.detailData = data
            onRequestAddToFolder: entryIds => addToFolderDialog.openFor(entryIds)
            onRequestChangeArt: (hash, name, platformId) => artPicker.openFor(hash, name, platformId)
            onRequestEditGame: (id, hash, platformId) => editGameDialog.loadAndOpen(id, hash, platformId)

            header: GameViewHeader {
                width: GridView.view.width - AppStyle.spacingSm * 2
                verticalPadding: AppStyle.spacingSm
                horizontalPadding: AppStyle.spacingSm
            }
        }
    }

    component GameViewHeader: Pane {
        background: Item {}
        contentItem: RowLayout {
            width: parent.width
            height: parent.height

            Text {
                text: "Showing " + root.gameCount
                color: Theme.textMuted
                font.family: AppStyle.fontFamily
                font.pixelSize: AppStyle.fontSizeMedium
                font.weight: Font.Medium
                Layout.alignment: Qt.AlignVCenter
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            Text {
                text: "By " + root.currentSortLabel + " (" + (root.sortAscending ? "ascending" : "descending") + ")"
                color: Theme.textMuted
                font.family: AppStyle.fontFamily
                font.pixelSize: AppStyle.fontSizeMedium
                font.weight: Font.Medium
                Layout.alignment: Qt.AlignVCenter
            }
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
}
