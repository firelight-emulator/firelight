import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// TODO
// The library's display menu: view mode, grid size, sort field, and order.
// Reads and writes the sort/view state on the GameView passed as `view`
FLPopup {
    id: root

    property var view

    width: Math.round(280 * AppStyle.scale)
    padding: AppStyle.spacingMd
    modal: false
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

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
                    if (root.view.sortRole === modelData.role) {
                        root.view.sortAscending = !root.view.sortAscending;
                    } else {
                        root.view.sortRole = modelData.role;
                        root.view.sortAscending = true;
                        root.view.persistFolderSort();
                    }

                }
                Icon {
                    visible: root.view.sortRole === modelData.role
                    name: root.view.sortAscending ? "arrow-down" : "arrow-up"
                    size: AppStyle.iconSizeSm
                    color: Theme.accent
                }
            }
        }
    }
}
