import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root

    Button {
        text: "achieve progress indicator"

        property int number: 0

        onClicked: function () {
            achievementProgressIndicator.openWith("", "test", "test", number, 80)
            number += 1
        }

        AchievementProgressIndicator {
            id: achievementProgressIndicator

            enabled: achievement_manager.progressNotificationsEnabled
        }
    }

    Button {
        text: "toggle achievement"

        onClicked: {
            achievementUnlockIndicator.openWith("", "Mighty Morphin' Shy Guys", "Defeat the groups of two Red, Pink, Yellow, Green, and Blue Shy Guys IN ORDER in the Blue Zone of Shy Guys Toybox")
        }

        AchievementUnlockIndicator {
            id: achievementUnlockIndicator
        }
    }

    Button {
        text: "show game launch popup"

        onClicked: {
            gameLaunchPopup.open()
        }

        GameLaunchPopup {
            id: gameLaunchPopup
        }
    }

    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true

    }
}