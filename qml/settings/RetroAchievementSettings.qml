import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    Flickable {
        anchors.fill: parent
        contentHeight: theColumn.height
        contentY: theColumn.activeFocusItem ? theColumn.activeFocusItem.y : 0
        ColumnLayout {
            id: theColumn
            width: parent.width
            spacing: 0
            // anchors.fill: parent

            RetroAchievementsAccountPane {
                id: raAccountPage
                Layout.alignment: Qt.AlignHCenter
                KeyNavigation.down: hardcoreModeToggle
                focus: true
            }

            ColumnLayout {
                Layout.fillWidth: true
                // visible: achievement_manager.loggedIn
                spacing: 0

                ToggleOption {
                    id: hardcoreModeToggle
                    Layout.fillWidth: true
                    KeyNavigation.down: learnMoreButton
                    label: "Enable hardcore mode"
                    description: "Playing in hardcore mode disables Suspend Points, Rewind, and other similar features.\n\nLater you'll be able to change this on a per-game basis."

                    checked: achievement_manager.defaultToHardcore

                    onCheckedChanged: {
                        achievement_manager.defaultToHardcore = checked
                    }
                }

                FirelightButton {
                    id: learnMoreButton
                    Layout.topMargin: 8
                    label: "Learn more"
                    KeyNavigation.down: achievementUnlockToggle
                    Layout.alignment: Qt.AlignHCenter
                    onClicked: {
                        Qt.openUrlExternally("https://docs.retroachievements.org/general/faq.html#what-is-hardcore-mode")
                    }
                }

                Text {
                    Layout.topMargin: 30
                    Layout.fillWidth: true
                    text: qsTr("Notifications")
                    font.pointSize: 11
                    font.family: Constants.regularFontFamily
                    font.weight: Font.DemiBold
                    Layout.bottomMargin: 8
                    color: "#a6a6a6"
                }

                ToggleOption {
                    id: achievementUnlockToggle
                    Layout.fillWidth: true
                    KeyNavigation.down: progressNotificationsToggle
                    label: "Achievement unlock notifications"
                    description: "Show a notification when you earn an achievement."

                    checked: achievement_manager.unlockNotificationsEnabled

                    onCheckedChanged: {
                        achievement_manager.unlockNotificationsEnabled = checked
                    }
                }

                ToggleOption {
                    id: progressNotificationsToggle
                    Layout.fillWidth: true
                    KeyNavigation.down: challengeIndicatorsToggle
                    Layout.minimumHeight: 42
                    label: "Progress notifications"
                    description: "Show a notification when you make progress on an achievement that has progress tracking."

                    checked: achievement_manager.progressNotificationsEnabled

                    onCheckedChanged: {
                        achievement_manager.progressNotificationsEnabled = checked
                    }
                }

                ToggleOption {
                    id: challengeIndicatorsToggle
                    Layout.fillWidth: true
                    Layout.minimumHeight: 42
                    label: "Challenge indicators"
                    description: "Show a small indicator on the screen when a challenge achievement is active."

                    checked: achievement_manager.challengeIndicatorsEnabled

                    onCheckedChanged: {
                        achievement_manager.challengeIndicatorsEnabled = checked
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

