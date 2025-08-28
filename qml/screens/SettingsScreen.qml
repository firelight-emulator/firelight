import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Pane {
    id: root

    property bool gameRunning: false
    property bool movingDown: true
    property string section

    Component.onCompleted: {
        sectionChanged();
    }

    background: Item{}
    horizontalPadding: AppStyle.windowPadding
    verticalPadding: 0

    FLTwoColumnMenu {
        // KeyNavigation.up: closeButton
        // anchors.bottom: parent.bottom
        // anchors.left: parent.left
        // anchors.leftMargin: 40
        // anchors.right: parent.right
        // anchors.rightMargin: 40
        // anchors.top: headerBar.bottom
        anchors.fill: parent
        menuItems: ["Appearance", "Directories", "Controllers", "Achievements", "Audio / Video", "Platforms", "About"]
        pages: [appearanceSettings, directorySettings, controllerSettings, retroAchievementSettings, videoSettings, platformSettings, about]
    }

    Component {
        id: about

        AboutPage {}
    }


    Component {
        id: appearanceSettings

        AppearanceSettingsPage {
        }
    }
    Component {
        id: platformSettings

        PlatformSettingsPage {
        }
    }
    Component {
        id: directorySettings

        DirectorySettings {
        }
    }
    Component {
        id: soundSettings

        SoundSettings {
        }
    }
    Component {
        id: controllerSettings

        ControllerSettings {
        }
    }
    Component {
        id: librarySettings

        LibrarySettings {
        }
    }
    Component {
        id: notificationSettings

        NotificationSettings {
        }
    }
    Component {
        id: videoSettings

        VideoSettings {
        }
    }
    Component {
        id: retroAchievementSettings

        RetroAchievementSettings {
            gameRunning: root.gameRunning
        }
    }
    Component {
        id: gbSettings

        GameBoySettings {
        }
    }
    Component {
        id: gbcSettings

        GameBoyColorSettings {
        }
    }
    Component {
        id: gbaSettings

        GbaSettings {
        }
    }
    Component {
        id: snesSettings

        SnesSettings {
        }
    }
    Component {
        id: nesSettings

        NesSettings {
        }
    }
    Component {
        id: n64Settings

        Nintendo64Settings {
        }
    }
    Component {
        id: ndsSettings

        NintendoDsSettings {
        }
    }
    Component {
        id: masterSystemSettings

        MasterSystemSettings {
        }
    }
    Component {
        id: genesisSettings

        GenesisSettings {
        }
    }
    Component {
        id: gameGearSettings

        GameGearSettings {
        }
    }

    Component {
        id: debugSettings

        DebugPage {
        }
    }
}