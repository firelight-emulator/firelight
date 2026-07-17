import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// Global emulation defaults (the base tier). Consoles override these per-core,
// and individual games override those. With no platform in scope only the
// frontend settings apply, since per-core options don't mean anything globally.
FocusScope {
    id: root

    implicitWidth: col.width
    implicitHeight: col.implicitHeight

    ColumnLayout {
        id: col
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 0

        Text {
            Layout.fillWidth: true
            Layout.bottomMargin: 4
            text: qsTr("Global defaults for all games. Consoles and individual games can override these.")
            font.pixelSize: AppStyle.fontSizeSmall
            font.family: Constants.regularFontFamily
            wrapMode: Text.WordWrap
            color: Theme.textMuted
        }

        SettingsPage {
            Layout.fillWidth: true
            page: "emulation"
            level: SettingsLevel.Global
        }
    }
}
