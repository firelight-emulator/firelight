import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Flickable {
    boundsBehavior: Flickable.StopAtBounds
    contentHeight: theColumn.height

    ScrollBar.vertical: ScrollBar {
    }

    ColumnLayout {
        id: theColumn
        spacing: 0
        width: parent.width
        // anchors.fill: parent

        ColumnLayout {
            Layout.fillWidth: true

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 160
                color: "#333333"
                radius: 8

                Pane {
                    visible: achievement_manager.loggedIn
                    anchors.fill: parent
                    background: Item {
                    }

                    contentItem: RowLayout {
                        Image {
                            Layout.preferredHeight: 120
                            Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                            source: achievement_manager.avatarUrl
                            fillMode: Image.PreserveAspectFit
                        }
                        ColumnLayout {
                            Text {
                                text: achievement_manager.displayName
                                font.pointSize: 12
                                font.family: Constants.regularFontFamily
                                font.weight: Font.DemiBold
                                color: "white"
                            }
                            Text {
                                text: "Points: " + achievement_manager.points
                                font.pointSize: 11
                                font.family: Constants.regularFontFamily
                                color: "white"
                            }
                            Item {
                                Layout.fillHeight: true
                            }
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        Button {
                            Layout.alignment: Qt.AlignRight | Qt.AlignBottom
                            text: "Log out"
                            onClicked: {
                                achievement_manager.logout()
                            }
                        }
                    }

                    // Text {
                    //     id: loggedInAs
                    //     anchors.top: parent.top
                    //     anchors.left: parent.left
                    //     text: qsTr("Logged in as %1").arg(achievement_manager.displayName)
                    //     font.pointSize: 12
                    //     font.family: Constants.regularFontFamily
                    //     color: "white"
                    // }
                    //
                    // Rectangle {
                    //     anchors.left: parent.left
                    // }
                }

                ColumnLayout {
                    visible: !achievement_manager.loggedIn
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
                        // background: Rectangle {
                        //     color: "#f16205"
                        //     width: 130
                        //     height: 50
                        //     radius: 8
                        // }
                        // onClicked: {
                        //     achievement_manager.showLoginDialog()
                        // }
                    }
                    Text {
                        // text: qsTr("<a href='https://retroachievements.org/createaccount.php'>Don't have an account?</a>")
                        text: "Don't have an account?"
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

            Text {
                Layout.topMargin: 30
                Layout.fillWidth: true
                text: "Default mode"
                color: "white"
                font.pointSize: 12
                Layout.alignment: Qt.AlignLeft
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
            }

            Text {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.bottomMargin: 12
                text: "You can change this setting for individual games."
                font.pointSize: 10
                Layout.alignment: Qt.AlignLeft
                font.family: Constants.regularFontFamily
                wrapMode: Text.WordWrap
                color: "#c1c1c1"
            }

            Item {
                Layout.fillWidth: true
                Layout.minimumHeight: 140
                RadioButton {
                    anchors.left: parent.left
                    height: parent.height
                    width: parent.width / 2 - 8
                    checked: true
                    horizontalPadding: 12
                    verticalPadding: 6

                    background: Rectangle {
                        radius: 8
                        color: "#333333"
                        border.color: parent.checked ? "#f16205" : "transparent"
                        border.width: 3
                    }

                    indicator: Item {
                    }

                    HoverHandler {
                        id: hardcoreHover
                        cursorShape: Qt.PointingHandCursor
                    }

                    contentItem: ColumnLayout {
                        Text {
                            Layout.fillWidth: true
                            text: "Hardcore mode"
                            color: "white"
                            font.pointSize: 11
                            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                            font.family: Constants.regularFontFamily
                            font.weight: Font.DemiBold
                        }

                        Text {
                            text: "Challenge yourself and earn Mastery points by disabling save states, cheats, and rewinding."
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            font.pointSize: 11
                            font.family: Constants.regularFontFamily
                            font.weight: Font.Normal
                            color: "#c1c1c1"
                        }
                        Item {
                            Layout.fillHeight: true
                        }
                    }
                }
                RadioButton {
                    anchors.right: parent.right
                    height: parent.height
                    width: parent.width / 2 - 8
                    horizontalPadding: 12
                    verticalPadding: 6

                    background: Rectangle {
                        radius: 8
                        color: "#333333"
                        border.color: parent.checked ? "#f16205" : "transparent"
                        border.width: 3
                    }

                    indicator: Item {
                    }

                    HoverHandler {
                        id: casualHover
                        cursorShape: Qt.PointingHandCursor
                    }

                    contentItem: ColumnLayout {
                        Text {
                            Layout.fillWidth: true
                            text: "Casual mode"
                            color: "white"
                            font.pointSize: 11
                            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                            font.family: Constants.regularFontFamily
                            font.weight: Font.DemiBold
                        }

                        Text {
                            text: "Play with save states, cheats, and rewinding enabled."
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            font.pointSize: 11
                            font.family: Constants.regularFontFamily
                            font.weight: Font.Normal
                            color: "#c1c1c1"
                        }

                        Item {
                            Layout.fillHeight: true
                        }
                    }
                }
            }

            // ToggleOption {
            //     Layout.fillWidth: true
            //     Layout.minimumHeight: 42
            //     label: "Hardcore mode"
            //     description: "Disables save states, cheats, and rewinding, but allows you to earn Mastery on achievements"
            // }

            //            Rectangle {
            //                Layout.fillWidth: true
            //                Layout.preferredHeight: 1
            //                color: "#333333"
            //            }
            //
            //            ToggleOption {
            //                Layout.fillWidth: true
            //                label: "Suspend game while disconnected"
            //                description: "Your game will be suspended when you lose connection to RetroAchievements and will resume
            // when you reconnect. This will prevent you from missing achievements due to disconnections."
            //            }

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

                checked: achievement_manager.unlockNotificationsEnabled

                onCheckedChanged: {
                    achievement_manager.unlockNotificationsEnabled = checked
                }
            }

            // ToggleOption {
            //     Layout.fillWidth: true
            //     label: "Play sound"
            //     Layout.leftMargin: 24
            // }

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

                checked: achievement_manager.progressNotificationsEnabled

                onCheckedChanged: {
                    achievement_manager.progressNotificationsEnabled = checked
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#333333"
            }

            ToggleOption {
                Layout.fillWidth: true
                Layout.minimumHeight: 42
                label: "Completion notifications"
                description: "Show a notification when you earn all of the achievements for a game."

                checked: true
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#333333"
            }

            ToggleOption {
                Layout.fillWidth: true
                Layout.minimumHeight: 42
                label: "Challenge indicators"
                description: "Show a small indicator on the screen when a challenge achievement is active."

                checked: achievement_manager.challengeIndicatorsEnabled

                onCheckedChanged: {
                    achievement_manager.challengeIndicatorsEnabled = checked
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#333333"
            }

            Item {
                Layout.fillWidth: true
                Layout.minimumHeight: 42
                Text {
                    anchors.left: parent.left
                    visible: !achievement_manager.loggedIn
                    text: qsTr("Not logged in to RetroAchievements")
                    font.pointSize: 12
                    font.family: Constants.regularFontFamily
                    color: "white"
                }

                Text {
                    anchors.left: parent.left
                    visible: achievement_manager.loggedIn
                    text: qsTr("Logged in as %1").arg(achievement_manager.displayName)
                    font.pointSize: 12
                    font.family: Constants.regularFontFamily
                    color: "white"
                }
            }


            TextField {
                id: user
                visible: !achievement_manager.loggedIn
                Layout.fillWidth: true
                placeholderText: qsTr("Username")
            }
            TextField {
                id: pass
                visible: !achievement_manager.loggedIn
                Layout.fillWidth: true
                placeholderText: qsTr("Password")
                echoMode: TextInput.Password
            }

            Button {
                Layout.alignment: Qt.AlignRight
                visible: !achievement_manager.loggedIn
                text: qsTr("Login")
                onClicked: {
                    achievement_manager.logInUserWithPassword(user.text, pass.text)
                }
            }

            Button {
                Layout.alignment: Qt.AlignRight
                visible: achievement_manager.loggedIn
                text: qsTr("Log out")
                onClicked: {
                    achievement_manager.logout()
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

        }
    }
}
