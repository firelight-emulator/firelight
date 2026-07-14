import QtQuick
import QtQuick.Controls
import QtQml.Models
import QtQuick.Layouts 1.0
import Firelight 1.0

Item {
    id: root

    // When set, the gallery is pre-filtered to a single game's captures.
    property string gameContentHash: ""

    property string typeFilter: "all" // "all" | "screenshot" | "clip"
    property bool showOnlyFavorites: false
    property bool sortAscending: false // newest first by default

    Component.onCompleted: CaptureModel.refresh()

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

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.margins: 12
            spacing: 8

            Text {
                text: "Media"
                color: Theme.textPrimary
                font.pixelSize: AppStyle.fontSizeLarge
                font.weight: Font.Bold
                font.family: Constants.regularFontFamily
            }

            Item {
                Layout.fillWidth: true
            }

            // Capture-type filter.
            Repeater {
                model: [
                    {label: "All", value: "all"},
                    {label: "Screenshots", value: "screenshot"},
                    {label: "Clips", value: "clip"}
                ]
                delegate: Button {
                    required property var modelData
                    text: modelData.label
                    checkable: true
                    checked: root.typeFilter === modelData.value
                    onClicked: root.typeFilter = modelData.value
                }
            }

            Button {
                text: "Favorites"
                checkable: true
                checked: root.showOnlyFavorites
                onClicked: root.showOnlyFavorites = !root.showOnlyFavorites
            }

            Button {
                text: root.sortAscending ? "Oldest first" : "Newest first"
                onClicked: root.sortAscending = !root.sortAscending
            }
        }

        GalleryGridView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: captureProxy
        }
    }

    Text {
        anchors.centerIn: parent
        visible: captureProxy.count === 0
        text: root.gameContentHash !== "" ? "No media for this game yet."
                                          : "No screenshots or clips yet."
        color: Theme.textMuted
        font.pixelSize: AppStyle.fontSizeMedium
        font.family: Constants.regularFontFamily
    }
}
