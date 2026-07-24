import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// TODO
// The library view's header: scope breadcrumb, the scope identity block (icon +
// name over a description/count line) with a clear-filters action, and the
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

    // Scope identity: icon chip + name over a muted description/count line
    RowLayout {
        Layout.fillWidth: true
        spacing: AppStyle.spacingMd

        // TODO
        // Icon chip: platform logo, custom folder art, or a glyph fallback
        Rectangle {
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: AppStyle.iconSizeLg
            Layout.preferredHeight: AppStyle.iconSizeLg
            radius: AppStyle.radiusMd
            color: Qt.rgba(root.view.scopeIconColor.r, root.view.scopeIconColor.g, root.view.scopeIconColor.b, 0.14)

            FLPlatformIcon {
                anchors.centerIn: parent
                visible: root.view.scopePlatformId >= 0
                platformId: root.view.scopePlatformId
                width: AppStyle.iconSizeMd
                height: AppStyle.iconSizeMd
            }
            Image {
                anchors.centerIn: parent
                visible: root.view.scopePlatformId < 0 && root.view.scopeIconUrl !== ""
                width: AppStyle.iconSizeMd
                height: AppStyle.iconSizeMd
                fillMode: Image.PreserveAspectFit
                sourceSize: Qt.size(width * 2, height * 2)
                source: root.view.scopeIconUrl
            }
            Icon {
                anchors.centerIn: parent
                visible: root.view.scopePlatformId < 0 && root.view.scopeIconUrl === ""
                name: root.view.scopeIconName
                filled: false
                size: AppStyle.iconSizeMd
                color: root.view.scopeIconColor
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 0

            Text {
                Layout.fillWidth: true
                text: root.view.scopeLabel
                color: Theme.textPrimary
                font.family: AppStyle.fontFamily
                font.pixelSize: AppStyle.fontSizeLarge
                font.weight: Font.Bold
                elide: Text.ElideRight
            }
            Text {
                Layout.fillWidth: true
                text: {
                    var count = root.view.gameCount + (root.view.gameCount === 1 ? " game" : " games");
                    return root.view.scopeDescription !== "" ? root.view.scopeDescription + " · " + count : count;
                }
                color: Theme.textMuted
                font.family: AppStyle.fontFamily
                font.pixelSize: AppStyle.fontSizeMedium
                elide: Text.ElideRight
            }
        }

        FLButton {
            Layout.alignment: Qt.AlignVCenter
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
