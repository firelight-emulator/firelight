import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Picks a manual folder and adds one or more entries to it. Opened from the
// game context menu and the multi-select bulk bar; acts on `targetIds`
FirelightDialog {
    id: control
    headerText: "Add to folder"
    showButtons: true
    showCancel: true
    acceptText: "Done"

    property var targetIds: []

    function openFor(ids) {
        control.targetIds = ids;
        control.open();
    }

    SortFilterProxyModel {
        id: manualFolders
        model: LibraryFolderModel
        filters: [
            ValueFilter {
                roleName: "folderType"
                value: 0
            }
        ]
        sorters: [
            RoleSorter {
                roleName: "displayName"
            }
        ]
    }

    contentItem: ColumnLayout {
        spacing: AppStyle.spacingSm

        Text {
            Layout.fillWidth: true
            Layout.topMargin: AppStyle.spacingSm
            visible: folderList.count === 0
            text: control.targetIds.length > 1 ? "Adding " + control.targetIds.length + " games. No folders yet — create one from the sidebar." : "No folders yet. Create one from the sidebar."
            color: Theme.textMuted
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeMedium
            wrapMode: Text.WordWrap
        }

        Text {
            Layout.fillWidth: true
            visible: folderList.count > 0 && control.targetIds.length > 1
            text: "Adding " + control.targetIds.length + " games"
            color: Theme.textMuted
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeSmall
        }

        ListView {
            id: folderList
            Layout.fillWidth: true
            Layout.preferredWidth: Math.round(360 * AppStyle.scale)
            Layout.preferredHeight: Math.min(contentHeight, Math.round(320 * AppStyle.scale))
            clip: true
            model: manualFolders
            spacing: 2
            boundsBehavior: Flickable.StopAtBounds

            delegate: ItemDelegate {
                id: folderRow
                required property int folderId
                required property string displayName
                required property string color
                width: ListView.view.width
                height: AppStyle.rowHeight
                padding: AppStyle.spacingSm

                background: Rectangle {
                    color: folderRow.hovered ? Theme.surfaceHover : "transparent"
                    radius: AppStyle.radiusSm
                }

                contentItem: RowLayout {
                    spacing: AppStyle.spacingSm
                    Icon {
                        Layout.preferredWidth: AppStyle.iconSizeSm
                        Layout.preferredHeight: AppStyle.iconSizeSm
                        name: "folder"
                        size: AppStyle.iconSizeSm
                        color: folderRow.color !== "" ? folderRow.color : Theme.textPrimary
                    }
                    Text {
                        Layout.fillWidth: true
                        text: folderRow.displayName
                        color: Theme.textPrimary
                        font.family: AppStyle.fontFamily
                        font.pixelSize: AppStyle.fontSizeMedium
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                onClicked: {
                    for (var i = 0; i < control.targetIds.length; i++) {
                        LibraryEntryModel.addEntryToFolder(control.targetIds[i], folderRow.folderId);
                    }
                    control.close();
                }
            }
        }
    }
}
