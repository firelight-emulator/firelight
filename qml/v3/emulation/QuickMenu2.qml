import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FLPage {
    id: root

    // property Component tabsHeader: Component {
    //     NavigationTabBar {
    //         tabs: ["Games", "Collections"]
    //
    //         currentIndex: root.libraryTab
    //
    //         clickAction: function () {
    //             root.enterFocus();
    //         }
    //
    //         Keys.onPressed: (event) => {
    //             if (event.key !== Qt.Key_Down || !libraryContentStack.currentItem || !libraryContentStack.currentItem.focusFirstItem) {
    //                 return;
    //             }
    //
    //             libraryContentStack.currentItem.focusFirstItem();
    //             event.accepted = true;
    //         }
    //
    //         onTabSelected: index => root.showTab(index)
    //     }
    // }

    FLTwoColumnPage {
        id: content
        anchors.fill: parent

        model: [
            {
                "type": "action",
                "label": "Resume game"
            },
            {
                "type": "action",
                "label": "Reset game"
            },
            {
                "type": "divider"
            },
            {
                "type": "action",
                "key": "suspend-points",
                "label": "Suspend points"
            },
            {
                "type": "action",
                "key": "rewind",
                "label": "Rewind"
            },
            {
                "type": "divider"
            },
            {
                "type": "action",
                "label": "Close game"
            }
            // {
            //     "type": "page",
            //     "key": "controllers",
            //     "label": "Controllers",
            //     "iconName": "controller",
            //     "page": controllerSettings
            // },
            // {
            //     "type": "divider"
            // },
            // {
            //     "type": "page",
            //     "key": "video",
            //     "label": "Video",
            //     "iconName": "display",
            //     "page": systemVideoSettings
            // },
            // {
            //     "type": "page",
            //     "key": "audio",
            //     "label": "Audio",
            //     "iconName": "display",
            //     "page": systemAudioSettings
            // },
            // {
            //     "type": "divider"
            // },
            // {
            //     "type": "header",
            //     "label": "Gameplay"
            // },
            // {
            //     "type": "page",
            //     "key": "emulation-general",
            //     "label": "General",
            //     "iconName": "display",
            //     "page": placeholderSettings
            // },
            // {
            //     "type": "page",
            //     "key": "emulation-picture",
            //     "label": "Picture",
            //     "iconName": "display",
            //     "page": emulationPictureSettings
            // },
            // {
            //     "type": "page",
            //     "key": "emulation-sound",
            //     "label": "Sound",
            //     "iconName": "display",
            //     "page": emulationSoundSettings
            // },
            // {
            //     "type": "page",
            //     "key": "emulation-input",
            //     "label": "Input",
            //     "iconName": "controller",
            //     "page": emulationInputSettings
            // },
            // {
            //     "type": "divider"
            // },
            // {
            //     "type": "header",
            //     "label": "Main stuff"
            // },
            // {
            //     "type": "page",
            //     "key": "retroachievements",
            //     "label": "Achievements",
            //     "iconName": "trophy",
            //     "page": retroAchievementSettings
            // },
            // {
            //     "type": "page",
            //     "key": "about",
            //     "label": "About",
            //     "iconName": "info",
            //     "page": about
            // }
        ]
    }
}