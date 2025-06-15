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
            value: UserLibrary.mainGameDirectory

            background: Rectangle {
                color: ColorPalette.neutral300
                radius: 6
                // border.color: ColorPalette.neutral700
                opacity: parent.hovered || (!InputMethodManager.usingMouse && parent.activeFocus) ? 0.2 : 0.1

                Behavior on opacity {
                    NumberAnimation {
                        duration: 100
                        easing.type: Easing.InOutQuad
                    }
                }

            }

            onValueChanged: function () {
                UserLibrary.mainGameDirectory = value
            }
        }

        Text {
            font.pixelSize: 15
            font.family: Constants.regularFontFamily
            font.weight: Font.Medium
            color: ColorPalette.neutral100
            text: "This is where you’ll put your game files. Firelight will automatically detect files in this directory and add them to your library."
            leftPadding: 12
            Layout.bottomMargin: 20
        }

        DirectoryOption {
            id: saveDirectoryOption
            KeyNavigation.up: gameDirectoryOption
            Layout.fillWidth: true
            label: "Saves directory"
            value: SaveManager.saveDirectory

            background: Rectangle {
                color: ColorPalette.neutral300
                radius: 6
                // border.color: ColorPalette.neutral700
                opacity: parent.hovered || (!InputMethodManager.usingMouse && parent.activeFocus) ? 0.2 : 0.1

                Behavior on opacity {
                    NumberAnimation {
                        duration: 100
                        easing.type: Easing.InOutQuad
                    }
                }

            }

            onValueChanged: function () {
                SaveManager.saveDirectory = value
            }
        }

        Text {
            font.pixelSize: 15
            font.family: Constants.regularFontFamily
            font.weight: Font.Medium
            color: ColorPalette.neutral100
            text: "This is where Firelight will save your save files and Suspend Point data."
            leftPadding: 12
            Layout.bottomMargin: 20
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

        }
    }
}