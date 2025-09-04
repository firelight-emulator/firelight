import QtQuick
import QtQuick.Controls
// import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

FirelightDialog {
    id: control
    property alias folderName: usernameTextInput.text
    headerText: "Create folder"

    onAboutToShow: {
        usernameTextInput.text = ""
    }

    contentItem: FocusScope {
        focus: true
        ColumnLayout {
            anchors.fill: parent
        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
        Pane {
          id: thePane
          Layout.fillWidth: true
          implicitHeight: 48
          property var globalCursorSpacing: 0

          background: Rectangle {
              color: ColorPalette.neutral800
              radius: 4
          }

          HoverHandler {
              acceptedDevices: PointerDevice.Mouse
              cursorShape: Qt.IBeamCursor
          }

          focus: true

          contentItem: FocusScope {
              focus: true
              Text {
                  anchors.fill: parent
                   font.pixelSize: 16
                  font.family: Constants.regularFontFamily
                  color: ColorPalette.neutral500
                  text: "Folder name"
                  verticalAlignment: Text.AlignVCenter
                  visible: usernameTextInput.length === 0
              }
              TextInput {
                  id: usernameTextInput
                  anchors.fill: parent
                  activeFocusOnTab: true
                  // KeyNavigation.down: passwordTextInput
                  property bool showGlobalCursor: true
                  property var globalCursorProxy: thePane
                  font.family: Constants.regularFontFamily
                  focus: true
                   font.pixelSize: 16
                  color: "white"
                  verticalAlignment: Text.AlignVCenter

                  // onAccepted: {
                  //     if (submitButton.enabled) {
                  //         submitButton.clicked()
                  //     }
                  // }
              }
          }
      }
        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
  }
  }
}