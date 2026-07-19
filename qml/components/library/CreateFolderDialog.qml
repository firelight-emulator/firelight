import QtQuick
import QtQuick.Controls
// import QtQuick.Controls.Material
import QtQuick.Layouts

FirelightDialog {
    id: control
    property alias folderName: usernameTextInput.text
    // Pre-fills the field on show (used for rename); "" for a fresh create
    property string initialText: ""
    headerText: "Create folder"

    onAboutToShow: {
        usernameTextInput.text = control.initialText;
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
                        font.pixelSize: AppStyle.fontSizeMedium
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
                        font.pixelSize: AppStyle.fontSizeMedium
                        color: "white"
                        verticalAlignment: Text.AlignVCenter
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
