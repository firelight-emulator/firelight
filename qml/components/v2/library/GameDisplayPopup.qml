import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// TODO
// The library's display menu: view mode, grid size, sort field, and order.
// Reads and writes the sort/view state on the GameView passed as `view`
Popup {
    id: root

    property var view

    // Grid-size presets within the library-tile-size setting's 100..240 range
    readonly property var _sizes: [
        {
            label: "Small",
            px: 110
        },
        {
            label: "Medium",
            px: 160
        },
        {
            label: "Large",
            px: 210
        }
    ]
    readonly property int _sizeIndex: AppearanceSettings.libraryTileSize < 135 ? 0 : AppearanceSettings.libraryTileSize < 185 ? 1 : 2

    width: Math.round(280 * AppStyle.scale)
    padding: AppStyle.spacingMd
    modal: false
    focus: true
    closePolicy: Popup.CloseOnEscape

    background: Rectangle {
        color: Theme.surfaceElevated
        radius: AppStyle.radiusMd
        border.color: Theme.border
        border.width: 1
    }

    // TODO
    // Small-caps section heading
    component SectionLabel: Text {
        color: Theme.textMuted
        font.family: Constants.regularFontFamily
        font.pixelSize: AppStyle.fontSizeSmall
        font.weight: Font.DemiBold
        Layout.topMargin: AppStyle.spacingXs
    }

    ColumnLayout {
        width: parent.width
        spacing: AppStyle.spacingSm

        SectionLabel {
            text: "VIEW"
        }
        FLSegmentedControl {
            Layout.fillWidth: true
            segments: [
                {
                    label: "Grid",
                    value: "grid"
                },
                {
                    label: "List",
                    value: "list"
                }
            ]
            currentValue: root.view.viewMode
            onActivated: function (value) {
                root.view.viewMode = value;
            }
        }

        SectionLabel {
            text: "GRID SIZE"
        }
        FLSegmentedControl {
            Layout.fillWidth: true
            segments: root._sizes.map(function (s) {
                return {
                    label: s.label,
                    value: String(s.px)
                };
            })
            currentValue: String(root._sizes[root._sizeIndex].px)
            onActivated: function (value) {
                AppearanceSettings.tileSizeBinding.value = value;
            }
        }

        SectionLabel {
            text: "SORT BY"
        }
        Repeater {
            model: root.view.sortOptions
            delegate: FLListRow {
                required property var modelData
                Layout.fillWidth: true
                label: modelData.label
                highlighted: root.view.sortRole === modelData.role
                onClicked: {
                    root.view.sortRole = modelData.role;
                    root.view.persistFolderSort();
                }
                Icon {
                    visible: root.view.sortRole === modelData.role
                    name: "check"
                    size: AppStyle.iconSizeSm
                    color: Theme.accent
                }
            }
        }

        SectionLabel {
            text: "ORDER"
        }
        FLSegmentedControl {
            Layout.fillWidth: true
            segments: [
                {
                    label: "Ascending",
                    value: "asc"
                },
                {
                    label: "Descending",
                    value: "desc"
                }
            ]
            currentValue: root.view.sortAscending ? "asc" : "desc"
            onActivated: function (value) {
                root.view.sortAscending = value === "asc";
                root.view.persistFolderSort();
            }
        }
    }
}
