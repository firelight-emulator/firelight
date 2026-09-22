// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FocusScope {
    id: root
    objectName: "CollectionGamePicker|" + (root.selecting ? "games" : "criteria")

    property bool selecting: true

    signal picksChanged(var entryIds)
    signal criteriaChanged(string json)

    property bool _isRefreshing: false
    property bool _isLoading: false

    focus: true

    function load(entryIds: var, criteriaJson: string) {
        root._isLoading = true;

        if (criteriaJson !== "") {
            mainModel.filter.setJson(criteriaJson);
        } else {
            mainModel.filter.clear();
        }

        root._isLoading = false;
        root._refresh(entryIds);
    }

    function enterFocus() {
        browse.enterFocus();
    }

    function _currentPicks(): var {
        const picks = [];

        for (let row = 0; row < pickedModel.count; row++) {
            if (!pickedGroup.isSelected(row)) {
                continue;
            }

            const entryId = pickedModel.getEntryIdAt(row);

            if (entryId !== -1) {
                picks.push(entryId);
            }
        }

        const mainRows = Object.keys(mainGroup.selected).map(key => parseInt(key)).sort((a, b) => a - b);

        for (let i = 0; i < mainRows.length; i++) {
            const entryId = mainModel.getEntryIdAt(mainRows[i]);

            if (entryId !== -1) {
                picks.push(entryId);
            }
        }

        return picks;
    }

    function _refresh(picks: var) {
        root._isRefreshing = true;

        mainModel.pickedEntryIds = picks;
        pickedModel.pickedEntryIds = picks;
        pickedModel.sortRole = mainModel.sortRole;
        pickedModel.sortAscending = mainModel.sortAscending;

        mainModel.applyFilters();
        pickedModel.applyFilters();

        mainGroup.clearSelection();
        pickedGroup.selectAll(pickedModel.count);

        root._isRefreshing = false;
        root._announcePicks();
    }

    function _announcePicks() {
        if (root._isRefreshing || !root.selecting) {
            return;
        }

        root.picksChanged(root._currentPicks());
    }

    LibraryEntrySortFilterModel {
        id: mainModel
        pickedMode: root.selecting ? LibraryEntrySortFilterModel.HidePicked : LibraryEntrySortFilterModel.ShowAll
        sourceModel: LibraryEntryModel

        onRefinementChanged: {
            if (!root._isLoading) {
                browse.queueRefresh();
            }
        }
    }

    LibraryEntrySortFilterModel {
        id: pickedModel
        pickedMode: LibraryEntrySortFilterModel.OnlyPicked
        sourceModel: LibraryEntryModel
    }

    SelectionGroup {
        id: mainGroup
        active: root.selecting
        onSelectedChanged: root._announcePicks()
    }

    SelectionGroup {
        id: pickedGroup
        active: root.selecting
        onSelectedChanged: root._announcePicks()
    }

    Connections {
        target: mainModel.filter
        enabled: !root.selecting

        function onChanged() {
            root.criteriaChanged(mainModel.filter.empty ? "" : mainModel.filter.json);
        }
    }

    FLBrowseView {
        id: browse
        anchors.fill: parent

        gridComponent: gridView

        isEmpty: mainModel.count === 0 && pickedModel.count === 0
        emptyComponent: mainModel.anyFiltersActive ? noMatchingFiltersView : noGamesView

        onCommit: root._refresh(root._currentPicks())

        GameFilterButton {
            id: filterButton
            entryModel: mainModel
            onClearRequested: mainModel.filter.clear()
        }

        GameSortButton {
            id: sortButton
            entryModel: mainModel
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }

    Component {
        id: gridView

        GameGridView {
            id: mainGrid
            model: mainModel
            currentSortLabel: mainModel.sortDisplayName
            sortAscending: mainModel.sortAscending
            canLaunch: false
            selectionGroup: mainGroup

            header: Column {
                id: gridHeader

                readonly property GridView outerView: gridHeader.GridView.view

                width: gridHeader.outerView ? gridHeader.outerView.width : 0

                Column {
                    id: pickedSection
                    width: parent.width
                    visible: root.selecting && pickedModel.count > 0

                    Text {
                        width: parent.width
                        height: AppStyle.gameViewHeaderHeight
                        leftPadding: AppStyle.spacingSm
                        rightPadding: AppStyle.spacingSm
                        bottomPadding: AppStyle.spacingXs
                        text: qsTr("Selected") + " (" + pickedModel.count + ")"
                        color: Theme.textMuted
                        font.family: AppStyle.fontFamily
                        font.pixelSize: AppStyle.fontSizeSmall
                        font.weight: Font.Normal
                        verticalAlignment: Text.AlignBottom
                    }

                    FLGridView {
                        id: pickedGrid
                        objectName: "CollectionGamePicker|picked"

                        width: parent.width
                        height: Math.ceil(pickedGrid.count / pickedGrid.columns) * pickedGrid.cellHeight

                        focus: false
                        interactive: false

                        model: pickedModel
                        selectionGroup: pickedGroup

                        cellWidth: gridHeader.outerView ? gridHeader.outerView.cellWidth : 0
                        cellHeight: gridHeader.outerView ? gridHeader.outerView.cellHeight : 0

                        delegate: GameGridViewItem {
                            required property var model

                            width: GridView.view.cellWidth
                            height: GridView.view.cellHeight

                            titleBoxHeight: mainGrid.labelHeight
                            canLaunch: false
                        }
                    }
                }

                Pane {
                    width: parent.width
                    height: AppStyle.gameViewHeaderHeight
                    verticalPadding: AppStyle.spacingXs
                    leftPadding: AppStyle.spacingSm
                    rightPadding: AppStyle.spacingSm

                    background: Item {}
                    contentItem: RowLayout {
                        Text {
                            text: qsTr("Showing %1 games").arg(mainModel.count)
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
                            text: mainModel.sortAscending ? qsTr("By %1 (ascending)").arg(mainModel.sortDisplayName) : qsTr("By %1 (descending)").arg(mainModel.sortDisplayName)
                            color: Theme.textMuted
                            font.family: AppStyle.fontFamily
                            font.pixelSize: AppStyle.fontSizeSmall
                            font.weight: Font.Normal
                            Layout.alignment: Qt.AlignBottom
                        }
                    }
                }
            }

            footer: Item {
                height: AppStyle.spacingXl * 2
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
                Layout.preferredWidth: AppStyle.defaultDialogMinimumWidth
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("No games match the current filters")
                color: Theme.textPrimary
                font.family: AppStyle.fontFamily
                font.pixelSize: AppStyle.fontSizeMedium
                font.weight: Font.Normal
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap
            }

            FLButton {
                id: clearAllFiltersButton
                objectName: "CollectionGamePicker|clearAllFilters"
                Layout.alignment: Qt.AlignHCenter
                focus: true
                text: qsTr("Clear all filters")
                onClicked: mainModel.filter.clear()
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }

    Component {
        id: noGamesView

        Item {
            Text {
                anchors.centerIn: parent
                text: qsTr("There are no games to choose from")
                color: Theme.textMuted
                font.family: AppStyle.fontFamily
                font.pixelSize: AppStyle.fontSizeMedium
            }
        }
    }
}
