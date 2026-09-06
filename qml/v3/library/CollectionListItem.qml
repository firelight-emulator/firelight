import QtQuick
import Firelight 1.0

FLListRow {
    id: root

    required property int folderId
    required property string displayName
    required property int folderType

    readonly property int gameCount: {
        const held = LibraryEntryModel.countByFolderId[String(root.folderId)];
        return held === undefined ? 0 : held;
    }

    readonly property bool isSmart: root.folderType === 1

    signal opened

    objectName: "CollectionListItem|" + root.displayName

    iconName: root.isSmart ? "bookmark-star" : "folder"
    label: root.displayName

    onClicked: root.opened()

    Text {
        text: root.gameCount === 1 ? "1 game" : root.gameCount + " games"
        color: Theme.textMuted
        font.family: AppStyle.fontFamily
        font.pixelSize: AppStyle.fontSizeSmall
    }
}
