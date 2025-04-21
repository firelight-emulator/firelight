import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts 1.0
import Firelight 1.0

StackView {
    id: root

    Rectangle {
        color: "black"
        anchors.fill: parent
        z: -1
    }

    property var entryId: 0
    function load(entryId) {
        root.entryId = entryId
        emulatorLoader.active = true
        root.pushItem(emulatorLoader)
    }

    function unload() {
        emulatorLoader.active = false
        root.entryId = 0
        root.clear()
        root.unloaded()
        emulatorLoader.blurAmount = 0
    }

    signal unloaded()

    Keys.onEscapePressed: {
        if (root.depth === 1) {
            root.pushItem(quickMenu, {}, StackView.PushTransition)
        } else if (root.depth > 1) {
            root.popCurrentItem()
        }
    }

    pushEnter: Transition {
        NumberAnimation {
            property: "x"
            from: -20
            to: 0
            duration: 250
            easing.type: Easing.InOutQuad
        }
        NumberAnimation {
            property: "opacity"
            from: 0
            to: 1
            duration: 250
            easing.type: Easing.InOutQuad
        }
    }

    popExit: Transition {
         NumberAnimation {
             property: "x"
             from: 0
             to: -20
             duration: 250
             easing.type: Easing.InOutQuad
         }
         NumberAnimation {
             property: "opacity"
             from: 1
             to: 0
             duration: 250
             easing.type: Easing.InOutQuad
         }
     }

     pushExit: Transition {
           NumberAnimation {
               property: "blurAmount"
               from: 0
               to: 1
               duration: 250
               easing.type: Easing.InOutQuad
           }
       }
     popEnter: Transition {
          NumberAnimation {
              property: "blurAmount"
              from: 1
              to: 0
              duration: 250
              easing.type: Easing.InOutQuad
          }
      }
     replaceEnter: Transition {}
        replaceExit: Transition {}

    Component {
        id: emuPage
        NewEmulatorPage {
            // onRewindPointsReady: function(points) {
            //     screenStack.pushItem(rewindMenu, {
            //         model: points,
            //         aspectRatio: emulatorLoader.item.videoAspectRatio
            //     }, StackView.Immediate)
            // }
        }
    }

    Component {
        id: suspendPointMenu
        Item {
            SuspendPoints {
                id: suspendData
                // contentHash: emulatorLoader.contentHash
                contentHash: "8e2c29a1e65111fe2078359e685e7943"
                saveSlotNumber: 1
            }
            GridView {
                anchors.fill: parent
                anchors.leftMargin: (parent.width % cellWidth) / 2
                anchors.rightMargin: (parent.width % cellWidth) / 2
                model: suspendData.suspendPoints
                cellWidth: 300
                cellHeight: 250

                populate: Transition {
                    id: popTransition
                    SequentialAnimation {
                        PropertyAction {
                            property: "opacity"
                            value: 0
                        }
                        PauseAnimation {
                            duration: popTransition.ViewTransition.index * 30
                        }
                        ParallelAnimation {
                            PropertyAnimation {
                                property: "opacity"
                                from: 0
                                to: 1
                                duration: 200
                                easing.type: Easing.InOutQuad
                            }
                            PropertyAnimation {
                                property: "y"
                                from: popTransition.ViewTransition.destination.y + 20
                                to: popTransition.ViewTransition.destination.y
                                duration: 200
                                easing.type: Easing.InOutQuad
                            }
                        }
                    }
                }
                delegate: Pane {
                    required property var model
                    required property var index
                    width: GridView.view.cellWidth
                    height: GridView.view.cellHeight
                    padding: 8
                    background: Item {}

                    property bool showGlobalCursor: true
                    property var globalCursorProxy: button

                    contentItem: Button {
                        id: button
                        padding: 8

                        Component.onCompleted: {
                            console.log("y: " + y)
                        }

                        y: pressed ? 10 : 8

                        layer.enabled: true
                        layer.effect: MultiEffect {
                            source: button
                            anchors.fill: button
                            autoPaddingEnabled: true
                            shadowBlur: 1.0
                            shadowColor: 'black'
                            shadowOpacity: 0.5
                            shadowEnabled: true
                            shadowVerticalOffset: button.pressed ? 1 : 3
                        }

                        HoverHandler {
                            id: hoverHandler
                            cursorShape: Qt.PointingHandCursor
                        }
                        background: Rectangle {
                            color: "#222222"
                            opacity: 0.9
                            radius: 6
                            border.color: "#737373"

                            Rectangle {
                                id: highlight
                                anchors.fill: parent
                                color: "white"
                                opacity: hoverHandler.hovered ? 0.1 : 0
                            }
                        }
                        contentItem: ColumnLayout {
                            Image {
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                                fillMode: Image.PreserveAspectFit
                                source: model.image_url
                                visible: model.has_data
                            }
                            RowLayout {
                                Layout.preferredHeight: 32
                                Layout.fillWidth: true
                                Text {
                                    Layout.fillHeight: true
                                    text: "Slot " + (model.index + 1)
                                    font.pixelSize: 16
                                    font.family: Constants.regularFontFamily
                                    font.weight: Font.DemiBold
                                    color: "white"
                                    horizontalAlignment: Text.AlignLeft
                                    verticalAlignment: Text.AlignVCenter
                                }
                                Item {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                }
                                Text {
                                    Layout.fillHeight: true
                                    text: model.timestamp
                                    font.pixelSize: 16
                                    font.family: Constants.regularFontFamily
                                    font.weight: Font.Normal
                                    color: "#d1d1d1"
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

    Component {
        id: quickMenu
        Pane {
            padding: 32
            background: Item{}
            contentItem: RowLayout {
                spacing: 32
                 ColumnLayout {
                     Layout.maximumWidth: 400
                     Layout.alignment: Qt.AlignCenter
                     FirelightMenuItem {
                         id: resumeGameButton
                         labelText: "Resume Game"
                         focus: true
                         Layout.fillWidth: true
                         KeyNavigation.down: restartGameButton
                         // Layout.preferredWidth: parent.width / 2
                         Layout.preferredHeight: 40
                         Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                         checkable: false
                         alignRight: true
                         onClicked: function () {
                             root.popCurrentItem()
                         }
                     }
                     FirelightMenuItem {
                         id: restartGameButton
                         labelText: "Restart Game"
                         Layout.fillWidth: true
                         KeyNavigation.down: rewindButton
                         // Layout.preferredWidth: parent.width / 2
                         Layout.preferredHeight: 40
                         Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                         checkable: false
                         alignRight: true
                     }
                     FirelightMenuItem {
                         id: rewindButton
                         labelText: "Rewind"
                         KeyNavigation.down: loadSuspendPointButton
                         Layout.fillWidth: true
                         // Layout.preferredWidth: parent.width / 2
                         Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                         Layout.preferredHeight: 40
                         checkable: false
                         alignRight: true
                     }
                     Rectangle {
                         Layout.fillWidth: true
                         // Layout.preferredWidth: parent.width / 2
                         Layout.preferredHeight: 1
                         Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                         opacity: 0.3
                         color: "#dadada"
                     }
                     FirelightMenuItem {
                         id: loadSuspendPointButton
                         labelText: "Load Suspend Point"
                         Layout.fillWidth: true
                         KeyNavigation.down: createSuspendPointButton
                         // Layout.preferredWidth: parent.width / 2
                         Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                         Layout.preferredHeight: 40
                         checkable: false
                         alignRight: true
                         // enabled: false
                         onClicked: {
                             quickMenuStack.pushItem(suspendPointMenu, {}, StackView.Immediate)
                         }
                     }
                     FirelightMenuItem {
                         id: createSuspendPointButton
                         KeyNavigation.down: undo
                         labelText: "Create Suspend Point"
                         Layout.fillWidth: true
                         // Layout.preferredWidth: parent.width / 2
                         Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                         Layout.preferredHeight: 40
                         checkable: false
                         alignRight: true
                         enabled: false
                     }
                     FirelightMenuItem {
                         id: undo
                         labelText: "Undo Last Load"
                         Layout.fillWidth: true
                         KeyNavigation.down: closeGameButton
                         // Layout.preferredWidth: parent.width / 2
                         Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                         Layout.preferredHeight: 40
                         checkable: false
                         alignRight: true
                     }
                     Rectangle {
                         Layout.fillWidth: true
                         // Layout.preferredWidth: parent.width / 2
                         Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                         Layout.preferredHeight: 1
                         opacity: 0.3
                         color: "#dadada"
                     }
                     FirelightMenuItem {
                         id: closeGameButton
                         labelText: "Close Game"
                         Layout.fillWidth: true
                         // Layout.preferredWidth: parent.width / 2
                         Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                         Layout.preferredHeight: 40
                         checkable: false
                         alignRight: true

                         onClicked: {
                             closeGameDialog.open()
                         }
                     }
                 }

                 StackView {
                     id: quickMenuStack
                     Layout.fillWidth: true
                     Layout.fillHeight: true
                 }

             }
        }

    }

    // initialItem: emulatorLoader

    Loader {
        id: emulatorLoader
        property var gameName
        property var entryId: root.entryId
        property var contentHash
        property bool shouldDeactivate: false

        // StackView.onActivated: {
        //     paused = false
        // }

        StackView.visible: true

        property real blurAmount: 0

        Behavior on blurAmount {
            NumberAnimation {
                easing.type: Easing.InOutQuad
                duration: 250
            }
        }

        layer.enabled: blurAmount !== 0
        layer.effect: MultiEffect {
            // enabled: root.blurAmount !== 0
            source: emulatorLoader
            anchors.fill: emulatorLoader
            blurEnabled: true
            blurMultiplier: 0
            blurMax: 64
            autoPaddingEnabled: false
            blur: emulatorLoader.blurAmount
        }


        Rectangle {
            id: dimmer
            color: "black"
            // opacity: emulatorLoader.StackView.status === StackView.Active ? 0 * 0.4
            // opacity: emulatorLoader.StackView.status !== StackView.Active ? 0.4 : 0
            opacity: emulatorLoader.blurAmount * 0.4
            anchors.fill: parent

            // Behavior on opacity {
            //     NumberAnimation {
            //         duration: 250
            //         easing.type: Easing.InOutQuad
            //     }
            // }

            z: 10
        }

        focus: true

        active: false
        sourceComponent: emuPage

        property bool paused: emulatorLoader.StackView.status !== StackView.Active
        function stopGame() {
            shouldDeactivate = true
            if (emulatorLoader.StackView.status === StackView.Active) {
                emulatorLoader.StackView.view.popCurrentItem()
            }
        }

        onPausedChanged: function () {
            if (emulatorLoader.item != null) {
                emulatorLoader.item.paused = emulatorLoader.paused
            }
        }

        // onActiveChanged: {
        //     if (!active && emulatorLoader.StackView.status === StackView.Active) {
        //         emulatorLoader.StackView.view.popCurrentItem()
        //     }
        // }

        // StackView.onDeactivated: {
        //     if (shouldDeactivate) {
        //         active = false
        //         shouldDeactivate = false
        //     }
        // }
        //
        // StackView.onActivating: {
        //     // setSource(emuPage, {
        //     //     gameData: emulatorLoader.gameData,
        //     //     saveData: emulatorLoader.saveData,
        //     //     corePath: emulatorLoader.corePath,
        //     //     contentHash: emulatorLoader.contentHash,
        //     //     saveSlotNumber: emulatorLoader.saveSlotNumber,
        //     //     platformId: emulatorLoader.platformId,
        //     //     contentPath: emulatorLoader.contentPath
        //     // })
        //     // active = true
        // }

        onLoaded: function () {
            const emu = (emulatorLoader.item as NewEmulatorPage)

            emu.pausedChanged.connect(function( ){
                console.log("game started")
            })

            emu.closing.connect(function() {
                emulatorLoader.active = false
                root.unloaded()
            })

            // overlay.opacity = 0

            emu.loadGame(emulatorLoader.entryId)
            emu.forceActiveFocus()
            // emulatorLoader.item.startGame()
            // emulatorLoader.item.paused = emulatorLoader.paused
            //
            // emulatorLoader.gameName = emulatorLoader.item.gameName
            // emulatorLoader.contentHash = emulatorLoader.item.contentHash
            // emulatorLoader.item.startGame(gameData, saveData, corePath, contentHash, saveSlotNumber, platformId, contentPath)
            // emulatorLoader.item.paused = emulatorLoader.paused
        }
    }

    FirelightDialog {
        id: closeGameDialog
        text: "Are you sure you want to close the game?"
        onAccepted: {
            root.unload()
        }
    }

}
