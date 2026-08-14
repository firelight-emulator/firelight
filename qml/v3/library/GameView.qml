// TODO: NEEDS REVIEW
import QtQuick
import QtQml
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.synchronizer
import Firelight 1.0

Item {
    id: root

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

    LibraryEntrySortFilterModel {
        id: gameModel
        sourceModel: LibraryEntryModel
        collapseVariants: GeneralSettings.collapseVariants

        onFiltersOrSortChanged: root.queueRefresh()
    }

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
    // readonly property int gameCount: gameMirror.count

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
        filterFolderId = -1;
        filterManualIds = [];
        filterSmartIds = [];
        scopeLabel = "All games";
        scopeCrumb = [];
        scopeIconUrl = "";
        scopeIconName = "browse";
        scopeIconColor = Theme.textPrimary;
        scopeDescription = "";
        clearSelection();
    }

    function setScopePlatform(platformId, label) {
        filterFolderId = -1;
        filterManualIds = [];
        filterSmartIds = [];
        scopeLabel = label;
        scopeCrumb = [];
        scopeIconUrl = "";
        scopeIconName = "";
        scopeIconColor = Theme.textPrimary;
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

    LibraryFilters {
        id: filters

        onPendingValuesChanged: root.queueRefresh()
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

    FLDialog {
        id: cannotLaunchGamePopup

        property string text: "Cannot launch game"
        openSound: null

        function openWithText(dialogText) {
            cannotLaunchGamePopup.text = dialogText;
            cannotLaunchGamePopup.open();
        }

        Text {
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: cannotLaunchGamePopup.text
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeMedium
            font.weight: Font.Normal
            color: Theme.textPrimary
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
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

        // FLIconButton {
        //     Layout.alignment: Qt.AlignHCenter
        //     iconName: "add"
        //     tooltipText: "Add or create"
        //     compact: false
        //     onClicked: addPopup.opened ? addPopup.close() : addPopup.open()
        //
        //     // RightClickMenu {
        //     //     id: addPopup
        //     //     x: parent.width + AppStyle.spacingXs
        //     //     y: 0
        //     //
        //     //     RightClickMenuItem {
        //     //         text: "Add game"
        //     //         onTriggered: {
        //     //             // GameAddDialog.openForExisting();
        //     //         }
        //     //     }
        //     //
        //     //     RightClickMenuItem {
        //     //         text: "Create folder"
        //     //         onTriggered: {
        //     //             // GameAddDialog.openForExisting();
        //     //         }
        //     //     }
        //     //
        //     //     enter: Transition {
        //     //         NumberAnimation {
        //     //             property: "opacity"
        //     //             from: 0
        //     //             to: 1
        //     //             duration: AppStyle.durationFast
        //     //             easing.type: Easing.InOutQuad
        //     //         }
        //     //         NumberAnimation {
        //     //             property: "x"
        //     //             from: gameSortPopup.x - 8
        //     //             to: gameSortPopup.x
        //     //             duration: AppStyle.durationFast
        //     //             easing.type: Easing.InOutQuad
        //     //         }
        //     //     }
        //     // }
        // }
        //
        // FLIconButton {
        //     Layout.alignment: Qt.AlignHCenter
        //     iconName: "search"
        //     tooltipText: "Search"
        //     compact: false
        // }

        FLIconButton {
            id: filterButton
            Layout.alignment: Qt.AlignHCenter
            iconName: "filter-alt"
            tooltipText: "Filter"
            filled: false
            compact: false
            iconColor: filterPopup.visible || gameModel.anyFiltersActive ? Theme.switch2Color : Theme.textPrimary
            onClicked: filterPopup.opened ? filterPopup.close() : filterPopup.open()

            Rectangle {
                color: Theme.switch2Color
                height: 6
                width: 6
                radius: width / 2
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.rightMargin: 8
                anchors.bottomMargin: 8
                visible: gameModel.anyFiltersActive
            }

            FLMenu {
                id: filterPopup
                x: filterButton.width + AppStyle.spacingXs
                minWidth: 300

                FLButton {
                    id: clearButton
                    text: "Clear all filters"
                    Layout.fillWidth: true
                    Layout.leftMargin: AppStyle.spacingSm
                    Layout.rightMargin: AppStyle.spacingSm
                    Layout.topMargin: AppStyle.spacingSm
                    Layout.bottomMargin: AppStyle.spacingSm
                    canInteract: gameModel.anyFiltersActive

                    FLFocus.focusSound: SoundEffects.menuNavigate
                    FLFocus.actions: [
                        FLAction {
                            keys: [Qt.Key_Select, Qt.Key_Return, Qt.Key_Enter, Qt.Key_Space]
                            label: qsTr("Select")
                            sound: clearButton.canInteract ? SoundEffects.openPopup : SoundEffects.cursorBump
                            onTriggered: gameModel.clearAllFilters()
                        }
                    ]

                    onClicked: {
                        gameModel.clearAllFilters();
                    }

                    // variant: "subtle"
                }

                FLToggleMenuItem {
                    label: "Favorites"
                    checked: gameModel.favoritesOnly
                    onSelected: function (selected) {
                        gameModel.favoritesOnly = selected;
                    }
                }

                FLSubmenuItem {
                    label: "Platform"
                    model: PlatformModel
                    textRole: "displayName"
                    valueRole: "platformId"

                    Synchronizer on currentValues {
                        targetObject: gameModel
                        targetProperty: "platformIds"
                    }
                }

                FLToggleMenuItem {
                    label: "Has achievements"
                }

                FLSubmenuItem {
                    label: "Time played"
                    model: []
                }

                FLSubmenuItem {
                    label: "Developer"
                    model: []
                }

                FLSubmenuItem {
                    label: "Publisher"
                    model: []
                }

                FLSubmenuItem {
                    label: "Genre"
                    model: []
                }

                FLSubmenuItem {
                    label: "Tags"
                    model: []
                }

                FLToggleMenuItem {
                    label: "Hide unplayable"
                    checked: gameModel.hideUnavailable
                    onSelected: function (selected) {
                        gameModel.hideUnavailable = selected;
                    }
                }

                // FLToggleMenuItem {
                //     label: "Title"
                // }
                //
                // FLSubmenuItem {
                //     label: "Playtime"
                //     model: [
                //         {
                //             "text": "Up to 1 hour",
                //             "value": "1h"
                //         },
                //         {
                //             "text": "Up to 2 hours",
                //             "value": "2h"
                //         },
                //         {
                //             "text": "Up to 3 hours",
                //             "value": "3h"
                //         },
                //         {
                //             "text": "Up to 4 hours",
                //             "value": "4h"
                //         },
                //         {
                //             "text": "Up to 5 hours",
                //             "value": "5h"
                //         },
                //         {
                //             "text": "Up to 10 hours",
                //             "value": "10h"
                //         },
                //         {
                //             "text": "Up to 15 hours",
                //             "value": "15h"
                //         }
                //     ]
                //
                //     onCurrentValuesChanged: function () {
                //         console.log("Selected platforms: " + currentValues.join(", "));
                //     }
                // }
                //
                // FLToggleMenuItem {
                //     label: "Number of players"
                // }
                //
                // FLToggleMenuItem {
                //     label: "Achievements"
                // }
                //
                // FLToggleMenuItem {
                //     label: "Developer"
                // }
                //
                // FLToggleMenuItem {
                //     label: "Publisher"
                // }
                //
                // FLToggleMenuItem {
                //     label: "Genre"
                // }
                //
                // FLToggleMenuItem {
                //     label: "Release year"
                // }
                //
                // FLToggleMenuItem {
                //     label: "Tags"
                // }
            }
        }

        FLIconButton {
            id: sortButton
            Layout.alignment: Qt.AlignHCenter
            iconName: "list-arrow"
            tooltipText: "Sort"
            compact: false
            iconColor: gameSortPopup.visible ? Theme.switch2Color : Theme.textPrimary
            onClicked: gameSortPopup.opened ? gameSortPopup.close() : gameSortPopup.open()

            // FLMenu {
            //     id: gameSortPopup
            //     x: sortButton.width + AppStyle.spacingXs
            //
            //     FLRadioGroup {
            //         model: root.sortOptions
            //         valueRole: "role"
            //         currentValue: root.sortRole
            //
            //         // // TODO: Put the close delay and auto close in FLMenu
            //         // onClosed: {
            //         //     if (gameSortPopup.currentValue !== root.sortRole) {
            //         //         root._pendingSortRole = gameSortPopup.currentValue;
            //         //     }
            //         // }
            //     }
            // }
            FLRadioMenu {
                id: gameSortPopup
                x: sortButton.width + AppStyle.spacingXs

                model: gameModel.sortOptions
                currentValue: gameModel.sortRole

                onClosed: gameModel.sortRole = gameSortPopup.currentValue
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

            FLMenu {
                id: displayPopup
                x: viewAsButton.width + AppStyle.spacingXs
                minWidth: 360

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
                    }
                }

                Timer {
                    id: viewModeConfirmTimer
                    interval: InputMethodManager.usingMouse ? 0 : AppStyle.confirmPause
                    onTriggered: displayPopup.close()
                }

                FLRadioGroup {
                    Keys.onPressed: event => {
                        event.accepted = displayPopup.navigate(event.key, event.isAutoRepeat);
                    }

                    model: [
                        {
                            text: "Grid",
                            value: "grid"
                        },
                        {
                            text: "List",
                            value: "list"
                        }
                    ]
                    currentValue: displayPopup.chosenViewMode
                    onActivated: value => {
                        displayPopup.chosenViewMode = value;
                        viewModeConfirmTimer.restart();
                    }
                }
                SettingsGroup {
                    group: "library-grid-appearance"
                    inMenu: true

                    // TODO
                    // The grid follows the handle while it moves and goes back to the stored value once
                    // it stops, so dragging the size around costs no writes
                    onSlid: function (key, value) {
                        if (key === "library-icon-grid-tile-size") {
                            AppearanceSettings.libraryIconGridTileSizePreview = value;
                        } else if (key === "library-icon-grid-tile-spacing") {
                            AppearanceSettings.libraryIconGridTileSpacingPreview = value;
                        }
                    }
                    onSettled: function (key) {
                        if (key === "library-icon-grid-tile-size") {
                            AppearanceSettings.libraryIconGridTileSizePreview = -1;
                        } else if (key === "library-icon-grid-tile-spacing") {
                            AppearanceSettings.libraryIconGridTileSpacingPreview = -1;
                        }
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
            sourceComponent: {
                if (gameModel.count === 0) {
                    if (gameModel.anyFiltersActive) {
                        return noMatchingFiltersView
                    }

                    return noGamesView
                }

                return root.viewMode === "grid" ? gridView : listView
            }
        }
    }

    Pane {
        id: testing
        width: 500
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        background: Rectangle {
            color: Theme.surfaceElevated

            // TODO
            // A panel laid over the grid has to swallow what the rows above it do not. Nothing
            // else here consumes a press: a Rectangle does not, and the rows use TapHandlers,
            // which take a passive grab by design and let the press carry on to the grid
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.AllButtons
                hoverEnabled: true
                onWheel: wheel => wheel.accepted = true
            }
        }

        FLColumnLayout {
            anchors.fill: parent
            spacing: AppStyle.spacingMd

            SettingsGroup {
                group: "library-grid-appearance"

                // TODO
                // The grid follows the handle while it moves and goes back to the stored value once
                // it stops, so dragging the size around costs no writes
                onSlid: function (key, value) {
                    if (key === "library-icon-grid-tile-size") {
                        AppearanceSettings.libraryIconGridTileSizePreview = value;
                    } else if (key === "library-icon-grid-tile-spacing") {
                        AppearanceSettings.libraryIconGridTileSpacingPreview = value;
                    }
                }
                onSettled: function (key) {
                    if (key === "library-icon-grid-tile-size") {
                        AppearanceSettings.libraryIconGridTileSizePreview = -1;
                    } else if (key === "library-icon-grid-tile-spacing") {
                        AppearanceSettings.libraryIconGridTileSpacingPreview = -1;
                    }
                }
            }
        }
    }

    // TODO
    // Whether the run in flight has already swapped the rows. A change arriving
    // before that point is picked up by the same apply; one arriving after needs
    // a pass of its own
    property bool _refreshApplied: false
    property bool _refreshQueued: false

    // TODO
    // Folds a change into the run in flight wherever it still can be, so several
    // filters changed in quick succession hide the view once rather than each
    // replaying the fade
    function queueRefresh() {
        if (refreshAnimation.running) {
            if (root._refreshApplied) {
                root._refreshQueued = true;
            }

            return;
        }

        root._refreshApplied = false;
        refreshAnimation.start();
    }

    SequentialAnimation {
        id: refreshAnimation

        running: false

        onRunningChanged: {
            if (running || !root._refreshQueued) {
                return;
            }

            root._refreshQueued = false;
            root._refreshApplied = false;
            refreshAnimation.start();
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
                root._refreshApplied = true;
                gameModel.applyFilters();
                root.sortRole = root._pendingSortRole;
                if (viewLoader.item) {
                    viewLoader.item.positionViewAtBeginning();
                }
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
    }

    SequentialAnimation {
        id: changeViewAnimation
        running: false
        ScriptAction {
            script: {
                FocusCursor.startBlink();
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
                viewLoader.item.positionViewAtBeginning();
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
                FocusCursor.endBlink();
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
            onRequestLaunch: (id, hash, platformId, playable, statusText) => {
                if (!playable) {
                    cannotLaunchGamePopup.openWithText(statusText);
                }
            }

            header: GameViewHeader {
                width: GridView.view.width - AppStyle.spacingSm * 2
                verticalPadding: AppStyle.spacingSm
                horizontalPadding: AppStyle.spacingSm
            }
        }
    }

    Component {
        id: noMatchingFiltersView
        ColumnLayout {
            spacing: AppStyle.spacingXl

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            Text {
                Layout.preferredWidth: 400
                Layout.alignment: Qt.AlignHCenter
                text: "Uh-oh! No games match the current filters"
                color: Theme.textPrimary
                font.family: AppStyle.fontFamily
                font.pixelSize: AppStyle.fontSizeMedium
                font.weight: Font.Normal
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            FLButton {
                id: clearAllFiltersButton
                text: "Clear all filters"
                Layout.alignment: Qt.AlignHCenter

                FLFocus.focusSound: SoundEffects.menuNavigate
                FLFocus.actions: [
                    FLAction {
                        keys: [Qt.Key_Select, Qt.Key_Return, Qt.Key_Enter, Qt.Key_Space]
                        label: qsTr("Select")
                        sound: SoundEffects.openPopup
                        onTriggered: gameModel.clearAllFilters()
                    }
                ]

                onClicked: {
                    gameModel.clearAllFilters()
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }

    }

    Component {
        id: noGamesView
        ColumnLayout {
            spacing: AppStyle.spacingXl

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            Text {
                Layout.preferredWidth: 400
                Layout.alignment: Qt.AlignHCenter
                text: "You don't have any games in your library yet"
                color: Theme.textPrimary
                font.family: AppStyle.fontFamily
                font.pixelSize: AppStyle.fontSizeMedium
                font.weight: Font.Normal
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            // FLButton {
            //     id: clearAllFiltersButton
            //     text: "Clear all filters"
            //     Layout.alignment: Qt.AlignHCenter
            //
            //     FLFocus.focusSound: SoundEffects.menuNavigate
            //     FLFocus.actions: [
            //         FLAction {
            //             keys: [Qt.Key_Select, Qt.Key_Return, Qt.Key_Enter, Qt.Key_Space]
            //             label: qsTr("Select")
            //             sound: SoundEffects.openPopup
            //             onTriggered: gameModel.clearAllFilters()
            //         }
            //     ]
            //
            //     onClicked: {
            //         gameModel.clearAllFilters()
            //     }
            // }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }

    }

    component GameViewHeader: Pane {
        background: Item {}
        contentItem: RowLayout {
            width: parent.width
            height: parent.height

            Text {
                text: "Showing " + gameModel.count
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
                text: "By " + gameModel.sortDisplayName + " (" + (root.sortAscending ? "ascending" : "descending") + ")"
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
        property int platformId
        property list<int> folderIds
        property int achievementsEarned
        property int achievementsTotal
        property var numSecondsPlayed
        property int releaseYear
        property string genres
    }
}
