import QtQuick
import QtQuick.Layouts 1.0

FLPage {
    id: root

    // When set, the gallery is pre-filtered to a single game's captures
    property string gameContentHash: ""

    property string typeFilter: "all" // "all" | "screenshot" | "clip"
    property bool showOnlyFavorites: false
    property bool sortAscending: false // newest first by default

    Component.onCompleted: CaptureModel.refresh()

    headerActions: [
        FLSegmentedControl {
            segments: [
                {
                    label: "All",
                    value: "all"
                },
                {
                    label: "Screenshots",
                    value: "screenshot"
                },
                {
                    label: "Clips",
                    value: "clip"
                }
            ]
            currentValue: root.typeFilter
            onActivated: function (value) {
                root.typeFilter = value;
            }
        },
        FLButton {
            text: "Favorites"
            checkable: true
            variant: checked ? "primary" : "default"
            checked: root.showOnlyFavorites
            onClicked: root.showOnlyFavorites = !root.showOnlyFavorites
        },
        FLButton {
            text: root.sortAscending ? "Oldest first" : "Newest first"
            onClicked: root.sortAscending = !root.sortAscending
        }
    ]

    SortFilterProxyModel {
        id: captureProxy
        model: CaptureModel

        filters: [
            ValueFilter {
                roleName: "favorite"
                value: true
                enabled: root.showOnlyFavorites
            },
            ValueFilter {
                roleName: "captureType"
                value: root.typeFilter
                enabled: root.typeFilter !== "all"
            },
            ValueFilter {
                roleName: "gameContentHash"
                value: root.gameContentHash
                enabled: root.gameContentHash !== ""
            }
        ]
        sorters: [
            RoleSorter {
                roleName: "timestamp"
                sortOrder: root.sortAscending ? Qt.AscendingOrder : Qt.DescendingOrder
            }
        ]
    }

    GalleryGridView {
        anchors.fill: parent
        model: captureProxy
    }

    Text {
        anchors.centerIn: parent
        visible: captureProxy.count === 0
        text: root.gameContentHash !== "" ? "No media for this game yet." : "No screenshots or clips yet."
        color: Theme.textMuted
        font.pixelSize: AppStyle.fontSizeMedium
        font.family: AppStyle.fontFamily
    }
}
