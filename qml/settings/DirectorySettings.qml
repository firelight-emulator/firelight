import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform

FocusScope {
    ColumnLayout {
        spacing: 8
        anchors.fill: parent

        DirectoryOption {
            id: gameDirectoryOption
            Layout.fillWidth: true
            label: "Game directory"
            focus: true
            description: "This is where you’ll put your game files. Firelight will automatically detect files in this directory and add them to your library."
            value: UserLibrary.mainGameDirectory

            onValueChanged: function () {
                UserLibrary.mainGameDirectory = value
            }
        }
        DirectoryOption {
            id: saveDirectoryOption
            KeyNavigation.up: gameDirectoryOption
            Layout.fillWidth: true
            label: "Saves directory"
            description: "This is where Firelight will save your save files and Suspend Point data."
            value: SaveManager.saveDirectory

            onValueChanged: function () {
                SaveManager.saveDirectory = value
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

        }
    }
}