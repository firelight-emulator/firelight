import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root

    title: "Choose a game"
    modal: true
    standardButtons: Dialog.Cancel
    anchors.centerIn: parent
    width: 420
    height: 520

    contentItem: ListView {
        clip: true
        model: LibraryEntryModel
        spacing: 2

        delegate: ItemDelegate {
            width: ListView.view.width
            visible: !model.hidden
            height: model.hidden ? 0 : implicitHeight

            contentItem: RowLayout {
                spacing: 10

                Image {
                    source: model.icon1x1SourceUrl
                    visible: model.icon1x1SourceUrl.length > 0
                    sourceSize.width: 40
                    sourceSize.height: 40
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                }
                Text {
                    text: model.displayName
                    color: Theme.textPrimary
                    font.pixelSize: AppStyle.fontSizeSmall
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
            }

            onClicked: {
                NetworkService.selectGame(model.entryId);
                root.close();
            }
        }

        ScrollBar.vertical: FLScrollBar {}
    }
}
