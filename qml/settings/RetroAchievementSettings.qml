import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FirelightStyle 1.0

ColumnLayout {
    id: root
    spacing: 0

    Image {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
        source: "file:system/_img/RA_Logo10.png"
        fillMode: Image.PreserveAspectFit
    }

    ColumnLayout {
        Layout.fillWidth: true

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 160
            color: "#333333"
            radius: 8

            ColumnLayout {
                anchors.fill: parent
                Text {
                    Layout.preferredWidth: parent.width * 0.75
                    text: qsTr("You're not logged in to RetroAchievements.\nLog in to start earning achievements!")
                    font.pointSize: 11
                    font.family: Constants.regularFontFamily
                    font.weight: Font.Medium
                    wrapMode: Text.WordWrap
                    color: "#bebfbe"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    Layout.topMargin: 12
                }
                Item {
                    Layout.fillHeight: true
                }
                Button {
                    text: qsTr("Log in")
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    // onClicked: {
                    //     achievement_manager.showLoginDialog()
                    // }
                }
                Text {
                    text: qsTr("<a href='https://retroachievements.org/createaccount.php'>Don't have an account?</a>")
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    font.pointSize: 10
                    // textFormat: Text.StyledText
                    font.family: Constants.regularFontFamily
                    font.weight: Font.Medium
                    wrapMode: Text.WordWrap
                    color: "#4673e4"
                    Layout.topMargin: 2
                    Layout.bottomMargin: 12
                }
            }


        }

        // Text {
        //     Layout.fillWidth: true
        //     Layout.minimumHeight: 42
        //     Layout.bottomMargin: 12
        //     text: qsTr("Customize the notifications you receive when unlocking or making progress on achievements")
        //     font.pointSize: 11
        //     wrapMode: Text.WordWrap
        //     font.family: Constants.regularFontFamily
        //     color: "#c1c1c1"
        // }

        ToggleOption {
            Layout.fillWidth: true
            Layout.minimumHeight: 42
            label: "Hardcore mode"
            description: "Disables save states, cheats, and rewinding, but allows you to earn Mastery on achievements"
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#333333"
        }

        ToggleOption {
            Layout.fillWidth: true
            label: "Suspend game while disconnected"
            description: "Your game will be suspended when you lose connection to RetroAchievements and will resume
 when you reconnect. This will prevent you from missing achievements due to disconnections."
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
            Layout.fillWidth: true
            label: "Achievement unlock notifications"
            description: "Show a notification when you earn an achievement."
        }

        ToggleOption {
            Layout.fillWidth: true
            label: "Play sound"
            Layout.leftMargin: 24
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#333333"
        }

        ToggleOption {
            Layout.fillWidth: true
            Layout.minimumHeight: 42
            label: "Progress notifications"
            description: "Show a notification when you make progress on an achievement that has progress tracking."
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#333333"
        }

        // Item {
        //     Layout.fillWidth: true
        //     Layout.minimumHeight: 42
        //     Text {
        //         anchors.left: parent.left
        //         visible: !achievement_manager.loggedIn
        //         text: qsTr("Not logged in to RetroAchievements")
        //         font.pointSize: 12
        //         font.family: Constants.regularFontFamily
        //         color: "white"
        //     }
        //
        //     Text {
        //         anchors.left: parent.left
        //         visible: achievement_manager.loggedIn
        //         text: qsTr("Logged in as %1").arg(achievement_manager.displayName)
        //         font.pointSize: 12
        //         font.family: Constants.regularFontFamily
        //         color: "white"
        //     }
        // }
        //
        //
        // TextField {
        //     id: user
        //     visible: !achievement_manager.loggedIn
        //     Layout.fillWidth: true
        //     placeholderText: qsTr("Username")
        // }
        // TextField {
        //     id: pass
        //     visible: !achievement_manager.loggedIn
        //     Layout.fillWidth: true
        //     placeholderText: qsTr("Password")
        //     echoMode: TextInput.Password
        // }
        //
        // Button {
        //     Layout.alignment: Qt.AlignRight
        //     visible: !achievement_manager.loggedIn
        //     text: qsTr("Login")
        //     onClicked: {
        //         achievement_manager.logInUserWithPassword(user.text, pass.text)
        //     }
        // }
        //
        // Button {
        //     Layout.alignment: Qt.AlignRight
        //     visible: achievement_manager.loggedIn
        //     text: qsTr("Log out")
        //     onClicked: {
        //         achievement_manager.logout()
        //     }
        // }
    }

    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true

    }
}