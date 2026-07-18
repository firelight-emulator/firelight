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
    property alias text: displayNameTextInput.text
    headerText: "Edit library entry"

    contentItem: RowLayout {
        spacing: 16
        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Pane {
                 id: thePane
                 Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                 Layout.fillWidth: true
                 Layout.preferredHeight: 48
                 background: Rectangle {
                     color: ColorPalette.neutral800
                     radius: 4
                 }

                 focus: true

                 HoverHandler {
                     acceptedDevices: PointerDevice.Mouse
                     cursorShape: Qt.IBeamCursor
                 }

                 contentItem: Item {
                     Text {
                         anchors.fill: parent
                          font.pixelSize: AppStyle.fontSizeMedium
                         font.family: Constants.regularFontFamily
                         color: ColorPalette.neutral500
                         text: "Display name"
                         verticalAlignment: Text.AlignVCenter
                         visible: displayNameTextInput.length === 0
                     }
                     TextInput {
                         id: displayNameTextInput
                         anchors.fill: parent
                         activeFocusOnTab: true
                         // KeyNavigation.down: passwordTextInput
                         property bool showGlobalCursor: true
                         property var globalCursorProxy: thePane
                         font.family: Constants.regularFontFamily
                         focus: true
                          font.pixelSize: AppStyle.fontSizeMedium
                         color: "white"
                         verticalAlignment: Text.AlignVCenter

                     }
                 }
             }
         }
     }
}
