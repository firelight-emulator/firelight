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
    color: Constants.colorTestBackground
    // color: "transparent"
    // Material.theme: Material.Light
    // Material.accent: Material.DeepOrange
    // Material.primary: Material.Indigo

    Component {
        id: homePage

        Item {
            Rectangle {
                radius: 12
                anchors.fill: parent
                color: Constants.colorTestSurface
            }

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

    Pane {
        id: fullPane
        anchors.fill: parent
        padding: 12

        background: Rectangle {
            color: "transparent"
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
                    text: "Firelight is made with love by BiscuitCakes"
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

    FontLoader {
        id: localFont
        source: "qrc:/fonts/lexend"
    }

    FontLoader {
        id: lexendLight
        source: "qrc:/fonts/lexend-light"
    }


    // Pane {
    //     id: content
    //     anchors.centerIn: parent
    //
    //     ColumnLayout {
    //         anchors.fill: parent
    //         spacing: 10
    //         Layout.alignment: Qt.AlignHCenter
    //
    //         Text {
    //             id: header
    //             text: "Welcome to Firelight"
    //             font.family: localFont.name
    //             font.pointSize: 16
    //             Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
    //         }
    //
    //         Text {
    //             id: subtitle
    //             text: "This is where I put a bunch of text saying something like hey, \nthanks for downloading the app!\n"
    //             font.pointSize: 11
    //             font.family: lexendLight .name
    //             wrapMode: Text.WordWrap
    //             horizontalAlignment: Text.AlignHCenter
    //             verticalAlignment: Text.AlignVCenter
    //             Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
    //         }
    //
    //         // MenuSeparator {
    //         //     padding: 0
    //         //     topPadding: 5
    //         //     bottomPadding: 12
    //         //     contentItem: Rectangle {
    //         //         implicitWidth: 200
    //         //         implicitHeight: 1
    //         //         color: username.border.color
    //         //     }
    //         // }
    //         Text {
    //             text: "Display Name"
    //             font.family: localFont.name
    //             horizontalAlignment: Text.AlignLeft
    //             verticalAlignment: Text.AlignVCenter
    //         }
    //
    //         TextField {
    //             id: username
    //             Layout.fillWidth: true
    //             // placeholderText: "Display Name"
    //         }
    //
    //         Text {
    //             text: "ROM Directory"
    //             font.family: localFont.name
    //             horizontalAlignment: Text.AlignLeft
    //             verticalAlignment: Text.AlignVCenter
    //         }
    //
    //         RowLayout {
    //             TextField {
    //                 readOnly: true
    //                 Layout.fillWidth: true
    //                 id: directory
    //             }
    //
    //             Button {
    //                 Layout.fillHeight: true
    //                 text: "Choose"
    //                 font.family: localFont.name
    //                 onClicked: {
    //                     folderDialog.open()
    //                 }
    //             }
    //
    //             FolderDialog {
    //                 id: folderDialog
    //                 title: "Select Directory"
    //                 // currentFolder: "/"
    //                 onAccepted: {
    //                     console.log("Selected folder: " + folderDialog.selectedFolder);
    //                     // Check if folderDialog.folder is a valid URL and has a scheme
    //                     if (folderDialog.selectedFolder.toString().length === 0) {
    //                         console.error("Invalid or empty folder path.");
    //                         return;
    //                     }
    //                 }
    //             }
    //         }
    //
    //         Button {
    //             text: "Next"
    //             Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
    //             font.family: localFont.name
    //             onClicked: {
    //                 console.log("hi")
    //             }
    //         }
    //     }
    // }
    // StackView {
    //     id: everything
    //     anchors.fill: parent
    //     focus: true
    //
    //     initialItem: mainMenu
    //
    //     popEnter: Transition {
    //     }
    //     popExit: Transition {
    //     }
    //     pushEnter: Transition {
    //     }
    //     pushExit: Transition {
    //     }
    // }
    //
    // // Notification {
    // // }
    //
    // // Rectangle {
    // //     id: loadingIndicator
    // //     parent: Overlay.overlay
    // //     visible: true
    // //
    // //     anchors.left: parent.left
    // //     anchors.bottom: parent.bottom
    // //     anchors.bottomMargin: 30
    // //     anchors.leftMargin: 30
    // //
    // //     color: "yellow"
    // //     width: 200
    // //     height: 50
    // //     radius: 50
    // //
    // //     BusyIndicator {
    // //         id: spinner
    // //         width: 40
    // //         height: 40
    // //         anchors.verticalCenter: parent.verticalCenter
    // //         anchors.left: parent.left
    // //         anchors.leftMargin: 10
    // //         // running: library_manager.scanning
    // //         running: true
    // //     }
    // //
    // //     Text {
    // //         text: "Gettin ur games"
    // //         font.pointSize: 12
    // //         color: "red"
    // //         anchors.left: spinner.right
    // //         anchors.leftMargin: 15
    // //         anchors.verticalCenter: parent.verticalCenter
    // //         horizontalAlignment: Text.AlignHCenter
    // //         verticalAlignment: Text.AlignVCenter
    // //     }
    // // }
    //
    // Component {
    //     id: mainMenu
    //
    //     Item {
    //         // Text {
    //         //     x: 20
    //         //     y: 20
    //         //     text: qsTr("Library")
    //         //     font.pointSize: 24
    //         //     color: "white"
    //         //     horizontalAlignment: Text.AlignLeft
    //         //     verticalAlignment: Text.AlignTop
    //         // }
    //
    //         // Rectangle {
    //         //     id: bar
    //         //
    //         //     height: 70
    //         //     anchors.top: parent.top
    //         //     anchors.left: parent.left
    //         //     anchors.right: parent.right
    //         //
    //         //     color: "transparent"
    //         //
    //         //     Button {
    //         //         id: cool
    //         //         width: 200
    //         //         height: 50
    //         //         // cursorShape: Qt.PointingHandCursor
    //         //
    //         //         anchors.centerIn: parent
    //         //
    //         //         text: "hey there"
    //         //     }
    //
    //
    //         // }
    //
    //         TabBar {
    //             id: bar
    //             anchors.horizontalCenter: parent.horizontalCenter
    //             height: 50
    //
    //             currentIndex: 2
    //
    //             TabButton {
    //                 width: 125
    //                 height: bar.height
    //                 font.family: localFont.name
    //                 enabled: false
    //                 text: qsTr("Home")
    //             }
    //             TabButton {
    //                 width: 125
    //                 height: bar.height
    //                 font.family: localFont.name
    //                 enabled: false
    //                 text: qsTr("Explore")
    //             }
    //             TabButton {
    //                 width: 125
    //                 height: bar.height
    //                 font.family: localFont.name
    //                 text: qsTr("Library")
    //                 onClicked: content.replace(libraryPage)
    //             }
    //             TabButton {
    //                 width: 125
    //                 font.family: localFont.name
    //                 height: bar.height
    //                 enabled: false
    //                 text: qsTr("Controllers")
    //             }
    //             TabButton {
    //                 width: 125
    //                 height: bar.height
    //                 font.family: localFont.name
    //                 text: qsTr("Settings")
    //                 enabled: false
    //             }
    //         }
    //
    //         StackView {
    //             id: content
    //
    //             anchors.left: parent.left
    //             anchors.right: parent.right
    //             anchors.top: bar.bottom
    //             anchors.bottom: parent.bottom
    //
    //             anchors.topMargin: 20
    //
    //             initialItem: libraryPage
    //
    //             popEnter: Transition {
    //             }
    //             popExit: Transition {
    //             }
    //             pushEnter: Transition {
    //             }
    //             pushExit: Transition {
    //             }
    //
    //             Component {
    //                 id: libraryPage
    //                 Item {
    //                     id: wrapper
    //
    //                     Item {
    //                         id: filters
    //                         anchors.left: parent.left
    //                         anchors.top: parent.top
    //                         anchors.bottom: parent.bottom
    //                         width: 200
    //                     }
    //
    //                     ListView {
    //                         id: libraryList
    //                         focus: true
    //                         clip: true
    //
    //                         ScrollBar.vertical: ScrollBar {
    //                             width: 15
    //                             interactive: true
    //                         }
    //
    //                         currentIndex: 0
    //                         anchors.left: filters.right
    //                         anchors.top: parent.top
    //                         anchors.bottom: parent.bottom
    //                         anchors.right: parent.right
    //
    //                         model: library_short_model
    //                         boundsBehavior: Flickable.StopAtBounds
    //                         delegate: gameListItem
    //                         // preferredHighlightBegin: height / 3
    //                         // preferredHighlightEnd: 2 * (height / 3) + currentItem.height
    //                     }
    //
    //                     Component {
    //                         id: gameListItem
    //
    //                         Rectangle {
    //                             id: wrapper
    //
    //                             width: ListView.view.width
    //                             height: 60
    //
    //                             color: mouseArea.containsMouse ? "#353438" : "transparent"
    //
    //                             MouseArea {
    //                                 id: mouseArea
    //                                 hoverEnabled: true
    //                                 anchors.fill: parent
    //
    //                                 cursorShape: Qt.PointingHandCursor
    //                                 onClicked: {
    //                                     gameLoader.loadGame(model.id)
    //                                 }
    //                             }
    //
    //                             // Item {
    //                             //     id: picture
    //                             //     width: 100
    //                             //     height: parent.height
    //                             // }
    //
    //                             Text {
    //                                 id: label
    //                                 anchors.top: parent.top
    //                                 anchors.topMargin: 5
    //                                 anchors.left: parent.left
    //                                 anchors.leftMargin: 10
    //
    //                                 height: parent.height / 2
    //                                 font.pointSize: 16
    //                                 font.family: localFont.name
    //                                 text: model.display_name
    //                                 color: "white"
    //                                 horizontalAlignment: Text.AlignLeft
    //                                 verticalAlignment: Text.AlignVCenter
    //                             }
    //
    //                             Text {
    //                                 id: bottomLabel
    //                                 anchors.top: label.bottom
    //                                 anchors.bottom: parent.bottom
    //                                 anchors.bottomMargin: 10
    //                                 anchors.left: parent.left
    //                                 anchors.leftMargin: 10
    //
    //                                 font.family: lexendLight.name
    //                                 font.pointSize: 12
    //                                 text: "Nintendo 64"
    //                                 color: "#989898"
    //                                 horizontalAlignment: Text.AlignLeft
    //                                 verticalAlignment: Text.AlignVCenter
    //                             }
    //                         }
    //                     }
    //                 }
    //             }
    //         }
    //     }
    // }
    //
    // GameLoader {
    //     id: gameLoader
    //
    //     onGameLoaded: function (entryId, romData, saveData, corePath) {
    //         everything.push(emulatorPage, {
    //             currentLibraryEntryId: entryId,
    //             romData: romData,
    //             saveData: saveData,
    //             corePath: corePath
    //         })
    //     }
    //
    //     onGameLoadFailedOrphanedPatch: function (entryId) {
    //         dialog.open()
    //     }
    // }
    //
    // Dialog {
    //     id: dialog
    //     title: "Title"
    //     modal: true
    //     anchors.centerIn: parent
    //     standardButtons: Dialog.Ok | Dialog.Cancel
    //
    //     onAccepted: console.log("Ok clicked")
    //     onRejected: console.log("Cancel clicked")
    // }
    //
    // Component {
    //     id: quickMenu
    //     Rectangle {
    //         id: lol
    //         opacity: 0.8
    //         color: "black"
    //
    //         Keys.onEscapePressed: {
    //             everything.pop()
    //         }
    //     }
    // }
    //
    //
    // Component {
    //     id: emulatorPage
    //     Item {
    //         property int currentLibraryEntryId
    //         property var romData
    //         property var saveData
    //         property string corePath
    //
    //         focus: true
    //
    //         StackView.visible: true
    //
    //         StackView.onActivated: function () {
    //             emulatorView.resume()
    //         }
    //
    //         StackView.onDeactivating: function () {
    //             emulatorView.pause()
    //         }
    //
    //         Keys.onEscapePressed: {
    //             emulatorView.pause()
    //             // everything.push(quickMenu)
    //         }
    //
    //
    //         Item {
    //             id: emulatorContainer
    //             anchors.fill: parent
    //
    //             property bool blurred: false
    //             property var blurAmount: 0.0
    //
    //             layer.enabled: true
    //             layer.effect: MultiEffect {
    //                 source: emulatorContainer
    //                 anchors.fill: emulatorContainer
    //                 blurEnabled: emulatorContainer.blurred
    //                 blurMultiplier: 3.0
    //                 blurMax: 64
    //                 blur: emulatorContainer.blurAmount
    //             }
    //
    //             states: [
    //                 State {
    //                     name: "Blurred"
    //                     when: emulatorContainer.blurred
    //                     PropertyChanges {
    //                         target: emulatorContainer
    //                         blurAmount: 1.0
    //                     }
    //                 },
    //                 State {
    //                     name: "NotBlurred"
    //                     when: !emulatorContainer.blurred
    //                     PropertyChanges {
    //                         target: emulatorContainer
    //                         blurAmount: 0.0
    //                     }
    //                 }
    //             ]
    //
    //             transitions: [
    //                 Transition {
    //                     from: "NotBlurred"
    //                     to: "Blurred"
    //                     SequentialAnimation {
    //                         ScriptAction {
    //                             script: {
    //                                 emulatorView.paused = true
    //                                 emulatorContainer.blurred = true;
    //                             }
    //                         }
    //                         NumberAnimation {
    //                             properties: "blurAmount"
    //                             duration: 200
    //                             easing.type: Easing.InOutQuad
    //                         }
    //                     }
    //                 },
    //                 Transition {
    //                     from: "Blurred"
    //                     to: "NotBlurred"
    //                     SequentialAnimation {
    //                         NumberAnimation {
    //                             properties: "blurAmount"
    //                             duration: 150
    //                             easing.type: Easing.InOutQuad
    //                         }
    //                         ScriptAction {
    //                             script: {
    //                                 emulatorView.paused = false
    //                             }
    //                         }
    //                         onStopped: {
    //                             emulatorContainer.blurred = false;
    //                         }
    //                     }
    //                 }
    //             ]
    //
    //
    //             EmulatorView {
    //                 id: emulatorView
    //
    //                 property bool isFullScreen: false
    //                 anchors.centerIn: parent
    //
    //                 // onOrphanPatchDetected: {
    //                 //     console.log("orphan patch detected")
    //                 //     everything.pop()
    //                 // }
    //
    //                 Component.onCompleted: {
    //                     this.load(currentLibraryEntryId, romData, saveData, corePath)
    //                 }
    //
    //                 states: [
    //                     State {
    //                         name: "FullScreenState"
    //                         when: emulatorView.isFullScreen
    //                         PropertyChanges {
    //                             target: emulatorView
    //                             width: parent.height * 1.333
    //                             height: parent.height
    //                         }
    //                     },
    //                     State {
    //                         name: "CenterInState"
    //                         when: !emulatorView.isFullScreen
    //                         PropertyChanges {
    //                             target: emulatorView
    //                             width: 640
    //                             height: 480
    //                         }
    //                     }
    //                 ]
    //
    //                 transitions: [
    //                     Transition {
    //                         from: "FullScreenState"
    //                         to: "CenterInState"
    //                         NumberAnimation {
    //                             properties: "width, height"
    //                             duration: 100
    //                             easing.type: Easing.InOutQuad
    //                         }
    //                     },
    //                     Transition {
    //                         from: "CenterInState"
    //                         to: "FullScreenState"
    //                         NumberAnimation {
    //                             properties: "width, height"
    //                             duration: 100
    //                             easing.type: Easing.InOutQuad
    //                         }
    //                     }
    //                 ]
    //             }
    //         }
    //
    //
    //         Button {
    //             text: "Back"
    //             onClicked: function () {
    //                 emulatorView.isFullScreen = !emulatorView.isFullScreen;
    //                 console.log("back clicked")
    //             }
    //         }
    //
    //         // Button {
    //         //     text: "Blurry"
    //         //     onClicked: function () {
    //         //         emulatorContainer.state = (emulatorContainer.state === "Blurred") ? "NotBlurred" : "Blurred";
    //         //         // fileDialog.open()
    //         //         console.log("blurred clicked")
    //         //     }
    //         // }
    //
    //         // FpsMultiplier {
    //         //     id: fpsMultiplierView
    //         // }
    //         // Slider {
    //         //     id: fpsMultiplier
    //         //     // onMoved: { fpsMultiplierView.setSliderValue(value) }
    //         //     stepSize: 0.5
    //         //     value: 1
    //         //     to: 10
    //         // }
    //     }
    // }
    //
    // Loader {
    //     id: emulatorLoader
    //     anchors.centerIn: parent
    // }
}
