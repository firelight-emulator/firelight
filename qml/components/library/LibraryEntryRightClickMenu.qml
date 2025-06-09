import QtQuick
import QtQuick.Controls
import Firelight 1.0

RightClickMenu {
    id: root

    required property int entryId
    required property bool enableAddToFolder
    required property bool showRemoveFromFolder
    required property bool showManageSaveData

    signal startGameClicked(int entryId)
    signal removeFromFolderClicked(int entryId)
    signal manageSaveDataClicked(int entryId)

    RightClickMenuItem {
        text: "Play"

        onTriggered: {
            startGameClicked(root.entryId)
        }
    }

    // RightClickMenuItem {
    //     text: "View details"
    //     onTriggered: {
    //         Router.navigateTo("/library/entries/" + tttt.model.entryId)
    //     }
    // }

    RightClickMenu {
        id: addToFolderRightClick
        title: "Add to folder"

        enabled: root.enableAddToFolder
        // visible: root.enableAddToFolder

        Instantiator {
            id: ins
            model: LibraryFolderModel
            delegate: RightClickMenuItem {
                text: model.display_name
                onTriggered: {
                    LibraryEntryModel.addEntryToFolder(root.entryId, model.playlist_id)
                    // Add your action here
                }
            }

            onObjectAdded: function (index, object) {
                addToFolderRightClick.insertItem(index, object)
            }
            onObjectRemoved: function (index, object) {
                addToFolderRightClick.removeItem(object)
            }
        }
    }

    RightClickMenuItem {
        text: "Remove from folder"
        enabled: root.showRemoveFromFolder

        onTriggered: {
            removeFromFolderClicked(root.entryId)
        }
    }

    RightClickMenuSeparator {
        width: parent.width
    }

    RightClickMenuItem {
        text: "Manage save data"
        enabled: root.showManageSaveData

        onTriggered: {
            manageSaveDataClicked(root.entryId)
        }
    }

}
