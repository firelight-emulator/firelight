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
    focus: true

    closePolicy: Popup.CloseOnPressOutsideParent | Popup.CloseOnEscape

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
        focusPolicy: Qt.NoFocus
    }

    contentItem: FocusScope {
        width: parent.width
        implicitHeight: contentLayout.implicitHeight

        Keys.onReleased: event => {
            if (event.key === Qt.Key_Back) {
                root.close();
                event.accepted = true;
            }
        }
        FLColumnLayout {
            id: contentLayout
            width: parent.width
            spacing: AppStyle.spacingXs

            SectionLabel {
                text: "VIEW AS"
            }
            FLSegmentedControl {
                focus: true
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
                text: "Grid density"
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
        }
    }
}
