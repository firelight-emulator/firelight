import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts 1.0
import Firelight 1.0

StackView {
    id: root

    property alias running: emulatorLoader.active

    TapHandler {

    }

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

    // Component {
    //     id: achievementsView
    //     AchievementSetView {
    //         contentHash: emulatorLoader.item.contentHash
    //     }
    // }

    Component {
        id: emuPage
        NewEmulatorPage {
            id: emuPage
            onAboutToRunFrame: function() {
                bufferGraph.addValue(emuPage.audioBufferLevel)
            }
            onRewindPointsReady: function(points) {
                root.pushItem(rewindMenu, {
                    model: points,
                    aspectRatio: emulatorLoader.item.videoAspectRatio
                }, StackView.Immediate)
            }
        }
    }

    Component {
        id: suspendPointMenu
        Item {
            SuspendPoints {
                id: suspendData
                contentHash: emulatorLoader.item.contentHash
                // contentHash: "8e2c29a1e65111fe2078359e685e7943"
                saveSlotNumber: 1
            }

            FirelightDialog {
                id: deleteSuspendPointDialog
                property int index: 0
                text: "Are you sure you want to delete the Suspend Point in slot " + (index + 1) + "?"
                onAccepted: {
                    suspendData.deleteSuspendPoint(index)
                    index = 0
                }
            }

            FirelightDialog {
                id: overwriteSuspendPointDialog
                property int index: 0
                text: "Are you sure you want to overwrite the data in slot " + (index + 1) + "?"
                onAccepted: {
                    emulatorLoader.item.writeSuspendPoint(index)
                    index = 0
                }
            }

            GridView {
                id: suspendPointGrid
                anchors.fill: parent
                anchors.leftMargin: (parent.width % cellWidth) / 2
                anchors.rightMargin: (parent.width % cellWidth) / 2
                model: suspendData.suspendPoints
                cellWidth: 300
                cellHeight: 360

                header: Pane {
                    width: GridView.view.width
                    verticalPadding: 0
                    horizontalPadding: 8
                    background: Item {}
                    contentItem: ColumnLayout {
                        spacing: 8
                        RowLayout {
                            spacing: 12
                            Layout.preferredHeight: 40
                            Layout.fillWidth: true
                            Text {
                                Layout.fillHeight: true
                                text: "Suspend Points"
                                color: "white"
                                font.family: Constants.regularFontFamily
                                font.pixelSize: 22
                                font.weight: Font.DemiBold
                                verticalAlignment: Text.AlignVCenter
                            }
                            Item {
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                            }
                            Button {
                                id: undoButton
                                Layout.fillHeight: true
                                horizontalPadding: 12
                                background: Rectangle {
                                    color: "white"
                                    opacity: parent.pressed ? 0.16 : 0.1
                                    radius: 4
                                    visible: createHoverHandler.hovered && parent.enabled
                                }
                                HoverHandler {
                                    id: createHoverHandler
                                    cursorShape: parent.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                                }
                                contentItem: RowLayout {
                                    spacing: 8
                                    FLIcon {
                                        id: createIcon
                                        icon: "undo"
                                        Layout.fillHeight: true
                                        size: height
                                        color: undoButton.enabled ? "white" : "#737373"
                                    }
                                    Text {
                                        Layout.fillHeight: true
                                        Layout.fillWidth: true
                                         text: "Undo last load"
                                         color: undoButton.enabled ? "white" : "#737373"
                                         font.family: Constants.regularFontFamily
                                         font.pixelSize: 16
                                         font.weight: Font.Medium
                                         verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment: Text.AlignHCenter
                                     }
                                 }

                                 enabled: emulatorLoader.item.canUndoLoadSuspendPoint

                                 onClicked: {
                                    emulatorLoader.item.undoLoadSuspendPoint()
                                 }
                            }

                        }
                        // RowLayout {
                        //     spacing: 12
                        //     Layout.preferredHeight: 40
                        //     Layout.fillWidth: true
                        //     Button {
                        //         Layout.fillHeight: true
                        //         horizontalPadding: 12
                        //         background: Rectangle {
                        //             color: "white"
                        //             opacity: parent.pressed ? 0.16 : 0.1
                        //             radius: 4
                        //             visible: createHoverHandler.hovered && parent.enabled
                        //         }
                        //         HoverHandler {
                        //             id: createHoverHandler
                        //             cursorShape: Qt.PointingHandCursor
                        //         }
                        //         contentItem: RowLayout {
                        //             spacing: 8
                        //             FLIcon {
                        //                 id: createIcon
                        //                 icon: "undo"
                        //                 Layout.fillHeight: true
                        //                 size: height
                        //                 color: "white"
                        //             }
                        //             Text {
                        //                 Layout.fillHeight: true
                        //                 Layout.fillWidth: true
                        //                  text: "Undo last load"
                        //                  color: "white"
                        //                  font.family: Constants.regularFontFamily
                        //                  font.pixelSize: 16
                        //                  font.weight: Font.Medium
                        //                  verticalAlignment: Text.AlignVCenter
                        //                     horizontalAlignment: Text.AlignHCenter
                        //              }
                        //          }
                        //
                        //          // onClicked: {
                        //          //    emulatorLoader.item.writeSuspendPoint(suspendPointGrid.count)
                        //          // }
                        //     }
                        //     Item {
                        //         Layout.fillHeight: true
                        //         Layout.fillWidth: true
                        //     }
                        //
                        // }
                    }
                }

                populate: FLGridViewPopulateTransition {}
                add: FLGridViewAddTransition {}

                delegate: Pane {
                    id: theThing
                    required property var model
                    required property var index
                    width: GridView.view.cellWidth
                    height: GridView.view.cellHeight
                    padding: 8
                    background: Item {}

                    property bool showGlobalCursor: true
                    property var globalCursorProxy: contentItem

                    contentItem: FocusScope {
                        FLSuspendPointCard {
                            anchors.fill: parent
                            visible: model.has_data
                            imageUrl: theThing.model.image_url
                            dateTimeString: theThing.model.timestamp
                            index: theThing.index

                            onDeleteClicked: function() {
                                deleteSuspendPointDialog.index = index
                                deleteSuspendPointDialog.open()
                            }

                            onOverwriteClicked: function() {
                                overwriteSuspendPointDialog.index = index
                                overwriteSuspendPointDialog.open()
                            }
                        }
                        Button {
                            anchors.fill: parent
                            visible: !model.has_data
                            HoverHandler {
                                id: theHoverHandler
                                cursorShape: Qt.PointingHandCursor
                            }
                            background: Rectangle {
                                color: "white"
                                opacity: parent.pressed ? 0.08 : theHoverHandler.hovered ? 0.24 : 0.16
                                border.color: "#737373"
                                radius: 4
                            }
                            contentItem: Text {
                                color: "white"
                                text: "Create in slot " + (theThing.index + 1)
                                font.pixelSize: 15
                                font.family: Constants.regularFontFamily
                                font.weight: Font.DemiBold
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            onClicked: {
                                console.log("clicked create suspend point")
                                emulatorLoader.item.writeSuspendPoint(theThing.index)
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: rewindMenu
        RewindMenu {
            onRewindPointSelected: function(point) {
                emulatorLoader.item.loadRewindPoint(point)
                root.currentItem.close()
                // root.popCurrentItem(StackView.Immediate)
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
                         onClicked: {
                             resetGameDialog.open()
                         }
                     }
                     FirelightMenuItem {
                         id: rewindButton
                         labelText: "Rewind"
                         KeyNavigation.down: suspendPointButton
                         Layout.fillWidth: true
                         // Layout.preferredWidth: parent.width / 2
                         Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                         Layout.preferredHeight: 40
                         checkable: false
                         enabled: !achievement_manager.loggedIn || !achievement_manager.inHardcoreMode
                         alignRight: true
                         onClicked: {
                             emulatorLoader.item.createRewindPoints()
                         }
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
                         id: suspendPointButton
                         labelText: "Suspend Points"
                         Layout.fillWidth: true
                         KeyNavigation.down: closeGameButton
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
                     // Rectangle {
                     //     Layout.fillWidth: true
                     //     // Layout.preferredWidth: parent.width / 2
                     //     Layout.preferredHeight: 1
                     //     Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                     //     opacity: 0.3
                     //     color: "#dadada"
                     // }
                     // FirelightMenuItem {
                     //     id: achievementsButton
                     //     labelText: "Achievements"
                     //     Layout.fillWidth: true
                     //     KeyNavigation.down: closeGameButton
                     //     // Layout.preferredWidth: parent.width / 2
                     //     Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                     //     Layout.preferredHeight: 40
                     //     checkable: false
                     //     alignRight: true
                     //     // enabled: false
                     //     onClicked: {
                     //         quickMenuStack.pushItem(achievementsView, {}, StackView.Immediate)
                     //     }
                     // }

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

    FirelightDialog {
        id: resetGameDialog
        text: "Are you sure you want to reset the game?"
        onAccepted: {
            emulatorLoader.item.resetGame()
             root.popCurrentItem()
        }
    }

    Window {
        id: debugWindow
        // visible: emulatorLoader.active
        visible: false
        width: 400
        height: 300

        color: "black"

        Pane {
            id: bufferGraph
            anchors.fill: parent

            property var bufferValues: []

            function addValue(value) {
                bufferValues.push(value)
                if (bufferValues.length > 100) {
                    bufferValues.shift()
                }
                canvas.requestPaint()
                // bufferGraph.update()
                // canvas.paint()
            }


            padding: 12
            background: Rectangle {
                color: "black"
            }

            contentItem: Canvas {
                id: canvas
                width: parent.width
                height: parent.height
                onPaint: {
                    var ctx = getContext("2d");
                    ctx.clearRect(0, 0, width, height);
                    ctx.fillStyle = "white";
                    for (var i = 0; i < bufferGraph.bufferValues.length; i++) {
                        var x = (i / bufferGraph.bufferValues.length) * width;
                        var y = height - (bufferGraph.bufferValues[i] * height);
                        ctx.fillRect(x, y, width / bufferGraph.bufferValues.length, height);
                    }
                    ctx.fillStyle = "green";
                    ctx.fillRect(0, height / 2, width, 1); // Draw a middle line
                }

            }

            Text {
                id: buffer
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                text: emulator.audioBufferLevel
                color: "white"
                font.pixelSize: 14
                font.family: Constants.regularFontFamily
            }

        }

    }

    // FirelightDialog {
    //     id: unsupportedEmulatorDialog
    //     text: "In order to use hardcore mode, please use the latest version of Firelight. \n\n If you continue, you will automatically be switched to softcore mode."
    //
    //     acceptText: "Continue"
    //     rejectText: "Close game"
    //
    //     Connections {
    //         target: achievement_manager
    //
    //         function onUnsupportedEmulatorError() {
    //             unsupportedEmulatorDialog.open()
    //         }
    //     }
    // }

}
