import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// TODO
// The library's display menu: view mode, grid size, sort field, and order.
// Reads and writes the sort/view state on the GameView passed as `view`
FLPopup {
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

    // TODO
    // Small-caps section heading
    component SectionLabel: Text {
        color: Theme.textMuted
        font.family: AppStyle.fontFamily
        font.pixelSize: AppStyle.fontSizeXSmall
        font.weight: Font.Medium
        font.capitalization: Font.AllUppercase
        font.letterSpacing: 0.5
        Layout.topMargin: AppStyle.spacingXs
    }

    ColumnLayout {
        width: parent.width
        spacing: AppStyle.spacingXs

        SectionLabel {
            text: "VIEW"
        }
        FLSegmentedControl {
            Layout.fillWidth: true
            segments: [
                {
                    icon: "view_list",
                    label: "List",
                    value: "list"
                },
                {
                    icon: "grid_view",
                    label: "Grid",
                    value: "grid"
                }
            ]

            currentValue: root.view.viewMode
            onActivated: function (value) {
                root.view.viewMode = value;
            }
        }

        SectionLabel {
            text: "Grid size"
            visible: root.view.viewMode === "grid"
        }
        FLSegmentedControl {
            Layout.fillWidth: true
            visible: root.view.viewMode === "grid"
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
            text: "Grid spacing"
            visible: root.view.viewMode === "grid"
        }
        FLSlider {
            Layout.fillWidth: true
            visible: root.view.viewMode === "grid"
            from: 0
            to: 100
            stepSize: 2
            value: 0
            onValueChanged: function (value) {
                console.log("value: " + value);
                AppearanceSettings.tileSpacingBinding.value = value;
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Theme.border
            Layout.topMargin: AppStyle.spacingSm
            Layout.bottomMargin: AppStyle.spacingXs
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

        // Rectangle {
        //     Layout.fillWidth: true
        //     Layout.preferredHeight: 1
        //     color: Theme.border
        //     Layout.topMargin: AppStyle.spacingSm
        //     Layout.bottomMargin: AppStyle.spacingXs
        // }

        // SectionLabel {
        //     text: "GROUP BY"
        // }
        // Repeater {
        //     model: root.view.groupOptions
        //     delegate: FLListRow {
        //         required property var modelData
        //         Layout.fillWidth: true
        //         label: modelData.label
        //         highlighted: root.view.groupBy === modelData.value
        //         onClicked: root.view.groupBy = modelData.value
        //         Icon {
        //             visible: root.view.groupBy === modelData.value
        //             name: "check"
        //             size: AppStyle.iconSizeSm
        //             color: Theme.accent
        //         }
        //     }
        // }
    }
}
