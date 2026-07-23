import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// TODO
// Dev-only living catalog of the FL* components. Reachable via F11 (see Main4),
// not in the nav. Add a component's variants/states here when you build it
FLPage {
    id: root

    background: Rectangle {
        color: Theme.background
    }

    headerActions: [
        FLIconButton {
            iconName: "close"
            tooltipText: "Close (F11)"
            onClicked: Router.back()
        }
    ]

    FLScrollView {
        id: scroll
        anchors.fill: parent

        Item {
            width: scroll.availableWidth
            implicitHeight: col.implicitHeight + AppStyle.windowPadding * 2

            ColumnLayout {
                id: col
                x: AppStyle.windowPadding
                y: AppStyle.windowPadding
                width: parent.width - AppStyle.windowPadding * 2
                spacing: AppStyle.spacingLg

                Text {
                    text: "Component Gallery"
                    color: Theme.textPrimary
                    font.family: AppStyle.fontFamily
                    font.pixelSize: AppStyle.fontSizeXLarge
                    font.weight: Font.DemiBold
                }

                Text {
                    text: "F11 or Esc to close  ·  color ← Theme, size ← AppStyle, reuse FL*"
                    color: Theme.textMuted
                    font.family: AppStyle.fontFamily
                    font.pixelSize: AppStyle.fontSizeSmall
                }

                //****************
                // Buttons
                //****************
                FLSectionHeader {
                    text: "Buttons"
                }
                Flow {
                    Layout.fillWidth: true
                    spacing: AppStyle.spacingSm
                    FLButton {
                        text: "Primary"
                        variant: "primary"
                    }
                    FLButton {
                        text: "Default"
                    }
                    FLButton {
                        text: "Danger"
                        variant: "danger"
                    }
                    FLButton {
                        text: "Subtle"
                        variant: "subtle"
                    }
                    FLButton {
                        text: "Leading icon"
                        iconName: "save"
                        variant: "primary"
                    }
                    FLButton {
                        text: "Trailing"
                        trailingIconName: "expand_more"
                    }
                    FLButton {
                        text: "Compact"
                        compact: true
                    }
                    FLButton {
                        text: "Disabled"
                        enabled: false
                    }
                }
                Flow {
                    Layout.fillWidth: true
                    spacing: AppStyle.spacingSm
                    FLIconButton {
                        iconName: "settings"
                        tooltipText: "Settings"
                    }
                    FLIconButton {
                        iconName: "favorite"
                        checkable: true
                    }
                    FLIconButton {
                        iconName: "grid_view"
                        checkable: true
                        checked: true
                    }
                    FLIconButton {
                        iconName: "close"
                        compact: true
                    }
                    FLIconButton {
                        iconName: "delete"
                        enabled: false
                    }
                }
                FLSegmentedControl {
                    segments: [
                        {
                            label: "Grid",
                            value: "grid"
                        },
                        {
                            label: "List",
                            value: "list"
                        },
                        {
                            label: "Detail",
                            value: "detail"
                        }
                    ]
                    currentValue: "grid"
                    onActivated: function (value) {
                        currentValue = value;
                    }
                }

                //****************
                // Inputs
                //****************
                FLSectionHeader {
                    text: "Inputs"
                }
                Flow {
                    Layout.fillWidth: true
                    spacing: AppStyle.spacingMd
                    FLTextField {
                        width: Math.round(240 * AppStyle.scale)
                        placeholderText: "Text field"
                    }
                    FLSearchField {
                        width: Math.round(240 * AppStyle.scale)
                    }
                    FLComboBox {
                        width: Math.round(200 * AppStyle.scale)
                        model: ["Option A", "Option B", "Option C"]
                    }
                }
                RowLayout {
                    spacing: AppStyle.spacingMd
                    FLToggle {}
                    FLToggle {
                        checked: true
                    }
                    Text {
                        text: "Toggle (off / on)"
                        color: Theme.textMuted
                        font.family: AppStyle.fontFamily
                        font.pixelSize: AppStyle.fontSizeSmall
                    }
                }
                FLSlider {
                    Layout.preferredWidth: Math.round(280 * AppStyle.scale)
                    from: 0
                    to: 100
                    value: 40
                }

                //****************
                // Surfaces
                //****************
                FLSectionHeader {
                    text: "Surfaces"
                }
                RowLayout {
                    spacing: AppStyle.spacingMd
                    FLPanel {
                        variant: "solid"
                        Text {
                            text: "Solid panel"
                            color: Theme.textPrimary
                            font.family: AppStyle.fontFamily
                            font.pixelSize: AppStyle.fontSizeMedium
                        }
                    }
                    FLPanel {
                        variant: "glass"
                        Text {
                            text: "Glass panel"
                            color: Theme.textPrimary
                            font.family: AppStyle.fontFamily
                            font.pixelSize: AppStyle.fontSizeMedium
                        }
                    }
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.maximumWidth: Math.round(420 * AppStyle.scale)
                    spacing: 0
                    FLListRow {
                        Layout.fillWidth: true
                        iconName: "folder"
                        label: "List row with a trailing badge"
                        FLBadge {
                            text: "3"
                        }
                    }
                    FLDivider {}
                    FLListRow {
                        Layout.fillWidth: true
                        iconName: "star"
                        label: "Another list row"
                    }
                }

                //****************
                // Data display
                //****************
                FLSectionHeader {
                    text: "Data display"
                }
                Flow {
                    Layout.fillWidth: true
                    spacing: AppStyle.spacingSm
                    FLBadge {
                        text: "Neutral"
                    }
                    FLBadge {
                        text: "Accent"
                        tone: "accent"
                    }
                    FLBadge {
                        text: "Success"
                        tone: "success"
                    }
                    FLBadge {
                        text: "Warning"
                        tone: "warning"
                    }
                    FLBadge {
                        text: "Danger"
                        tone: "danger"
                    }
                }
                Flow {
                    Layout.fillWidth: true
                    spacing: AppStyle.spacingMd
                    Repeater {
                        model: ["home", "settings", "favorite", "search", "download", "delete", "play_arrow", "info"]
                        Icon {
                            required property string modelData
                            name: modelData
                            size: AppStyle.iconSizeMd
                            color: Theme.textPrimary
                        }
                    }
                }
                FLEmptyState {
                    Layout.preferredWidth: Math.round(320 * AppStyle.scale)
                    iconName: "search_off"
                    title: "Nothing here"
                    subtitle: "An empty-state placeholder"
                }

                //****************
                // Library
                //****************
                FLSectionHeader {
                    text: "Library"
                }
                RowLayout {
                    spacing: AppStyle.spacingLg
                    GameTile {
                        size: Math.round(150 * AppStyle.scale)
                        platformId: 1
                        title: "Game Tile"
                        titleVisible: true
                    }
                    FLPlatformIcon {
                        width: Math.round(56 * AppStyle.scale)
                        height: width
                        platformId: 1
                    }
                }

                Item {
                    Layout.preferredHeight: AppStyle.spacingXl
                }
            }
        }
    }
}
