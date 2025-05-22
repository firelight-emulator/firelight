import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts 1.0
import Firelight 1.0

StackView {
    id: root

    property alias running: emulatorLoader.active
    property GameSettings gameSettings: GameSettings {
        platformId: 14
        contentHash: "b1de7027824c434ce8de59782705f5c9"
    }

    TapHandler {

    }

    Rectangle {
        color: "black"
        anchors.fill: parent
        z: -1
    }

    property var entryId: 0
    property var contentHash: ""
    property var platformId: 0

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

    // Component {
    //     id: achievementsView
    //     AchievementSetView {
    //         contentHash: emulatorLoader.item.contentHash
    //     }
    // }

    Component {
        id: gameSettingsComponent

        GameSettings {
            id: theGameSettings
            platformId: root.platformId
            contentHash: root.contentHash
        }
    }

    Loader {
        id: gameSettingsLoader
        sourceComponent: gameSettingsComponent
        active: emulatorLoader.active

    }

    Component {
        id: emuPage
        NewEmulatorPage {
            id: emuPage
            gameSettings: gameSettingsLoader.item
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
            opacity: emulatorLoader.blurAmount * 0.55
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

        onLoaded: function () {
            const emu = (emulatorLoader.item as NewEmulatorPage)

            emu.closing.connect(function() {
                emulatorLoader.active = false
                root.unloaded()
            })

            // overlay.opacity = 0

            emu.loadGame(emulatorLoader.entryId)
            emu.forceActiveFocus()

            emu.contentHashChanged.connect(function() {
                root.contentHash = emu.contentHash
            })

            emu.platformIdChanged.connect(function() {
                root.platformId = emu.platformId
            })

            // theGameSettings.contentHash = emu.contentHash
            // theGameSettings.platformId = emu.platformId
            // emulatorLoader.item.startGame()
            // emulatorLoader.item.paused = emulatorLoader.paused
            //
            // emulatorLoader.gameName = emulatorLoader.item.gameName
            // emulatorLoader.contentHash = emulatorLoader.item.contentHash
            // emulatorLoader.item.startGame(gameData, saveData, corePath, contentHash, saveSlotNumber, platformId, contentPath)
            // emulatorLoader.item.paused = emulatorLoader.paused
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

}
