import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts 1.0
import Firelight 1.0

FocusScope {
    id: root

    property bool movingRight: true
    property int lastIndex: 0
    property int currentIndex: 0

    signal doneButtonPressed()

    onCurrentIndexChanged: function () {
        movingRight = currentIndex > lastIndex
        lastIndex = currentIndex
        if (currentIndex === 0) {
            contentStack.replaceCurrentItem(welcomePage, {}, StackView.ReplaceTransition)
        } else if (currentIndex === 1) {
            contentStack.replaceCurrentItem(directoriesPage, {}, StackView.ReplaceTransition)
        } else if (currentIndex === 2) {
            contentStack.replaceCurrentItem(achievementsPage, {}, StackView.ReplaceTransition)
        } else if (currentIndex === 3) {
            contentStack.replaceCurrentItem(readyPage, {}, StackView.ReplaceTransition)
        }
    }

    Pane {
        id: headerBar
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.left: parent.left
        padding: 24
        horizontalPadding: 48

        KeyNavigation.down: contentStack

        background: Item {
        }

        contentItem: RowLayout {
            spacing: 24

            Button {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.horizontalStretchFactor: 1
                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }
                onClicked: function () {
                    root.currentIndex = 0
                }
                background: Item {
                }
                contentItem:
                    Text {
                        text: "Welcome"
                        color: ColorPalette.neutral100
                        font.pixelSize: 16
                        font.weight: root.currentIndex === 0 ? Font.DemiBold : Font.Medium
                        opacity: root.currentIndex === 0 ? 1.0 : 0.5
                        font.family: Constants.regularFontFamily
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
            }

            Button {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.horizontalStretchFactor: 1
                background: Item {
                }

                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }
                onClicked: function () {
                    root.currentIndex = 1
                }
                contentItem:
                    Text {
                        text: "Directories"
                        color: ColorPalette.neutral100
                        font.weight: root.currentIndex === 1 ? Font.DemiBold : Font.Medium
                        opacity: root.currentIndex === 1 ? 1.0 : 0.5
                        font.pixelSize: 16
                        font.family: Constants.regularFontFamily
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
            }

            Button {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.horizontalStretchFactor: 1
                background: Item {
                }
                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }
                onClicked: function () {
                    root.currentIndex = 2
                }
                contentItem:
                    Text {
                        text: "Achievements"
                        color: ColorPalette.neutral100
                        font.weight: root.currentIndex === 2 ? Font.DemiBold : Font.Medium
                        opacity: root.currentIndex === 2 ? 1.0 : 0.5
                        font.pixelSize: 16
                        font.family: Constants.regularFontFamily
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
            }

            Button {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.horizontalStretchFactor: 1
                background: Item {
                }
                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }
                onClicked: function () {
                    root.currentIndex = 3
                }
                contentItem:
                    Text {
                        text: "Ready to play!"
                        color: ColorPalette.neutral100
                        font.weight: root.currentIndex === 3 ? Font.DemiBold : Font.Medium
                        opacity: root.currentIndex === 3 ? 1.0 : 0.5
                        font.pixelSize: 16
                        font.family: Constants.regularFontFamily
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
            }
        }
    }

    Component {
        id: welcomePage
        Pane {
            background: Item {
            }
            contentItem: ColumnLayout {
                width: Math.max(parent.width / 3, 700)
                anchors.horizontalCenter: parent.horizontalCenter
                Image {
                    Layout.topMargin: 24
                    source: "file:system/_img/logo.png"
                    Layout.preferredWidth: 300
                    Layout.preferredHeight: 300
                    sourceSize.width: 512
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                    fillMode: Image.PreserveAspectFit
                }
                Text {
                    Layout.topMargin: 24
                    text: "Welcome to"
                    color: ColorPalette.neutral300
                    font.pixelSize: 18
                    font.weight: Font.Normal

                    font.family: Constants.regularFontFamily
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                Text {
                    text: "Firelight"
                    color: ColorPalette.neutral100
                    Layout.topMargin: 4
                    font.pixelSize: 48
                    font.weight: Font.Bold
                    font.family: Constants.regularFontFamily
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                FirelightButton {
                    Layout.topMargin: 48
                    focus: true
                    label: "Let's go!"
                    Layout.alignment: Qt.AlignCenter

                    onClicked: function () {
                        root.currentIndex = 1
                    }
                }
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
        }
    }

    Component {
        id: directoriesPage
        Pane {
            background: Item {
            }
            contentItem: ColumnLayout {
                DirectoryOption {
                    id: gameDirectoryOption
                    Layout.fillWidth: true
                    label: "Game directory"
                    description: "This is where you’ll put your game files. Firelight will automatically detect files in this directory and add them to your library."
                    value: UserLibrary.mainGameDirectory

                    onValueChanged: function () {
                        UserLibrary.mainGameDirectory = value
                    }

                    // Component.onCompleted: {
                    //     // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
                    //     emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed")
                    // }
                    //
                    // onCheckedChanged: function () {
                    //     if (checked) {
                    //         emulator_config_manager.setOptionValueForPlatform(1, "snes9x_up_down_allowed", "enabled")
                    //     } else {
                    //         emulator_config_manager.setOptionValueForPlatform(1, "snes9x_up_down_allowed", "disabled")
                    //     }
                    // }
                }
                DirectoryOption {
                    id: saveDirectoryOption
                    KeyNavigation.up: gameDirectoryOption
                    KeyNavigation.down: dirNextButton
                    Layout.fillWidth: true
                    label: "Saves directory"
                    description: "This is where Firelight will save your save files and Suspend Point data."
                    value: SaveManager.saveDirectory

                    onValueChanged: function () {
                        SaveManager.saveDirectory = value
                    }

                    // Component.onCompleted: {
                    //     // checked = emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed") === "enabled"
                    //     emulator_config_manager.getOptionValueForPlatform(1, "snes9x_up_down_allowed")
                    // }
                    //
                    // onCheckedChanged: function () {
                    //     if (checked) {
                    //         emulator_config_manager.setOptionValueForPlatform(1, "snes9x_up_down_allowed", "enabled")
                    //     } else {
                    //         emulator_config_manager.setOptionValueForPlatform(1, "snes9x_up_down_allowed", "disabled")
                    //     }
                    // }
                }
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
                RowLayout {
                    spacing: 12
                    Layout.fillHeight: false
                    Layout.fillWidth: true
                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                    FirelightButton {
                        id: dirNextButton
                        label: "Next"
                        onClicked: function () {
                            root.currentIndex = 2
                        }
                    }
                }
            }
        }
    }

    Component {
        id: achievementsPage
        Pane {
            background: Item {
            }
            contentItem: ColumnLayout {
                spacing: 8
                Image {
                    Layout.topMargin: 36
                    Layout.maximumHeight: 60
                    Layout.minimumHeight: 60
                    Layout.alignment: Qt.AlignHCenter
                    source: "file:system/_img/raFullLogo.png"
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    text: "You're logged in"
                    visible: achievement_manager.loggedIn
                    Layout.topMargin: 80
                    Layout.alignment: Qt.AlignHCenter
                    color: ColorPalette.neutral200
                    font.pixelSize: 16
                    font.weight: Font.Normal
                    font.family: Constants.regularFontFamily
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                Pane {
                    id: raAccountPane
                    visible: achievement_manager.loggedIn
                    Layout.topMargin: 16
                    Layout.maximumWidth: 400
                    Layout.preferredWidth: 400
                    Layout.minimumWidth: 200
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    background: Rectangle {
                        radius: 4
                        color: ColorPalette.neutral900
                    }

                    contentItem: RowLayout {
                        spacing: 16
                        Image {
                            source: achievement_manager.avatarUrl
                            fillMode: Image.PreserveAspectFit
                            Layout.preferredWidth: 100
                            Layout.preferredHeight: 100
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
                            Text {
                                text: "Member since January 1, 2025"
                                color: ColorPalette.neutral300
                                font.pixelSize: 16
                                font.weight: Font.Normal

                                font.family: Constants.regularFontFamily
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignVCenter
                            }
                        }

                        Item {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter
                    visible: !achievement_manager.loggedIn

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
                        Layout.topMargin: 8
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
                                font.family: Constants.regularFontFamily
                                font.pointSize: 12
                                color: "white"
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                    }

                    Pane {
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

                        contentItem: Item {
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
                                anchors.fill: parent
                                echoMode: TextInput.Password
                                font.family: Constants.regularFontFamily
                                font.pointSize: 12
                                color: "white"
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                    }

                    FirelightButton {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: 12
                        label: "Submit"
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
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: 12
                        label: "Go to website"
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                RowLayout {
                    spacing: 12
                    Layout.fillHeight: false
                    Layout.fillWidth: true
                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                    FirelightButton {
                        label: "Next"
                        onClicked: function () {
                            root.currentIndex = 3
                        }
                    }
                }
            }
        }
    }

    Component {
        id: readyPage
        Pane {
            background: Item {
            }
            contentItem: ColumnLayout {

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
                Text {
                    text: "You're ready to go!"
                    Layout.topMargin: 36
                    Layout.alignment: Qt.AlignHCenter
                    color: ColorPalette.neutral200
                    font.pixelSize: 22
                    font.weight: Font.Normal
                    font.family: Constants.regularFontFamily
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                // Text {
                //     text: "Thanks for using Firelight. It means a lot to me! - BiscuitCakes"
                //     Layout.topMargin: 36
                //     Layout.alignment: Qt.AlignHCenter
                //     color: ColorPalette.neutral400
                //     font.pixelSize: 18
                //     font.weight: Font.Normal
                //     font.family: Constants.regularFontFamily
                //     horizontalAlignment: Text.AlignHCenter
                //     verticalAlignment: Text.AlignVCenter
                // }

                FirelightButton {
                    Layout.topMargin: 48
                    label: "Go to Home menu"
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    onClicked: function () {
                        root.doneButtonPressed()
                        // contentStack.pushItem(achievementsPage, {}, StackView.PushTransition)
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
        }
    }


    StackView {
        id: contentStack
        anchors.top: headerBar.bottom
        // anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        // width: Math.max(1200, width)
        // anchors.horizontalCenter: parent.horizontalCenter
        // width: 1200
        anchors.leftMargin: Math.max(Math.max(parent.width - 1200, 0) / 2, 60)
        anchors.rightMargin: Math.max(Math.max(parent.width - 1200, 0) / 2, 60)
        anchors.bottomMargin: 48

        background: Item {
        }

        focus: true

        //
        objectName: "Home Content Stack View"
        // anchors.fill: parent


        initialItem: welcomePage

        pushEnter: Transition {
            NumberAnimation {
                property: "opacity"
                from: 0.0
                to: 1.0
                duration: 200
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                property: "x"
                from: 30
                to: 0
                duration: 200
                easing.type: Easing.InOutQuad
            }
        }
        pushExit: Transition {
            NumberAnimation {
                property: "opacity"
                from: 1.0
                to: 0.0
                duration: 200
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                property: "x"
                from: 0;
                to: -30
                duration: 200
                easing.type: Easing.InOutQuad
            }
        }
        popEnter: Transition {
        }
        popExit: Transition {
        }
        replaceEnter: Transition {
            NumberAnimation {
                property: "opacity"
                from: 0.0
                to: 1.0
                duration: 250
                easing.type: Easing.InOutQuad
            }
            // NumberAnimation {
            //     property: "x"
            //     from: 30 * (root.movingRight ? 1 : -1)
            //     to: 0
            //     duration: 200
            //     easing.type: Easing.InOutQuad
            // }
        }
        replaceExit: Transition {
            NumberAnimation {
                property: "opacity"
                from: 1.0
                to: 0.0
                duration: 100
                easing.type: Easing.InOutQuad
            }
            // NumberAnimation {
            //     property: "x"
            //     from: 0;
            //     to: 30 * (root.movingRight ? -1 : 1)
            //     duration: 200
            //     easing.type: Easing.InOutQuad
            // }
        }
    }

}