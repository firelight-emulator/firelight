import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FLTwoColumnPage {
    id: root
    objectName: "ConfigureAllControllersPage"

    headerText: qsTr("Configure Controllers")
    menuWidth: 400
    contentMaxWidth: AppStyle.readableContentWidth
    routeBase: "/controllers/configure"

    model: ControllerProfileModel.getAllProfiles().map(profile => {
        return {
            "type": "page",
            "key": String(profile.id),
            "label": profile.name,
            "page": settingsPage
        }
    })

    Component {
        id: settingsPage

        FLColumnLayout {
            spacing: AppStyle.spacingMd

            LinkItem {
                label: qsTr("Button Mappings")
                route: "/controllers/configure/1/button-mappings"
                description: "Configure button mappings for the different platforms"
            }

            LinkItem {
                label: qsTr("Shortcuts")
                route: "/controllers/configure/1/shortcuts"
                description: "Set up shortcuts for quick access to common actions"
            }

            LinkItem {
                label: qsTr("Analog Stick Settings")
                route: "/controllers/configure/1/analog-sensors"
                description: "Adjust deadzones and sensitivity for the analog sticks"
            }

            FLDivider {}

            LinkItem {
                label: qsTr("Test Controller")
                route: "/controllers/configure/1/test-controller"
            }

            FLDivider {}

            SettingsGroup {
                group: "controller-rumble-lights"
                showHeader: false
                profileId: 1
            }

            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
            }


        }
    }

    component LinkItem: FLMenuItem {
        required property string route
        showDescription: true

        controlItem: Icon {
            name: "chevron-forward"
            size: AppStyle.iconSizeSm
            color: Theme.borderStrong
            weight: Font.DemiBold
        }

        onClicked: function () {
            Router.navigate(route);
        }
    }

    // model: [
    //     {
    //         "type": "page",
    //         "key": "suspend-points",
    //         "label": qsTr("General Settings"),
    //         "page": generalSettingsPage
    //     },
    //     {
    //         "type": "page",
    //         "key": "suspend-points2",
    //         "label": qsTr("Shortcuts"),
    //         "page": shortcutsPage
    //     },
    //     {
    //         "type": "page",
    //         "key": "suspend-points3",
    //         "label": qsTr("Analog Stick Settings"),
    //         "page": analogSticksPage
    //     },
    //     {
    //         "type": "page",
    //         "key": "suspend-points5",
    //         "label": qsTr("Button Mappings"),
    //         "page": temp
    //     }
    // ]

    Component {
        id: temp

        Text {
            text: "placeholder"
            color: "white"
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeMedium
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    Component {
        id: generalSettingsPage

        SettingsGroup {
            group: "controller-rumble-lights"
            showHeader: false
            profileId: 1
        }
    }

    Component {
        id: analogSticksPage

        AnalogTuningPage {
            profileId: 1
        }
    }

    Component {
        id: shortcutsPage

        FLPage {
            GamepadProfile {
                id: profile
                profileId: 1
            }

            FLListView {
                anchors.fill: parent
                model: profile.shortcutsModel

                delegate: FLMenuItem {
                    required property var model
                    required property int index

                    width: ListView.view.width
                    label: model.name
                    controlItem: Text {
                        text: "hiya"
                        color: Theme.textPrimary
                        font.pixelSize: AppStyle.fontSizeSmall
                        font.family: AppStyle.fontFamily
                        font.weight: Font.Medium
                        verticalAlignment: Text.AlignVCenter
                    }
                    // onClicked: {
                    //     profile.editShortcut(modelData.id)
                    // }
                }
            }
        }
    }
}