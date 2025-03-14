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
            spacing: 48

            Button {
                Layout.fillHeight: true
                Layout.fillWidth: true
                property bool showGlobalCursor: true
                Layout.horizontalStretchFactor: 1
                KeyNavigation.right: directoriesButton
                focus: true
                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }
                onClicked: function () {
                    root.currentIndex = 0
                }
                background: Rectangle {
                    color: "transparent"
                    radius: 4
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
                id: directoriesButton
                Layout.fillHeight: true
                Layout.fillWidth: true
                property bool showGlobalCursor: true
                Layout.horizontalStretchFactor: 1
                KeyNavigation.right: achievementsButton
                background: Rectangle {
                    color: "transparent"
                    radius: 4
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
                id: achievementsButton
                Layout.fillHeight: true
                Layout.fillWidth: true
                property bool showGlobalCursor: true
                Layout.horizontalStretchFactor: 1
                KeyNavigation.right: readyButton
                background: Rectangle {
                    color: "transparent"
                    radius: 4
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
                id: readyButton
                Layout.fillHeight: true
                Layout.fillWidth: true
                property bool showGlobalCursor: true
                Layout.horizontalStretchFactor: 1
                background: Rectangle {
                    color: "transparent"
                    radius: 4
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
                    source: "qrc:/images/firelight-logo"
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
                    focus: true
                    description: "This is where you’ll put your game files. Firelight will automatically detect files in this directory and add them to your library."
                    value: UserLibrary.mainGameDirectory

                    onValueChanged: function () {
                        UserLibrary.mainGameDirectory = value
                    }
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
                RetroAchievementsAccountPane {
                    id: accountPane
                    focus: true
                    KeyNavigation.down: achievement_manager.loggedIn ? hardcoreToggle : dirNextButton
                    Layout.fillWidth: true
                    Layout.fillHeight: false
                }

                ToggleOption {
                    id: hardcoreToggle
                    Layout.fillWidth: true
                    visible: achievement_manager.loggedIn
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
                    label: "Learn more"
                    visible: achievement_manager.loggedIn
                    KeyNavigation.down: dirNextButton
                    Layout.topMargin: 8
                    Layout.alignment: Qt.AlignHCenter
                    onClicked: {
                        Qt.openUrlExternally("https://docs.retroachievements.org/general/faq.html#what-is-hardcore-mode")
                    }
                }
                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
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
                        KeyNavigation.up: !achievement_manager.loggedIn ? accountPane : null
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
                    focus: true
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
        anchors.topMargin: 12
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