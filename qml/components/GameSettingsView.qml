import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FocusScope {
    id: root

    // 0 = Game (this game's overrides), 1 = Platform (all games on this console).
    property var level: 0
    required property var contentHash
    required property var platformId
    required property var platformName

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        TabBar {
            Layout.alignment: Qt.AlignCenter
            currentIndex: root.level
            onCurrentIndexChanged: {
                if (root.level !== currentIndex) {
                    root.level = currentIndex
                }
            }

            TabButton {
                width: 120
                text: "Game"
            }
            TabButton {
                width: 120
                text: "Platform"
            }
        }

        Text {
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: parent.width * 0.75
            font.pixelSize: 15
            font.family: Constants.regularFontFamily
            font.weight: Font.Medium
            color: Theme.textPrimary
            text: root.level === 0 ? "Settings are applied only to the current game" : "Settings are applied to all " + root.platformName + " games"
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            lineHeight: 1.2
        }

        CoreSelector {
            Layout.fillWidth: true
            Layout.leftMargin: 12
            Layout.rightMargin: 12
            Layout.bottomMargin: 8
            level: root.level
            platformId: root.platformId
            contentHash: root.contentHash
        }

        EmulationSettingsSurface {
            Layout.fillWidth: true
            Layout.fillHeight: true
            level: root.level
            contentHash: root.contentHash
            platformId: root.platformId
        }
    }
}
