import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    // boundsBehavior: Flickable.StopAtBounds
    // contentHeight: theColumn.height
    //
    // ScrollBar.vertical: ScrollBar {
    // }

    ColumnLayout {
        id: theColumn
        spacing: 0
        anchors.fill: parent
        // anchors.fill: parent

        RetroAchievementsAccountPane {
            id: raAccountPage
            Layout.alignment: Qt.AlignHCenter
            focus: true
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

        ColumnLayout {
            Layout.fillWidth: true
            // visible: achievement_manager.loggedIn
            spacing: 0

            Text {
                Layout.topMargin: 30
                text: "Default mode"
                color: "white"
                font.pointSize: 12
                Layout.alignment: Qt.AlignLeft
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
            }

            Text {
                Layout.fillHeight: true
                Layout.bottomMargin: 12
                text: "Only Hardcore mode is available at the moment."
                font.pixelSize: 14
                Layout.alignment: Qt.AlignLeft
                font.family: Constants.regularFontFamily
                // font.weight: Font.
                wrapMode: Text.WordWrap
                color: ColorPalette.neutral300
            }

            // RadioButton {
            //     Layout.fillWidth: true
            //     Layout.preferredHeight: 140
            //     checked: achievement_manager.defaultToHardcore
            //     horizontalPadding: 12
            //     verticalPadding: 6
            //
            //     background: Rectangle {
            //         radius: 8
            //         color: "#333333"
            //         border.color: parent.checked ? "#f16205" : "transparent"
            //         border.width: 3
            //     }
            //
            //     indicator: Item {
            //     }
            //
            //     onCheckedChanged: {
            //         if (checked) {
            //             achievement_manager.defaultToHardcore = true
            //         }
            //     }
            //
            //     HoverHandler {
            //         id: hardcoreHover
            //         cursorShape: Qt.PointingHandCursor
            //     }
            //
            //     contentItem: ColumnLayout {
            //         Text {
            //             Layout.fillWidth: true
            //             text: "Hardcore mode"
            //             color: "white"
            //             font.pointSize: 11
            //             Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            //             font.family: Constants.regularFontFamily
            //             font.weight: Font.DemiBold
            //         }
            //
            //         Text {
            //             text: "Challenge yourself and earn Mastery points by disabling save states, cheats, and rewinding."
            //             Layout.fillWidth: true
            //             wrapMode: Text.WordWrap
            //             font.pointSize: 11
            //             font.family: Constants.regularFontFamily
            //             font.weight: Font.Normal
            //             color: "#c1c1c1"
            //         }
            //         Item {
            //             Layout.fillHeight: true
            //         }
            //     }
            // }
            //
            // RadioButton {
            //     Layout.fillWidth: true
            //     Layout.preferredHeight: 60
            //     horizontalPadding: 12
            //     checked: !achievement_manager.defaultToHardcore
            //     verticalPadding: 6
            //
            //     background: Rectangle {
            //         radius: 8
            //         color: "#333333"
            //         border.color: parent.checked ? "#f16205" : "transparent"
            //         border.width: 3
            //     }
            //
            //     indicator: Item {
            //     }
            //
            //     onCheckedChanged: {
            //         if (checked) {
            //             achievement_manager.defaultToHardcore = false
            //         }
            //     }
            //
            //     Connections {
            //         target: achievement_manager
            //
            //         function onDefaultToHardcoreChanged() {
            //             console.log(":)")
            //         }
            //     }
            //
            //     HoverHandler {
            //         id: casualHover
            //         cursorShape: Qt.PointingHandCursor
            //     }
            //
            //     contentItem: ColumnLayout {
            //         Text {
            //             Layout.fillWidth: true
            //             text: "Casual mode"
            //             color: "white"
            //             font.pointSize: 11
            //             Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            //             font.family: Constants.regularFontFamily
            //             font.weight: Font.DemiBold
            //         }
            //
            //         Text {
            //             text: "Play with save states, cheats, and rewinding enabled."
            //             Layout.fillWidth: true
            //             wrapMode: Text.WordWrap
            //             font.pointSize: 11
            //             font.family: Constants.regularFontFamily
            //             font.weight: Font.Normal
            //             color: "#c1c1c1"
            //         }
            //
            //         Item {
            //             Layout.fillHeight: true
            //         }
            //     }
            // }

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

            // ToggleOption {
            //     Layout.fillWidth: true
            //     Layout.minimumHeight: 42
            //     label: "Completion notifications"
            //     description: "Show a notification when you earn all of the achievements for a game."
            //
            //     checked: true
            // }
            //
            // Rectangle {
            //     Layout.fillWidth: true
            //     Layout.preferredHeight: 1
            //     color: "#333333"
            // }

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

            Button {
                id: logoutButton
                Layout.topMargin: 12
                Layout.fillWidth: true
                visible: achievement_manager.loggedIn
                // text: qsTr("Log out of RetroAchievements")
                onClicked: {
                    achievement_manager.logout()
                }

                implicitHeight: 48

                contentItem: Text {
                    text: qsTr("Log out of RetroAchievements")
                    color: "white"
                    font.pointSize: 11
                    font.family: Constants.regularFontFamily
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                }

                hoverEnabled: true

                background: Rectangle {
                    color: logoutButton.hovered ? ColorPalette.red700 : ColorPalette.red500
                    radius: 8
                }
            }
        }



        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

        }
    }

    Dialog {
        id: control
        property string text

        property string errorMessage: ""
        property bool showButtons: true
        modal: true
        parent: Overlay.overlay
        anchors.centerIn: parent
        padding: 12

        background: Rectangle {
            color: Constants.colorTestSurfaceContainerLowest
            radius: 6
            implicitWidth: 400
            implicitHeight: 200
        }

        header: Text {
            text: "Log in to RetroAchievements"
            color: "white"
            font.family: Constants.regularFontFamily
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pointSize: 11
            leftPadding: 10
            rightPadding: 10
            topPadding: 8
            bottomPadding: 8
            font.weight: Font.DemiBold
        }

        contentItem: ColumnLayout {
            spacing: 12

            TextField {
                id: user
                Layout.alignment: Qt.AlignHCenter
                placeholderText: qsTr("Username")

            }
            TextField {
                id: pass
                Layout.alignment: Qt.AlignHCenter
                placeholderText: qsTr("Password")
                echoMode: TextInput.Password
            }

            Rectangle {
                visible: control.errorMessage.length > 0
                Layout.fillWidth: true
                Layout.preferredHeight: 28
                color: "yellow"

                Text {
                    anchors.fill: parent
                    text: control.errorMessage
                    color: "orange"
                    font.family: Constants.regularFontFamily
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: 12

                }
            }
        }

        footer: DialogButtonBox {
            background: Rectangle {
                color: "transparent"
            }
            Button {
                background: Rectangle {
                    color: "transparent"
                }
                contentItem: Text {
                    text: qsTr("Cancel")
                    // anchors.centerIn: parent
                    color: Constants.colorTestTextActive
                    font.family: Constants.regularFontFamily
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: 12
                }

                onClicked: function () {
                    control.reject()
                }

                HoverHandler {
                    acceptedDevices: PointerDevice.Mouse
                    cursorShape: Qt.PointingHandCursor
                }
            }
            Button {
                anchors.verticalCenter: parent.verticalCenter
                background: Rectangle {
                    color: "white"
                    radius: 6
                }
                contentItem: Text {
                    text: qsTr("Log in")
                    anchors.centerIn: parent
                    color: Constants.colorTestBackground
                    font.family: Constants.regularFontFamily
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: 12
                }

                onClicked: function () {
                    achievement_manager.logInUserWithPassword(user.text, pass.text)
                }

                Connections {
                    target: achievement_manager

                    function onLoginSucceeded() {
                        control.accept()
                    }

                    function onLoginFailedWithInvalidCredentials() {
                        console.log("oh no")
                        control.errorMessage = "Invalid credentials"
                    }

                    function onLoginFailedWithExpiredToken() {
                        console.log("oh no")
                        control.errorMessage = "Expired token"
                    }

                    function onLoginFailedWithAccessDenied() {
                        console.log("oh no")
                        control.errorMessage = "Access denied"
                    }

                    function onLoginFailedWithInternalError() {
                        console.log("oh no")
                        control.errorMessage = "Internal error"
                    }
                }

                HoverHandler {
                    acceptedDevices: PointerDevice.Mouse
                    cursorShape: Qt.PointingHandCursor
                }
            }
        }

        Overlay.modal: Rectangle {
            color: "black"
            anchors.fill: parent
            opacity: 0.4

            Behavior on opacity {
                NumberAnimation {
                    duration: 200
                }
            }
        }

        enter: Transition {
            NumberAnimation {
                property: "opacity";
                from: 0.0;
                to: 1.0
                duration: 200
            }
            NumberAnimation {
                property: "scale";
                from: 0.9;
                to: 1.0
                duration: 200
            }
        }

        exit: Transition {
            NumberAnimation {
                property: "opacity";
                from: 1.0;
                to: 0.0;
                duration: 200
            }
            NumberAnimation {
                property: "scale";
                from: 1.0;
                to: 0.9
                duration: 200
            }
        }
    }
}
