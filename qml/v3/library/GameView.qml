// TODO: NEEDS REVIEW
import QtQuick
import QtQml
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FocusScope {
    id: root

    focus: true

    // The collection being shown, or -1 for the whole library
    property alias filterFolderId: gameModel.openFolderId

    readonly property bool scopeIsSmart: gameModel.openFolderIsSmart

    function focusFirstItem() {
        browse.focusFirstItem();
    }

    function enterFocus() {
        browse.enterFocus();
    }

    LibraryEntrySortFilterModel {
        id: gameModel
        sourceModel: LibraryEntryModel
        folderModel: LibraryFolderModel

        onRefinementChanged: browse.queueRefresh()
        onOpenFolderChanged: browse.resetPosition()
    }

    Component.onCompleted: browse.setViewMode(AppearanceSettings.libraryViewMode)

    Connections {
        target: AppearanceSettings

        function onLibraryViewModeChanged() {
            if (displayPopup.opened) {
                browse.requestViewMode(AppearanceSettings.libraryViewMode);
            } else {
                browse.setViewMode(AppearanceSettings.libraryViewMode);
            }
        }
    }

    // For multi-select
    property var selectedIds: ({})
    property int selectionAnchorRow: -1

    readonly property bool sortIsPinnedToCollection: gameModel.sortPinnedToOpenFolder

    onFilterFolderIdChanged: root.clearSelection()

    function clearOrResetFilters() {
        if (!root.scopeIsSmart) {
            gameModel.clearAllFilters();
            return;
        }

        gameModel.resetToSaved();
    }

    function saveFiltersToCollection() {
        if (!root.scopeIsSmart) {
            return;
        }

        const saved = gameModel.filter.json;

        if (LibraryFolderModel.updateSmartFolder(root.filterFolderId, saved)) {
            gameModel.filter.savedJson = saved;
        }
    }

    function selectOnly(entryId, rowIndex) {
        var s = {};
        s[entryId] = true;
        root.selectedIds = s;
        root.selectionAnchorRow = rowIndex;
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
    }

    function rangeSelectTo(rowIndex) {
        if (root.selectionAnchorRow < 0) {
            root.selectionAnchorRow = rowIndex;
        }
        var lo = Math.min(root.selectionAnchorRow, rowIndex);
        var hi = Math.max(root.selectionAnchorRow, rowIndex);
        var s = {};
        for (var i = lo; i <= hi && i < gameModel.count; i++) {
            s[gameModel.getEntryIdAt(i)] = true;
        }
        root.selectedIds = s;
    }

    function clearSelection() {
        root.selectedIds = ({});
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

    FLDialog {
        id: cannotLaunchGamePopup

        property string text: "Cannot launch game"

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

    FLBrowseView {
        id: browse
        anchors.fill: parent

        gridComponent: gridView
        listComponent: listView

        isEmpty: gameModel.count === 0
        emptyComponent: {
            if (gameModel.anyFiltersActive) {
                return noMatchingFiltersView;
            }

            if (root.filterFolderId !== -1) {
                return emptyCollectionView;
            }

            return noGamesView;
        }

        onCommit: gameModel.applyFilters()

        FLIconButton {
            id: editCollectionButton
            Layout.alignment: Qt.AlignHCenter
            iconName: "edit"
            tooltipText: "Edit collection"
            filled: false
            compact: false
            visible: root.filterFolderId !== -1
            onClicked: {
                if (root.filterFolderId === -1) {
                    return;
                }
            }
        }

        GameFilterButton {
            id: filterButton
            entryModel: gameModel
            scopeIsSmart: root.scopeIsSmart
            onSaveRequested: root.saveFiltersToCollection()
            onClearRequested: root.clearOrResetFilters()
        }

        GameSortButton {
            id: sortButton
            entryModel: gameModel
            showsPinRow: root.filterFolderId !== -1
        }

        FLIconButton {
            id: viewAsButton
            Layout.alignment: Qt.AlignHCenter
            iconName: browse.viewMode === "grid" ? "grid_view" : "view_list"
            tooltipText: "View as"
            compact: false
            onClicked: displayPopup.opened ? displayPopup.close() : displayPopup.open()

            FLMenu {
                id: displayPopup
                x: viewAsButton.width + AppStyle.spacingXs
                minWidth: 360

                SettingsGroup {
                    title: "View as"
                    showHeader: false
                    group: "library-view-mode"
                    surface: FLMenuItem.Surface.InMenu
                    focus: true
                }

                FLMenuSeparator {}

                SettingsGroup {
                    visible: browse.viewMode === "list"
                    showHeader: false
                    showTopPadding: false
                    title: "List settings"
                    group: "library-list-appearance"
                    surface: FLMenuItem.Surface.InMenu
                }

                SettingsGroup {
                    visible: browse.viewMode === "grid"
                    showHeader: false
                    showTopPadding: false
                    title: "Grid settings"
                    group: "library-grid-appearance"
                    surface: FLMenuItem.Surface.InMenu

                    // Let the display update on sliders but don't write the value until released
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

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }

    Component {
        id: listView
        GameListView {
            model: gameModel
            selectedIds: root.selectedIds
            onGameClicked: (entryId, rowIndex, modifiers) => root.handleGameClick(entryId, rowIndex, modifiers)

            header: GameViewHeader {
                height: AppStyle.gameViewHeaderHeight
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
            currentSortLabel: gameModel.sortDisplayName
            sortAscending: gameModel.sortAscending
            selectedIds: root.selectedIds

            onGameClicked: (entryId, rowIndex, modifiers) => root.handleGameClick(entryId, rowIndex, modifiers)
            onRequestLaunch: (id, hash, platformId, playable, statusText) => {
                if (!playable) {
                    cannotLaunchGamePopup.openWithText(statusText);
                    return;
                }

                EmulationService.loadEntry(id);
            }

            header: GameViewHeader {
                height: AppStyle.gameViewHeaderHeight
                width: GridView.view.width
                verticalPadding: AppStyle.spacingXs
                leftPadding: AppStyle.spacingSm
                rightPadding: AppStyle.spacingSm
            }

            footer: Item {
                height: 48
                width: GridView.view.width
            }
        }
    }

    Component {
        id: noMatchingFiltersView
        FLColumnLayout {
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
                text: root.scopeIsSmart ? "Reset to saved filters" : "Clear all filters"
                Layout.alignment: Qt.AlignHCenter

                FLFocus.focusSound: SoundEffects.menuNavigate
                FLFocus.actions: [
                    FLAction {
                        keys: [Qt.Key_Select, Qt.Key_Return, Qt.Key_Enter, Qt.Key_Space]
                        label: qsTr("Select")
                        sound: SoundEffects.openPopup
                        onTriggered: root.clearOrResetFilters()
                    }
                ]

                onClicked: {
                    root.clearOrResetFilters();
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }

    Component {
        id: emptyCollectionView

        FLColumnLayout {
            spacing: AppStyle.spacingXl

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            Text {
                Layout.preferredWidth: 400
                Layout.alignment: Qt.AlignHCenter
                text: root.scopeIsSmart ? "Nothing matches this collection yet" : "Nothing in this collection yet"
                color: Theme.textPrimary
                font.family: AppStyle.fontFamily
                font.pixelSize: AppStyle.fontSizeMedium
                horizontalAlignment: Text.AlignHCenter
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

            property bool showDad: false

            Component.onCompleted: {
                showDad = Math.random() < 0.5
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            Text {
                Layout.preferredWidth: 400
                Layout.alignment: Qt.AlignHCenter
                text: "Here's where your games would go\n\nIF YOU HAD ANY"
                visible: !parent.showDad
                color: Theme.textPrimary
                font.family: AppStyle.fontFamily
                font.pixelSize: AppStyle.fontSizeMedium
                font.weight: Font.Normal
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            Image {
                visible: parent.showDad
                Layout.alignment: Qt.AlignHCenter
                source: "file:///Users/alexcharles/Documents/this-is-where.webp"

            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }

    component GameViewHeader: Pane {
        horizontalPadding: 0
        verticalPadding: 0

        background: Item {}
        contentItem: RowLayout {
            Text {
                text: "Showing " + gameModel.count + " games"
                color: Theme.textMuted
                font.family: AppStyle.fontFamily
                font.pixelSize: AppStyle.fontSizeSmall
                font.weight: Font.Normal
                Layout.alignment: Qt.AlignBottom
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            Text {
                text: "By " + gameModel.sortDisplayName + " (" + (gameModel.sortAscending ? "ascending" : "descending") + ")"
                color: Theme.textMuted
                font.family: AppStyle.fontFamily
                font.pixelSize: AppStyle.fontSizeSmall
                font.weight: Font.Normal
                Layout.alignment: Qt.AlignBottom
            }
        }
    }
}
