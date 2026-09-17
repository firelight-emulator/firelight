import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FocusScope {
    id: root

    focus: true

    signal collectionOpened(int collectionId)
    signal newCollectionRequested

    function focusFirstItem() {
        browse.focusFirstItem();
    }

    function enterFocus() {
        browse.enterFocus();
    }

    FLBrowseView {
        id: browse
        anchors.fill: parent

        isEmpty: LibraryFolderModel.count === 0

        gridComponent: collectionGrid
        listComponent: collectionList
        emptyComponent: noCollectionsView

        FLIconButton {
            id: newButton
            Layout.alignment: Qt.AlignHCenter
            iconName: "new-folder"
            tooltipText: "Create collection"
            filled: false
            compact: false
            onClicked: Router.navigate("/library/create-collection")
        }

        FLIconButton {
            id: reorderButton
            Layout.alignment: Qt.AlignHCenter
            iconName: "open_with"
            tooltipText: "Reorder collections"
            filled: false
            compact: false
            onClicked: Router.navigate("/library/reorder-collections")
            canInteract: !browse.isEmpty
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }

    Component {
        id: collectionGrid

        FLGridView {
            id: gridView
            model: LibraryFolderModel
            clip: true
            cellWidth: AppearanceSettings.libraryIconGridTileSize + Math.round(AppearanceSettings.libraryIconGridTileSpacing)
            cellHeight: cellWidth + 50

            delegate: CollectionTile {
                width: GridView.view.cellWidth
                height: GridView.view.cellHeight

                onClicked: root.collectionOpened(folderId)
            }

            header: Pane {
                height: AppStyle.gameViewHeaderHeight
                width: GridView.view.width
                verticalPadding: AppStyle.spacingXs
                leftPadding: AppStyle.spacingSm
                rightPadding: AppStyle.spacingSm
                background: Item {}
            }

            footer: Item {
                height: 48
                width: GridView.view.width
            }
        }
    }

    Component {
        id: collectionList

        FLListView {
            model: LibraryFolderModel
            clip: true
            spacing: AppStyle.spacingXs

            delegate: CollectionListItem {
                width: ListView.view.width

                onOpened: root.collectionOpened(folderId)
            }
        }
    }

    Component {
        id: noCollectionsView

        FLColumnLayout {
            spacing: AppStyle.spacingXl

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            Text {
                Layout.preferredWidth: 400
                Layout.alignment: Qt.AlignHCenter
                text: "You don't have any collections yet"
                color: Theme.textPrimary
                font.family: AppStyle.fontFamily
                font.pixelSize: AppStyle.fontSizeMedium
                horizontalAlignment: Text.AlignHCenter
            }

            FLButton {
                text: "Create Collection"
                Layout.alignment: Qt.AlignHCenter

                FLFocus.focusSound: SoundEffects.menuNavigate
                FLFocus.actions: [
                    FLAction {
                        keys: [Qt.Key_Select, Qt.Key_Return, Qt.Key_Enter, Qt.Key_Space]
                        label: qsTr("Select")
                        sound: SoundEffects.openPopup
                        onTriggered: root.newCollectionRequested()
                    }
                ]

                onClicked: root.newCollectionRequested()
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }
}
