import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// Global emulation defaults (the base tier). Consoles override these per-core,
// and individual games override those. Only shows the common (frontend) settings
// since per-core options don't apply globally.
FocusScope {
    id: root

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Text {
            Layout.fillWidth: true
            text: qsTr("Emulation")
            font.pixelSize: 26
            font.family: Constants.regularFontFamily
            font.weight: Font.Bold
            color: "white"
        }

        Text {
            Layout.fillWidth: true
            text: qsTr("Global defaults for all games. Consoles and individual games can override these.")
            font.pixelSize: 15
            font.family: Constants.regularFontFamily
            font.weight: Font.Normal
            wrapMode: Text.WordWrap
            color: ColorPalette.neutral300
            Layout.bottomMargin: 8
        }

        EmulationSettingsSurface {
            Layout.fillWidth: true
            Layout.fillHeight: true
            level: 2 // Global tier
        }
    }
}
