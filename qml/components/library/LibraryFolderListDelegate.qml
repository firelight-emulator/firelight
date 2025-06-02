import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0
import QtQml.Models
import Firelight 1.0

FirelightMenuItem {
    id: root
    required property var model
    required property var index

      property string sectionTitle: model.display_name

      labelText: model.display_name
      property bool showGlobalCursor: true

      signal deleted(int folderId)
      signal editClicked(int folderId, string folderName)

      // labelIcon: "\ue40a"
      height: 42
      width: ListView.view.width

      onToggled: {
          if (checked) {
            ListView.view.currentIndex = index
            FilteredLibraryEntryModel.folderId = model.playlist_id
          }
      }

      onRightClicked: {
            folderRightClick.popupNormal()
      }

      RightClickMenu {
          id: folderRightClick

          RightClickMenuItem {
              text: "Rename"

              onTriggered: {
                editClicked(model.playlist_id, model.display_name)
              }
          }

          RightClickMenuItem {
              text: "Delete"
              dangerous: true

              onTriggered: {
                    LibraryFolderModel.deleteFolder(model.playlist_id)
                    if (root.ListView.isCurrentItem) {
                        FilteredLibraryEntryModel.folderId = -1
                        root.deleted(model.playlist_id)
                    }
              }
          }
      }
}