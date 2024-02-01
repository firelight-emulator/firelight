import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

ApplicationWindow {
    id: window
    width: 1280
    height: 720
    visible: true
    title: qsTr("Firelight")
    color: "#1c1b1f"
    // color: "transparent"
    Material.theme: Material.Dark
    Material.accent: Material.color("#ff704f")

    StackView {
        id: everything
        anchors.fill: parent
        focus: true

        initialItem: mainMenu

        popEnter: Transition {
        }
        popExit: Transition {
        }
        pushEnter: Transition {
        }
        pushExit: Transition {
        }
    }

    // Notification {
    // }

    // Rectangle {
    //     id: loadingIndicator
    //     parent: Overlay.overlay
    //     visible: true
    //
    //     anchors.left: parent.left
    //     anchors.bottom: parent.bottom
    //     anchors.bottomMargin: 30
    //     anchors.leftMargin: 30
    //
    //     color: "yellow"
    //     width: 200
    //     height: 50
    //     radius: 50
    //
    //     BusyIndicator {
    //         id: spinner
    //         width: 40
    //         height: 40
    //         anchors.verticalCenter: parent.verticalCenter
    //         anchors.left: parent.left
    //         anchors.leftMargin: 10
    //         // running: library_manager.scanning
    //         running: true
    //     }
    //
    //     Text {
    //         text: "Gettin ur games"
    //         font.pointSize: 12
    //         color: "red"
    //         anchors.left: spinner.right
    //         anchors.leftMargin: 15
    //         anchors.verticalCenter: parent.verticalCenter
    //         horizontalAlignment: Text.AlignHCenter
    //         verticalAlignment: Text.AlignVCenter
    //     }
    // }

    Component {
        id: mainMenu

        Item {
            // Text {
            //     x: 20
            //     y: 20
            //     text: qsTr("Library")
            //     font.pointSize: 24
            //     color: "white"
            //     horizontalAlignment: Text.AlignLeft
            //     verticalAlignment: Text.AlignTop
            // }

            // Rectangle {
            //     id: bar
            //
            //     height: 70
            //     anchors.top: parent.top
            //     anchors.left: parent.left
            //     anchors.right: parent.right
            //
            //     color: "transparent"
            //
            //     Button {
            //         id: cool
            //         width: 200
            //         height: 50
            //         // cursorShape: Qt.PointingHandCursor
            //
            //         anchors.centerIn: parent
            //
            //         text: "hey there"
            //     }


            // }

            TabBar {
                id: bar
                anchors.horizontalCenter: parent.horizontalCenter
                height: 50

                currentIndex: 2

                TabButton {
                    width: 125
                    height: bar.height
                    enabled: false
                    text: qsTr("Home")
                }
                TabButton {
                    width: 125
                    height: bar.height
                    enabled: false
                    text: qsTr("Explore")
                }
                TabButton {
                    width: 125
                    height: bar.height
                    text: qsTr("Library")
                    onClicked: content.replace(libraryPage)
                }
                TabButton {
                    width: 125
                    height: bar.height
                    enabled: false
                    text: qsTr("Controllers")
                }
                TabButton {
                    width: 125
                    height: bar.height
                    text: qsTr("Settings")
                    enabled: false
                }
            }

            StackView {
                id: content

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: bar.bottom
                anchors.bottom: parent.bottom

                anchors.topMargin: 20

                initialItem: libraryPage

                popEnter: Transition {
                }
                popExit: Transition {
                }
                pushEnter: Transition {
                }
                pushExit: Transition {
                }

                Component {
                    id: libraryPage
                    Item {
                        id: wrapper

                        Item {
                            id: filters
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            width: 200
                        }

                        ListView {
                            id: libraryList
                            focus: true
                            clip: true

                            ScrollBar.vertical: ScrollBar {
                                width: 15
                                interactive: true
                            }

                            currentIndex: 0
                            anchors.left: filters.right
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            anchors.right: parent.right

                            model: library_short_model
                            boundsBehavior: Flickable.StopAtBounds
                            delegate: gameListItem
                            // preferredHighlightBegin: height / 3
                            // preferredHighlightEnd: 2 * (height / 3) + currentItem.height
                        }

                        Component {
                            id: gameListItem

                            Rectangle {
                                id: wrapper

                                width: ListView.view.width
                                height: 60

                                color: mouseArea.containsMouse ? "#353438" : "transparent"

                                MouseArea {
                                    id: mouseArea
                                    hoverEnabled: true
                                    anchors.fill: parent

                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        gameLoader.loadGame(model.id)
                                    }
                                }

                                // Item {
                                //     id: picture
                                //     width: 100
                                //     height: parent.height
                                // }

                                Text {
                                    id: label
                                    anchors.top: parent.top
                                    anchors.topMargin: 5
                                    anchors.left: parent.left
                                    anchors.leftMargin: 10

                                    height: parent.height / 2
                                    font.pointSize: 16
                                    text: model.display_name
                                    color: "white"
                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter
                                }

                                Text {
                                    id: bottomLabel
                                    anchors.top: label.bottom
                                    anchors.bottom: parent.bottom
                                    anchors.bottomMargin: 10
                                    anchors.left: parent.left
                                    anchors.leftMargin: 10

                                    font.pointSize: 12
                                    text: "Nintendo 64"
                                    color: "#989898"
                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    GameLoader {
        id: gameLoader

        onGameLoaded: function (entryId, romData, saveData, corePath) {
            everything.push(emulatorPage, {
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

    Dialog {
        id: dialog
        title: "Title"
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.Ok | Dialog.Cancel

        onAccepted: console.log("Ok clicked")
        onRejected: console.log("Cancel clicked")
    }

    Component {
        id: quickMenu
        Rectangle {
            id: lol
            opacity: 0.8
            color: "black"

            Keys.onEscapePressed: {
                everything.pop()
            }
        }
    }

    Component {
        id: emulatorPage
        Item {
            property int currentLibraryEntryId
            property var romData
            property var saveData
            property string corePath

            // layer.enabled: true

            focus: true

            StackView.visible: true

            StackView.onActivated: function () {
                emulatorView.resume()
            }

            StackView.onDeactivating: function () {
                emulatorView.pause()
            }

            Keys.onEscapePressed: {
                emulatorView.pause()
                // everything.push(quickMenu)
            }


            Item {
                id: emulatorContainer
                anchors.fill: parent

                property bool blurred: false
                property var blurAmount: 0.0

                layer.enabled: true
                layer.effect: MultiEffect {
                    source: emulatorContainer
                    anchors.fill: emulatorContainer
                    blurEnabled: emulatorContainer.blurred
                    blurMultiplier: 5.0
                    blurMax: 64
                    blur: emulatorContainer.blurAmount
                }

                states: [
                    State {
                        name: "Blurred"
                        when: emulatorContainer.blurred
                        PropertyChanges {
                            target: emulatorContainer
                            blurAmount: 1.0
                        }
                    },
                    State {
                        name: "NotBlurred"
                        when: !emulatorContainer.blurred
                        PropertyChanges {
                            target: emulatorContainer
                            blurAmount: 0.0
                        }
                    }
                ]

                transitions: [
                    Transition {
                        from: "NotBlurred"
                        to: "Blurred"
                        SequentialAnimation {
                            ScriptAction {
                                script: {
                                    emulatorContainer.blurred = true;
                                }
                            }
                            NumberAnimation {
                                properties: "blurAmount"
                                duration: 300
                                easing.type: Easing.InOutQuad
                            }
                        }
                    },
                    Transition {
                        from: "Blurred"
                        to: "NotBlurred"
                        SequentialAnimation {
                            NumberAnimation {
                                properties: "blurAmount"
                                duration: 300
                                easing.type: Easing.InOutQuad
                            }
                            onStopped: {
                                emulatorContainer.blurred = false;
                            }
                        }
                    }
                ]


                EmulatorView {
                    id: emulatorView

                    property bool isFullScreen: false
                    anchors.centerIn: parent

                    // onOrphanPatchDetected: {
                    //     console.log("orphan patch detected")
                    //     everything.pop()
                    // }

                    Component.onCompleted: {
                        this.load(currentLibraryEntryId, romData, saveData, corePath)
                    }

                    states: [
                        State {
                            name: "FullScreenState"
                            when: emulatorView.isFullScreen
                            PropertyChanges {
                                target: emulatorView
                                width: parent.width
                                height: parent.height
                            }
                        },
                        State {
                            name: "CenterInState"
                            when: !emulatorView.isFullScreen
                            PropertyChanges {
                                target: emulatorView
                                width: 640
                                height: 480
                            }
                        }
                    ]

                    transitions: [
                        Transition {
                            from: "FullScreenState"
                            to: "CenterInState"
                            NumberAnimation {
                                properties: "width, height"
                                duration: 100
                                easing.type: Easing.InOutQuad
                            }
                        },
                        Transition {
                            from: "CenterInState"
                            to: "FullScreenState"
                            NumberAnimation {
                                properties: "width, height"
                                duration: 100
                                easing.type: Easing.InOutQuad
                            }
                        }
                    ]
                }

                // ShaderEffectSource {
                //     anchors.centerIn: parent
                //     hideSource: true
                //     sourceItem: emulatorView
                // }
            }


            // Button {
            //     text: "Back"
            //     onClicked: function () {
            //         emulatorView.isFullScreen = !emulatorView.isFullScreen;
            //         console.log("back clicked")
            //     }
            // }

            Button {
                text: "Blurry"
                onClicked: function () {
                    emulatorContainer.state = (emulatorContainer.state === "Blurred") ? "NotBlurred" : "Blurred";
                    // emulatorContainer.blurred = !emulatorContainer.blurred;
                    console.log("blurred clicked")
                }
            }

            // FpsMultiplier {
            //     id: fpsMultiplierView
            // }
            // Slider {
            //     id: fpsMultiplier
            //     // onMoved: { fpsMultiplierView.setSliderValue(value) }
            //     stepSize: 0.5
            //     value: 1
            //     to: 10
            // }
        }
    }

    Loader {
        id: emulatorLoader
        anchors.centerIn: parent
    }
}
