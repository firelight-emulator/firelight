import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQml.Models
import QtQuick.Layouts 1.0
import QtNetwork
import QtMultimedia
import QtQuick.Effects
import Firelight 1.0

Item {
    id: root

    property int currentIndex: -1
    property string filterText: ""
    property bool showJumpToTop: true
    property bool sortAscending: true

    // Sorting and filtering properties
    property string sortRole: "displayName"
    property string viewMode: "grid" // "list", "grid"

    property bool showOnlyFavorites: false
    property int filterFolderId: -1
    // Whether the currently-filtered folder is a smart folder (membership is
    // computed live via matchesSmartFolder) rather than a manual one.
    property bool filterFolderIsSmart: false
    property int filterPlatformId: -1

    // Guards the per-folder sort write-back so applying a folder's remembered
    // sort doesn't immediately persist it back as a "user change".
    property bool _applyingFolderSort: false

    // Applies a folder's remembered game sort (no-op when the folder has none,
    // so the current/default sort is kept).
    function applyFolderSort(sortRole, ascending) {
        if (sortRole && sortRole.length > 0) {
            _applyingFolderSort = true;
            root.sortRole = sortRole;
            root.sortAscending = ascending;
            _applyingFolderSort = false;
        }
    }

    // Remembers the current sort on the active folder (called when the user
    // changes sort while viewing a folder).
    function persistFolderSort() {
        if (!_applyingFolderSort && filterFolderId !== -1) {
            LibraryFolderModel.setFolderSort(filterFolderId, sortRole, sortAscending);
        }
    }

    function clearFilters() {
        showOnlyFavorites = false
        filterFolderId = -1
        filterFolderIsSmart = false
        filterPlatformId = -1
    }

    function showAllGames() {
        showOnlyFavorites = false
        filterFolderId = -1
        filterFolderIsSmart = false
        filterPlatformId = -1
    }

    function filterByFavorites() {
        showOnlyFavorites = true
        filterFolderId = -1
        filterFolderIsSmart = false
        filterPlatformId = -1
    }

    function filterByFolderId(folderId, isSmart, sortRole, sortAscending) {
        showOnlyFavorites = false
        filterFolderIsSmart = isSmart === true
        filterFolderId = folderId
        filterPlatformId = -1
        applyFolderSort(sortRole, sortAscending)
    }

    function filterByPlatform(platformId) {
        showOnlyFavorites = false
        filterFolderId = -1
        filterFolderIsSmart = false
        filterPlatformId = platformId
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
                roleName: "platformId"
                value: root.filterPlatformId
                enabled: root.filterPlatformId !== -1
            },

            FunctionFilter {
                property int folderId: root.filterFolderId
                property bool isSmart: root.filterFolderIsSmart
                enabled: folderId !== -1

                function filter(data: RoleData): bool {
                    // Smart folders compute membership live from criteria;
                    // manual folders use folder_entries membership.
                    if (isSmart) {
                        return LibraryEntryModel.matchesSmartFolder(folderId, data.entryId);
                    }
                    return data.folderIds.includes(folderId);
                }

                onFolderIdChanged: {
                    // gameList.currentIndex = -1
                    invalidate();
                }

                onIsSmartChanged: {
                    invalidate();
                }
            },

            FunctionFilter {
                property string filterText: root.filterText

                function filter(data: RoleData): bool {
                    return data.displayName.toLowerCase().indexOf(root.filterText.toLowerCase()) !== -1;
                }

                onFilterTextChanged: {
                    // gameList.currentIndex = -1
                    invalidate();
                }
            }
        ]
        sorters: [
            RoleSorter {
                roleName: root.sortRole
                sortOrder: root.sortAscending ? Qt.AscendingOrder : Qt.DescendingOrder
            }
        ]
    }

    // Rectangle {
    //     width: parent.width
    //     height: 48
    //     y: -viewLoader.item.contentY
    //     color: "blue"
    // }

    // Pane {
    //     id: headerPane
    //     anchors.top: parent.top
    //     anchors.left: parent.left
    //     anchors.right: parent.right
    //     padding: 0
    //     topPadding: 16
    //
    //     background: Item {}
    //
    //     contentItem: ColumnLayout {
    //         spacing: 16
    //         RowLayout {
    //             spacing: 12
    //             Layout.fillWidth: true
    //             // Rectangle {
    //             //     color: "red"
    //             //     implicitHeight: 72
    //             //     implicitWidth: 72
    //             // }
    //
    //             ColumnLayout {
    //                 Text {
    //                     text: "All games"
    //                     font.pointSize: 18
    //                     font.weight: Font.Bold
    //                      font.family: Constants.regularFontFamily
    //                     color: "white"
    //                 }
    //                 // Text {
    //                 //     text: "Description"
    //                 //     font.pointSize: 12
    //                 //      font.weight: Font.Medium
    //                 //      font.family: Constants.regularFontFamily
    //                 //     color: "#919191"
    //                 // }
    //             }
    //         }
    //
    //         RowLayout {
    //             Layout.fillWidth: true
    //             Layout.fillHeight: true
    //             Layout.bottomMargin: 8
    //             spacing: 8
    //
    //             TextField {
    //                 id: searchField
    //                 implicitHeight: 40
    //                 implicitWidth: 300
    //                 placeholderText: "Search..."
    //                 text: root.filterText
    //                 font.family: Constants.regularFontFamily
    //                 font.pointSize: 11
    //                 onTextChanged: {
    //                     root.filterText = text;
    //                 }
    //                 background: Rectangle {
    //                     color: "#292929"
    //                     radius: 4
    //                 }
    //             }
    //
    //             Item {
    //                 Layout.fillWidth: true
    //                 implicitHeight: 1
    //             }
    //
    //             Button {
    //                 text: root.viewMode === "list" ? "Grid View" : "List View"
    //                 onClicked: {
    //                     root.viewMode = root.viewMode === "list" ? "grid" : "list";
    //                 }
    //             }
    //         }
    //     }
    // }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Pane {
            id: headerThing

            background: Item {}
            Layout.preferredHeight: 48
            Layout.fillWidth: true

            Text {
                height: parent.height
                text: "Category One > Category Two > Category Three"
                color: "#ffffff"
                font.family: Constants.regularFontFamily
                font.pixelSize: AppStyle.fontSizeMedium
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 1
            color: "#ffffff"
            opacity: 0.2
        }

        Loader {
            id: viewLoader

            clip: true

            // anchors.topMargin: 48
            // anchors.fill: parent
            Layout.fillHeight: true
            Layout.fillWidth: true

            sourceComponent: root.viewMode === "grid" ? gridView : listView
            onLoaded: {
                item.currentIndex = Qt.binding(() => root.currentIndex);
            }
        }
    }



    Component {
        id: listView

        GameListView {
            model: gameModel
            onSortRoleChanged: {
                root.sortRole = sortRole;
                root.persistFolderSort();
            }
            onSortAscendingChanged: {
                root.sortAscending = sortAscending;
                root.persistFolderSort();
            }
        }
    }

    Component {
        id: gridView

        GameGridView {
            model: gameModel
        }
    }

    component RoleData: QtObject {
        property int entryId
        property string displayName
        property list<int> folderIds
    }
}