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
      height: 48
      width: ListView.view.width

      onToggled: {
          if (checked) {
            ListView.view.currentIndex = index
            FilteredLibraryEntryModel.folderId = model.playlist_id
          }
      }
}