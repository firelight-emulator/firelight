import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// TODO
// The library view's header: scope breadcrumb, title + count + clear, and the
// multi-select bulk-action bar. Reads scope/selection state on the GameView `view`
ColumnLayout {
    id: root

    property var view

    spacing: AppStyle.spacingSm

    // Breadcrumb (folder scopes only)
    RowLayout {
        Layout.fillWidth: true
        visible: root.view.scopeCrumb.length > 0
        spacing: AppStyle.spacingXs

        Text {
            text: "Library"
            color: Theme.textMuted
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeSmall
        }
        Repeater {
            model: root.view.scopeCrumb
            delegate: RowLayout {
                required property var modelData
                required property int index
                spacing: AppStyle.spacingXs
                Icon {
                    name: "chevron-forward"
                    size: AppStyle.iconSizeSm
                    color: Theme.textMuted
                }
                Text {
                    text: parent.modelData.label
                    color: parent.index === root.view.scopeCrumb.length - 1 ? Theme.textPrimary : Theme.textMuted
                    font.family: AppStyle.fontFamily
                    font.pixelSize: AppStyle.fontSizeSmall
                    font.weight: Font.DemiBold
                    TapHandler {
                        onTapped: root.view.folderCrumbClicked(parent.modelData.folderId)
                    }
                    HoverHandler {
                        cursorShape: Qt.PointingHandCursor
                    }
                }
            }
        }
    }

    // Title + count + clear
    RowLayout {
        Layout.fillWidth: true
        spacing: AppStyle.spacingSm

        Text {
            text: root.view.scopeLabel
            color: Theme.textPrimary
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeLarge
            font.weight: Font.Bold
        }
        Text {
            Layout.alignment: Qt.AlignBaseline
            text: root.view.gameCount + (root.view.gameCount === 1 ? " game" : " games")
            color: Theme.textMuted
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeMedium
        }
        Item {
            Layout.fillWidth: true
        }
        FLButton {
            visible: root.view.isDirty
            compact: true
            variant: "subtle"
            text: "Clear filters"
            onClicked: root.view.clearAll()
        }
    }

    // Bulk action bar — appears while games are multi-selected
    RowLayout {
        Layout.fillWidth: true
        visible: root.view.selectedCount > 0
        spacing: AppStyle.spacingSm

        Text {
            text: root.view.selectedCount + " selected"
            color: Theme.accent
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeMedium
            font.weight: Font.DemiBold
        }
        FLButton {
            compact: true
            variant: "default"
            iconName: "favorite"
            text: "Favorite"
            onClicked: root.view.bulkFavorite(true)
        }
        FLButton {
            compact: true
            variant: "default"
            text: "Add to folder…"
            onClicked: root.view.openAddToFolder()
        }
        FLButton {
            compact: true
            variant: "default"
            text: "Remove from folder"
            visible: root.view.removableFolderId !== -1
            onClicked: root.view.bulkRemoveFromFolder()
        }
        Item {
            Layout.fillWidth: true
        }
        FLButton {
            compact: true
            variant: "subtle"
            text: "Clear selection"
            onClicked: root.view.clearSelection()
        }
    }
}
