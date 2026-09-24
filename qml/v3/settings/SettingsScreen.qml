// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FLTwoColumnPage {
    id: root
    objectName: "SettingsScreen|" + root.currentKey

    headerText: "Settings"
    routeBase: "/settings"
    contentMaxWidth: AppStyle.readableContentWidth

    property string highlightKey: ""

    model: [
        {
            "type": "page",
            "key": "controllers",
            "label": "Controllers",
            "iconName": "controller",
            "page": controllerSettingsPage
        },
        {
            "type": "divider"
        },
        {
            "type": "page",
            "key": "video",
            "label": "Video",
            "iconName": "display",
            "page": systemVideoSettings
        },
        {
            "type": "page",
            "key": "audio",
            "label": "Audio",
            "iconName": "display",
            "page": systemAudioSettings
        },
        {
            "type": "divider"
        },
        {
            "type": "header",
            "label": "Gameplay"
        },
        {
            "type": "page",
            "key": "emulation-general",
            "label": "General",
            "iconName": "display",
            "page": placeholderSettings
        },
        {
            "type": "page",
            "key": "emulation-picture",
            "label": "Picture",
            "iconName": "display",
            "page": emulationPictureSettings
        },
        {
            "type": "page",
            "key": "emulation-sound",
            "label": "Sound",
            "iconName": "display",
            "page": emulationSoundSettings
        },
        {
            "type": "page",
            "key": "emulation-input",
            "label": "Input",
            "iconName": "controller",
            "page": emulationInputSettings
        },
        {
            "type": "divider"
        },
        {
            "type": "header",
            "label": "Main stuff"
        },
        {
            "type": "page",
            "key": "retroachievements",
            "label": "Achievements",
            "iconName": "trophy",
            "page": retroAchievementSettings
        },
        {
            "type": "page",
            "key": "about",
            "label": "About",
            "iconName": "info",
            "page": about
        }
    ]

    SettingsSearchModel {
        id: searchModel
    }

    function refreshSearch() {
        if (searchModel.query.trim() === "") {
            root.showCurrentPage();
            return;
        }

        root.showPage(searchResults);
    }

    function openResult(route: string, key: string) {
        if (route === "" || Router.matchedPattern !== root.routeBase) {
            return;
        }

        root.highlightKey = key;
        root.focusContent();
        Router.replace(route);
        root.showCurrentPage();
    }

    function openTopResult() {
        if (searchModel.count > 0) {
            root.openResult(searchModel.topRoute(), searchModel.topKey());
        }
    }

    function describeResult(pageLabel: string, groupLabel: string): string {
        return groupLabel === "" ? pageLabel : pageLabel + " › " + groupLabel;
    }

    header: FLSearchField {
        id: searchField
        Layout.fillWidth: true
        Layout.topMargin: AppStyle.spacingMd + AppStyle.spacingSm
        placeholder: qsTr("Search settings")
        onTextChanged: {
            searchModel.query = searchField.text;
            root.refreshSearch();
        }
        onActiveFocusChanged: {
            if (searchField.activeFocus) {
                root.refreshSearch();
            }
        }
        Keys.onReturnPressed: event => {
            event.accepted = true;
            root.openTopResult();
        }
        Keys.onEnterPressed: event => {
            event.accepted = true;
            root.openTopResult();
        }
    }

    Component {
        id: searchResults

        FocusScope {
            implicitHeight: resultColumn.implicitHeight

            FLFocus.mode: searchModel.count === 0 ? FLFocus.Skip : FLFocus.Normal

            FLColumnLayout {
                id: resultColumn
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 0

                Repeater {
                    model: searchModel

                    delegate: FLMenuItem {
                        id: resultRow
                        required property var model
                        label: resultRow.model.label
                        description: root.describeResult(resultRow.model.pageLabel, resultRow.model.groupLabel)
                        showDescription: !resultRow.model.isPage
                        onClicked: root.openResult(resultRow.model.route, resultRow.model.key)
                    }
                }

                Text {
                    Layout.fillWidth: true
                    Layout.topMargin: AppStyle.spacingLg
                    visible: searchModel.count === 0
                    text: qsTr("No settings found")
                    color: Theme.textMuted
                    horizontalAlignment: Text.AlignHCenter
                    font.family: AppStyle.fontFamily
                    font.pixelSize: AppStyle.fontSizeMedium
                }
            }
        }
    }

    Component {
        id: emulationSettings

        GlobalEmulationSettings {}
    }

    Component {
        id: about

        AboutPage {
            FLFocus.mode: FLFocus.Skip
        }
    }

    Component {
        id: appearanceSettings

        SettingsPage {
            page: "appearance"
        }
    }

    Component {
        id: systemVideoSettings

        SettingsPage {
            page: "video"
        }
    }

    Component {
        id: systemAudioSettings

        SettingsPage {
            page: "audio"
        }
    }

    Component {
        id: controllerSettings

        ControllerSettings {}
    }

    Component {
        id: controllerSettingsPage

        ControllersSettingsPage {}
    }

    Component {
        id: notificationSettings

        SettingsPage {
            page: "notifications"
        }
    }

    Component {
        id: emulationPictureSettings

        SettingsPage {
            page: "emulation-picture"
        }
    }

    Component {
        id: emulationSoundSettings

        SettingsPage {
            page: "emulation-sound"
        }
    }

    Component {
        id: emulationInputSettings

        SettingsPage {
            page: "emulation-input"
        }
    }

    Component {
        id: retroAchievementSettings

        RetroAchievementSettings {
            gameRunning: EmulationService.isGameRunning
        }
    }

    Component {
        id: placeholderSettings

        Item {
            FLFocus.mode: FLFocus.Skip

            Text {
                anchors.centerIn: parent
                text: "Coming soon"
                color: Theme.textMuted
                font.family: AppStyle.fontFamily
                font.pixelSize: AppStyle.fontSizeMedium
            }
        }
    }
}
