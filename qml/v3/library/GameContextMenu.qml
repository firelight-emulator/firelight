import QtQuick
import Firelight 1.0

FLMenu {
    id: control

    required property var entry

    FLMenuItem {
        label: qsTr("Play")
    }

    FLMenuSeparator {}

    FLMenuItem {
        label: "View details"
    }

    FLMenuItem {
        label: control.entry.favorite ? "Remove from favorites" : "Add to favorites"
        onClicked: {
            control.entry.favorite = !control.entry.favorite;
            FLDimmer.redrawCaller();
        }
    }

    FLToggleMenuItem {
        label: "Hidden"
        checked: control.entry.hidden
        onSelected: function (selected) {
            control.entry.hidden = selected;
        }
    }

    FLMenuSeparator {}

    FLSubmenuItem {
        label: "Collections"
        model: LibraryFolderModel.manualFolders
        textRole: "displayName"
        valueRole: "folderId"

        selectionIsExternal: true
        currentValues: control.entry.folderIds

        onOptionToggled: function (value, selected) {
            if (selected) {
                LibraryEntryModel.addEntryToFolder(control.entry.entryId, value);
            } else {
                LibraryEntryModel.removeEntryFromFolder(control.entry.entryId, value);
            }
        }
    }

    FLMenuSeparator {}

    FLMenuItem {
        label: "Manage game"
    }
}
