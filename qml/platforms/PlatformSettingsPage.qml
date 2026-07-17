import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    id: root

    // One generic, catalog-driven page for every console; pushed with the
    // target platform's id + name.
    Component {
        id: platformPage
        PlatformEmulationSettingsPage {
        }
    }

    readonly property var platforms: [
        { platformId: 1,  name: "Game Boy",           icon: "gb" },
        { platformId: 2,  name: "Game Boy Color",     icon: "gbc" },
        { platformId: 3,  name: "Game Boy Advance",   icon: "gba" },
        { platformId: 5,  name: "NES",                icon: "nes" },
        { platformId: 6,  name: "SNES",               icon: "snes" },
        { platformId: 7,  name: "Nintendo 64",        icon: "n64" },
        { platformId: 10, name: "Nintendo DS",        icon: "ds" },
        { platformId: 12, name: "Master System",      icon: "master-system" },
        { platformId: 13, name: "Genesis/Mega Drive", icon: "genesis" },
        { platformId: 14, name: "Game Gear",          icon: "gamegear" }
    ]

    Flickable {
        anchors.fill: parent
        contentHeight: col.height
        ColumnLayout {
            spacing: 0
            id: col
            width: parent.width

            Text {
                Layout.fillWidth: true
                text: qsTr("Platform settings")
                font.pixelSize: AppStyle.fontSizeLarge
                font.family: Constants.regularFontFamily
                font.weight: Font.Bold
                Layout.bottomMargin: AppStyle.spacingSm
                color: Theme.textPrimary
            }

            Text {
                Layout.fillWidth: true
                text: qsTr("Here's where you can change the default settings for each platform. Later you'll be able to change them on a per-game basis.")
                font.pixelSize: AppStyle.fontSizeMedium
                font.family: Constants.regularFontFamily
                font.weight: Font.Normal
                wrapMode: Text.WordWrap
                color: Theme.textMuted
                Layout.bottomMargin: AppStyle.spacingSm
            }

            Rectangle {
                Layout.topMargin: AppStyle.spacingSm
                Layout.bottomMargin: AppStyle.spacingSm
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: Theme.border
            }

            Repeater {
                model: root.platforms

                delegate: Button {
                    id: platformButton
                    required property var modelData

                    Layout.fillWidth: true
                    Layout.preferredHeight: Math.round(80 * AppStyle.scale)
                    hoverEnabled: true

                    onClicked: root.StackView.view.push(platformPage, {
                        platformId: platformButton.modelData.platformId,
                        platformName: platformButton.modelData.name
                    })

                    background: Rectangle {
                        color: platformButton.hovered ? Theme.surfaceHover : "transparent"
                        radius: AppStyle.radiusMd
                    }
                    contentItem: Row {
                        spacing: AppStyle.spacingSm
                        Image {
                            source: "qrc:images/platform-icons/" + platformButton.modelData.icon
                            width: parent.height
                            height: parent.height
                            fillMode: Image.PreserveAspectFit
                            sourceSize.height: 128
                        }
                        Text {
                            text: platformButton.modelData.name
                            height: parent.height
                            font.pixelSize: AppStyle.fontSizeMedium
                            font.family: Constants.regularFontFamily
                            font.weight: Font.Normal
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignLeft
                            color: Theme.textPrimary
                        }
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }
}
