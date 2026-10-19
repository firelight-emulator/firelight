import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FLTwoColumnPage {
    id: root
    objectName: "ConfigureButtonMappingsPage"

    required property int profileId

    headerText: qsTr("Configure Controller: Button Mappings")
    contentMaxWidth: AppStyle.readableContentWidth

    model: [
        {
            "type": "page",
            "key": "left-stick1",
            "label": qsTr("Example 1"),
            "page": generalSettingsPage
        },
        {
            "type": "page",
            "key": "right-stick1",
            "label": qsTr("Example 2"),
            "page": shortcutsPage
        }
    ]

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
                model: ShortcutsModel { profileId: 1 }

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