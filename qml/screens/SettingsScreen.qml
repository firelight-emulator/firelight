import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Pane {
    id: root

    property bool gameRunning: false
    property bool movingDown: true

    Component.onCompleted: syncFromRoute()

    // Keep the selected sub-page and the /settings/<section> URL in sync.
    function syncFromRoute() {
        if (!Router.isActive("/settings")) {
            return;
        }
        var m = Router.match(Router.path, ["/settings/:section"]);
        if (m.matched) {
            menu.selectSection(m.params.section);
        } else if (Router.path === "/settings" && menu.currentSection) {
            Router.replace("/settings/" + menu.currentSection);
        }
    }

    Connections {
        target: Router
        function onPathChanged() {
            root.syncFromRoute();
        }
    }

    background: Item{}
    horizontalPadding: AppStyle.windowPadding
    verticalPadding: 16

    FLTwoColumnMenu {
        id: menu
        anchors.fill: parent
        menuItems: ["Appearance", "Directories", "Controllers", "Achievements", "Audio / Video", "Emulation", "Platforms", "About"]
        routeNames: ["appearance", "directories", "controllers", "achievements", "av", "emulation", "platforms", "about"]
        pages: [appearanceSettings, directorySettings, controllerSettings, retroAchievementSettings, videoSettings, emulationSettings, platformSettings, about]

        onCurrentSectionChanged: {
            if (currentSection && Router.isActive("/settings")) {
                var target = "/settings/" + currentSection;
                if (Router.path !== target) {
                    Router.replace(target);
                }
            }
        }
    }

    Component {
        id: emulationSettings

        GlobalEmulationSettings {
        }
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