import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0
import Firelight 1.0

FocusScope {
    id: root

    signal nextClicked()

    implicitHeight: achievement_manager.loggedIn ? loggedInPane.height : contentPane.height
    implicitWidth: achievement_manager.loggedIn ? loggedInPane.width : contentPane.width

    Pane {
        id: loggedInPane
        visible: achievement_manager.loggedIn
        anchors.fill: parent
        focus: achievement_manager.loggedIn
        background: Item {}
        contentItem:  ColumnLayout {
            spacing: 8
            Image {
                Layout.maximumHeight: 60
                Layout.minimumHeight: 60
                sourceSize.height: 60
                Layout.alignment: Qt.AlignHCenter
                source: "qrc:images/retroachievements-banner"
                fillMode: Image.PreserveAspectFit
            }

            Pane {
                id: raAccountPane
                Layout.topMargin: 16
                // Layout.maximumWidth: 400
                // Layout.preferredWidth: 600
                // Layout.minimumWidth: 200
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                focus: true
                background: Rectangle {
                    radius: 4
                    color: ColorPalette.neutral900
                }

                contentItem: RowLayout {
                    spacing: 16
                    Image {
                        source: achievement_manager.avatarUrl
                        fillMode: Image.PreserveAspectFit
                        Layout.preferredWidth: 140
                        Layout.preferredHeight: 140
                    }
                    ColumnLayout {
                        spacing: 8
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignTop
                        Text {
                            text: achievement_manager.displayName
                            color: ColorPalette.neutral200
                            font.pixelSize: 22
                            font.weight: Font.DemiBold

                            font.family: Constants.regularFontFamily
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }
                        Text {
                            text: achievement_manager.points
                            color: ColorPalette.neutral300
                            font.pixelSize: 16
                            font.weight: Font.Normal

                            font.family: Constants.regularFontFamily
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }


                        Item {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }

                    FirelightButton {
                        Layout.alignment: Qt.AlignRight | Qt.AlignBottom
                        focus: true
                        circle: true
                        tooltipLabel: "Log out"
                        tooltipOnTop: true
                        iconCode: "\ue9ba"
                        onClicked: {
                            logoutDialog.open()
                        }
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }

    FirelightDialog {
        id: logoutDialog
        text: "Are you sure you want to log out of RetroAchievements?\n\nYou'll need to log in again to earn achievements."

        onAccepted: {
            achievement_manager.logout()
        }
    }

    Pane {
        id: contentPane
        visible: !achievement_manager.loggedIn
        focus: !achievement_manager.loggedIn
        anchors.fill: parent
        background: Item {
        }

        contentItem: ColumnLayout {
                id: meColumn
                spacing: 8
                Image {
                    Layout.maximumHeight: 60
                    Layout.minimumHeight: 60
                    sourceSize.height: 60
                    Layout.alignment: Qt.AlignHCenter
                    source: "qrc:images/retroachievements-banner"
                    fillMode: Image.PreserveAspectFit
                }
                Text {
                        text: "Login"
                        Layout.topMargin: 24
                        Layout.alignment: Qt.AlignHCenter
                        color: ColorPalette.neutral100
                        font.pixelSize: 18
                        font.weight: Font.Normal
                        font.family: Constants.regularFontFamily
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    Pane {
                        id: thePane
                        Layout.topMargin: 8
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: 280
                        Layout.preferredHeight: 48
                        background: Rectangle {
                            color: ColorPalette.neutral800
                            radius: 4
                        }

                        focus: true

                        HoverHandler {
                            acceptedDevices: PointerDevice.Mouse
                            cursorShape: Qt.IBeamCursor
                        }

                        contentItem: Item {
                            Text {
                                anchors.fill: parent
                                font.pointSize: 12
                                font.family: Constants.regularFontFamily
                                color: ColorPalette.neutral500
                                text: "Username"
                                verticalAlignment: Text.AlignVCenter
                                visible: usernameTextInput.length === 0
                            }
                            TextInput {
                                id: usernameTextInput
                                anchors.fill: parent
                                activeFocusOnTab: true
                                KeyNavigation.down: passwordTextInput
                                property bool showGlobalCursor: true
                                property var globalCursorProxy: thePane
                                font.family: Constants.regularFontFamily
                                focus: true
                                font.pointSize: 12
                                color: "white"
                                verticalAlignment: Text.AlignVCenter

                                onAccepted: {
                                    if (submitButton.enabled) {
                                        submitButton.clicked()
                                    }
                                }
                            }
                        }
                    }

                    Pane {
                        id: theOtherPane
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: 280
                        Layout.preferredHeight: 48
                        background: Rectangle {
                            color: ColorPalette.neutral800
                            radius: 4
                        }

                        HoverHandler {
                            acceptedDevices: PointerDevice.Mouse
                            cursorShape: Qt.IBeamCursor
                        }

                        contentItem: FocusScope {
                            Text {
                                anchors.fill: parent
                                font.pointSize: 12
                                font.family: Constants.regularFontFamily
                                color: ColorPalette.neutral500
                                text: "Password"
                                verticalAlignment: Text.AlignVCenter
                                visible: passwordTextInput.length === 0
                            }
                            TextInput {
                                id: passwordTextInput
                                activeFocusOnTab: true
                                anchors.fill: parent
                                echoMode: TextInput.Password
                                KeyNavigation.down: submitButton
                                property bool showGlobalCursor: true
                                property var globalCursorProxy: theOtherPane
                                font.family: Constants.regularFontFamily
                                font.pointSize: 12
                                color: "white"
                                verticalAlignment: Text.AlignVCenter

                                onAccepted: {
                                    if (submitButton.enabled) {
                                        submitButton.clicked()
                                    }
                                }
                            }
                        }
                    }

                    FirelightButton {
                        id: submitButton
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: 12
                        KeyNavigation.down: goToWebsiteButton
                        label: "Submit"
                        enabled: usernameTextInput.text !== "" && passwordTextInput.text !== ""

                        onClicked: function () {
                            achievement_manager.logInUserWithPassword(usernameTextInput.text, passwordTextInput.text)
                            submitButton.focus = false
                            root.focus = false
                        }
                    }

                    FirelightDialog {
                        id: loginErrorDialog
                        text: "Hey there buddy"
                        showCancel: false

                        Connections {
                            target: achievement_manager

                            function onLoginFailedWithInvalidCredentials() {
                                loginErrorDialog.text = "Invalid credentials"
                                loginErrorDialog.open()
                            }

                            function onLoginFailedWithAccessDenied() {
                                loginErrorDialog.text = "There's an issue with your RetroAchievements account"
                                loginErrorDialog.open()
                            }

                            function onLoginFailedWithInternalError() {
                                loginErrorDialog.text = "There was an issue connecting to RetroAchievements; please try again in a few minutes"
                                loginErrorDialog.open()
                            }
                        }
                    }

                    Text {
                        text: "Don't have an account?"
                        Layout.topMargin: 48
                        Layout.alignment: Qt.AlignHCenter
                        color: ColorPalette.neutral100
                        font.pixelSize: 18
                        font.weight: Font.Normal
                        font.family: Constants.regularFontFamily
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    Text {
                        text: "This button will take you to the RetroAchievements website to create one"
                        Layout.alignment: Qt.AlignHCenter
                        color: ColorPalette.neutral300
                        font.pixelSize: 16
                        font.weight: Font.Normal
                        font.family: Constants.regularFontFamily
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    FirelightButton {
                        id: goToWebsiteButton
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: 12
                        label: "Go to website"
                        onClicked: {
                            Qt.openUrlExternally("https://retroachievements.org/createaccount.php")
                        }
                    }

                    Connections {
                        target: achievement_manager

                        function onLoginSucceeded() {
                            // control.accept()
                        }

                        function onLoginFailedWithInvalidCredentials() {
                            console.log("Invalid credentials")
                        }

                        function onLoginFailedWithExpiredToken() {
                            console.log("Expired token")
                        }

                        function onLoginFailedWithAccessDenied() {
                            console.log("Access denied")
                        }

                        function onLoginFailedWithInternalError() {
                            console.log("Internal error")
                        }
                    }



                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                // RowLayout {
                //     spacing: 12
                //     Layout.fillHeight: false
                //     Layout.fillWidth: true
                //     Item {
                //         Layout.fillWidth: true
                //         Layout.fillHeight: true
                //     }
                //     FirelightButton {
                //         label: "Next"
                //         onClicked: function () {
                //             nextClicked()
                //         }
                //     }
                // }
            }
    }
}