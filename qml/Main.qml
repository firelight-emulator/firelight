import QtQuick
import QtQuick.Controls
// import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0
import FirelightStyle 1.0

ApplicationWindow {
    id: window
    width: 1280
    height: 720
    visible: true
    title: qsTr("Firelight")

    Component {
        id: homePage

        Item {
            // Rectangle {
            //     radius: 12
            //     anchors.fill: parent
            //     color: Constants.colorTestSurface
            // }

            Text {
                text: "Here's where the Home menu will go!"
                anchors.centerIn: parent
                color: Constants.colorTestTextMuted
                font.pointSize: 16
                font.family: localFont.name
            }
        }
    }

    Component {
        id: explorePage

        Item {
            Rectangle {
                radius: 12
                anchors.fill: parent
                color: Constants.colorTestSurface
            }

            Text {
                text: "Here's where the Explore menu will go!"
                anchors.centerIn: parent
                color: Constants.colorTestTextMuted
                font.pointSize: 16
                font.family: localFont.name
            }
        }
    }

    Component {
        id: libraryPage
        LibraryPage {
            fontFamilyName: localFont.name
            onEntryClicked: function (entryId) {
                if (emulatorPage.doingAThing) {
                    console.log("already doing a thing!!!!!!")
                } else {
                    gameLoader.loadGame(entryId)
                }

                console.log(emulatorPage)
                // // background.visible = !background.visible
                // console.log("entry clicked: " + entryId)
                // gameLoader.loadGame(entryId)
            }
        }
    }

    Component {
        id: controllersPage
        Item {
            Rectangle {
                radius: 12
                anchors.fill: parent
                color: Constants.colorTestSurface
            }

            Text {
                text: "Here's where the Controllers menu will go!"
                anchors.centerIn: parent
                color: Constants.colorTestTextMuted
                font.pointSize: 16
                font.family: localFont.name
            }
        }
    }

    Component {
        id: settingsPage
        Item {
            Rectangle {
                radius: 12
                anchors.fill: parent
                color: Constants.colorTestSurface
            }

            Text {
                text: "Here's where the Settings menu will go!"
                anchors.centerIn: parent
                color: Constants.colorTestTextMuted
                font.pointSize: 16
                font.family: localFont.name
            }
        }
    }

    Component {
        id: emulatorPage
        EmulatorPage {
            Keys.onEscapePressed: {
                // background.visible = false
                everything.push(mainMenu)
            }
        }
    }

    StackView {
        id: everything
        anchors.fill: parent
        focus: true

        initialItem: mainMenu

        popEnter: Transition {
        }
        popExit: Transition {
            ParallelAnimation {
                PropertyAnimation {
                    property: "scale"
                    from: 1
                    to: 1.2
                    duration: 300
                    easing.type: Easing.InOutQuad
                }
                PropertyAnimation {
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: 300
                    easing.type: Easing.InOutQuad
                }
            }
        }
        pushEnter: Transition {
            ParallelAnimation {
                PropertyAnimation {
                    property: "scale"
                    from: 1.2
                    to: 1
                    duration: 300
                    easing.type: Easing.InOutQuad
                }
                PropertyAnimation {
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 300
                    easing.type: Easing.InOutQuad
                }
            }
        }
        pushExit: Transition {
        }
        replaceEnter: Transition {
        }
        replaceExit: Transition {
        }
    }

    Component {
        id: mainMenu
        Pane {
            id: fullPane
            // padding: 12

            Keys.onEscapePressed: {
                if (StackView.index > 0) {
                    var me = everything.get(1)
                    everything.replace(null, me)
                    // everything.pop()
                }
            }

            StackView.onActivating: function () {
                background.visible = everything.depth <= 1;
            }

            StackView.onDeactivating: function () {
            }

            background: Rectangle {
                color: Constants.colorTestBackground
            }

            contentItem: Rectangle {
                color: "transparent"

                NavigationRail {
                    id: navRail
                    fontFamily: localFont.name

                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: 200

                    onHomeClicked: {
                        if (content.newIndex === 0 || content.currentIndex === 0) {
                            return
                        }
                        content.newIndex = 0
                        content.replace(homePage)
                    }

                    onExploreClicked: {
                        if (content.newIndex === 1 || content.currentIndex === 1) {
                            return
                        }
                        content.newIndex = 1
                        content.replace(explorePage)
                    }

                    onLibraryClicked: {
                        if (content.newIndex === 2 || content.currentIndex === 2) {
                            return
                        }
                        content.newIndex = 2
                        content.replace(libraryPage)
                    }

                    onControllersClicked: {
                        if (content.newIndex === 3 || content.currentIndex === 3) {
                            return
                        }
                        content.newIndex = 3
                        content.replace(controllersPage)
                    }

                    onSettingsClicked: {
                        if (content.newIndex === 4 || content.currentIndex === 4) {
                            return
                        }
                        content.newIndex = 4
                        content.replace(settingsPage)
                    }
                }

                Rectangle {
                    id: header
                    height: 28
                    anchors.top: parent.top
                    anchors.left: navRail.right
                    anchors.leftMargin: 12
                    anchors.right: parent.right
                    color: "transparent"

                    Text {
                        text: "6:30pm"
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignRight
                        verticalAlignment: Text.AlignVCenter
                        color: Constants.colorTestText
                        font.pointSize: 12
                        font.family: localFont.name
                    }
                }

                StackView {
                    id: content
                    property int currentIndex: 0
                    property int newIndex: 0

                    clip: true

                    anchors.topMargin: 12
                    anchors.top: header.bottom
                    anchors.left: navRail.right
                    anchors.leftMargin: 12
                    anchors.right: parent.right
                    anchors.bottom: footer.top
                    anchors.bottomMargin: 12

                    initialItem: homePage

                    popEnter: Transition {
                    }
                    popExit: Transition {
                    }
                    pushEnter: Transition {
                    }
                    pushExit: Transition {
                    }

                    replaceEnter: Transition {
                        SequentialAnimation {
                            ParallelAnimation {
                                PropertyAnimation {
                                    property: "y"
                                    from: (content.newIndex > content.currentIndex) ? 50 : -50
                                    to: 0
                                    duration: 300
                                    easing.type: Easing.InOutQuad
                                }
                                PropertyAnimation {
                                    property: "opacity"
                                    from: 0
                                    to: 1
                                    duration: 300
                                    easing.type: Easing.InOutQuad
                                }
                            }
                            ScriptAction {
                                script: {
                                    content.currentIndex = content.newIndex
                                }
                            }
                        }
                    }

                    replaceExit: Transition {
                        SequentialAnimation {
                            ParallelAnimation {
                                PropertyAnimation {
                                    property: "y"
                                    from: 0
                                    to: (content.newIndex > content.currentIndex) ? -50 : 50
                                    duration: 300
                                    easing.type: Easing.InOutQuad
                                }
                                PropertyAnimation {
                                    property: "opacity"
                                    from: 1
                                    to: 0
                                    duration: 300
                                    easing.type: Easing.InOutQuad
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    id: footer
                    height: 8
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    color: "transparent"

                    Text {
                        text: "Firelight is made with ❤️ by BiscuitCakes"
                        anchors.centerIn: parent
                        color: Constants.colorTestTextMuted
                        font.pointSize: 8
                        font.family: lexendLight.name
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }

        }
    }


    FontLoader {
        id: localFont
        source: "qrc:/fonts/lexend"
    }

    FontLoader {
        id: lexendLight
        source: "qrc:/fonts/lexend-light"
    }

    GameLoader {
        id: gameLoader

        onGameLoaded: function (entryId, romData, saveData, corePath) {
            everything.replace(emulatorPage, {
                currentLibraryEntryId: entryId,
                romData: romData,
                saveData: saveData,
                corePath: corePath
            })
        }

        onGameLoadFailedOrphanedPatch: function (entryId) {
            dialog.open()
        }
    }
}
