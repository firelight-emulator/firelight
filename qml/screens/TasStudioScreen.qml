import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// TAS Studio (Phase 1 scaffold): the editable piano-roll over a TasStudioController.
// All transport + editing routes through the C++ proxy (unit-tested); this file is the
// presentation layer. The video pane is a placeholder — docking the live EmulatorItem
// and a held-buttons overlay is the remaining GUI wiring
// (see libs/firelight/tas/README.md). Loads a self-contained demo movie so the grid has
// content without a running game.
Rectangle {
    id: root
    color: ColorPalette.neutral1000

    // When set (by Main3 while a game is running), the studio drives that live game
    // instead of the self-contained demo movie.
    property var liveEmulator: null

    // Swallow keyboard input so it drives ONLY the game (which reads keys via the
    // app's global window-level handler and records them), not the studio's own
    // controls. The transport/grid are operated with the mouse. Grabbing focus here
    // keeps key events from reaching the studio's focusable children.
    focus: true
    Component.onCompleted: root.forceActiveFocus()
    Keys.onPressed: function (event) { event.accepted = true }
    Keys.onReleased: function (event) { event.accepted = true }

    // The Game Boy button columns shown in the grid (id = RETRO_DEVICE_ID_JOYPAD_*).
    readonly property var buttonCols: [
        { id: 8, label: "A" },
        { id: 0, label: "B" },
        { id: 3, label: "St" },
        { id: 2, label: "Se" },
        { id: 4, label: "↑" },
        { id: 5, label: "↓" },
        { id: 6, label: "←" },
        { id: 7, label: "→" }
    ]
    readonly property int frameColWidth: 64
    readonly property int cellWidth: 34
    readonly property int rowHeight: 22

    TasStudioController {
        id: tas
        Component.onCompleted: {
            if (root.liveEmulator) {
                tas.bindLiveEmulator(root.liveEmulator) // drive the running game
            } else {
                tas.loadDemo(240) // self-contained preview
            }
        }
        Component.onDestruction: {
            if (root.liveEmulator) {
                tas.bindLiveEmulator(null) // release TAS control, resume play
            }
        }
    }

    // Keep the playhead in view as playback/seek moves it.
    Connections {
        target: tas
        function onPlayheadChanged() {
            // Follow the playhead / latest recorded frame; clamp to the committed
            // row count (during recording currentFrame runs ahead of it).
            var idx = Math.min(tas.currentFrame, grid.count - 1);
            if (idx >= 0)
                grid.positionViewAtIndex(idx, ListView.Contain);
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // --- header: title, transport, counters ---
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Text {
                text: "TAS Studio"
                color: ColorPalette.fontColorOne
                font.pixelSize: 22
                font.bold: true
            }

            Text {
                visible: tas.liveMode
                text: "● LIVE"
                color: ColorPalette.red500
                font.pixelSize: 13
                font.bold: true
                Layout.leftMargin: 4
            }

            Item { Layout.fillWidth: true }

            // focusPolicy NoFocus: mouse-operated only, so clicking never steals
            // keyboard focus from the root (which swallows game keys).
            Button { focusPolicy: Qt.NoFocus; text: "◀◀"; onClicked: tas.seekTo(0) }
            Button { focusPolicy: Qt.NoFocus; text: "◀"; onClicked: tas.stepBackward() }
            Button {
                focusPolicy: Qt.NoFocus
                text: tas.playing ? "‖" : "▶"
                onClicked: tas.togglePlay()
            }
            Button { focusPolicy: Qt.NoFocus; text: "▶"; onClicked: tas.stepForward() }
            Button {
                focusPolicy: Qt.NoFocus
                visible: tas.liveMode && !tas.replaying
                text: tas.recording ? "■ Stop" : "● Record"
                onClicked: tas.recording ? tas.stopRecording() : tas.startRecording()
            }
            Button {
                focusPolicy: Qt.NoFocus
                // Play the recorded movie back; hidden while recording or before
                // anything is recorded.
                visible: tas.liveMode && !tas.recording && tas.frameCount > 0
                text: tas.replaying ? "■ Stop" : "▶ Replay"
                onClicked: tas.replaying ? tas.stopReplay() : tas.startReplay()
            }
            Button {
                focusPolicy: Qt.NoFocus
                text: "Reload demo"
                visible: !tas.liveMode
                onClicked: tas.loadDemo(240)
            }

            Text {
                text: "frame " + tas.currentFrame + " / " + tas.frameCount
                      + "    rerecords " + tas.rerecordCount
                color: ColorPalette.fontColorTwo
                font.pixelSize: 14
                font.family: "monospace"
            }
        }

        // --- live video: a mirror of the running EmulatorItem's texture (no
        //     reparenting — the game keeps rendering on its own page; we display a
        //     live copy here), fit to the game's aspect ratio. ---
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: tas.liveMode ? 340 : 180
            color: ColorPalette.neutral900
            border.color: ColorPalette.neutral700
            radius: 4
            clip: true

            ShaderEffectSource {
                id: videoMirror
                visible: tas.liveMode && root.liveEmulator !== null
                sourceItem: tas.liveMode ? root.liveEmulator : null
                live: true          // keep re-rendering even while the source is occluded
                hideSource: false   // leave the game visible on its own page
                anchors.centerIn: parent
                property real ar: (root.liveEmulator && root.liveEmulator.trueAspectRatio > 0)
                                  ? root.liveEmulator.trueAspectRatio : (160.0 / 144.0)
                width: Math.min(parent.width - 8, (parent.height - 8) * ar)
                height: width / ar
            }

            Text {
                anchors.centerIn: parent
                visible: !videoMirror.visible
                text: "Live video docks here in-app"
                color: ColorPalette.neutral500
                font.pixelSize: 14
            }
        }

        // --- column header ---
        Row {
            Layout.fillWidth: true
            Rectangle {
                width: root.frameColWidth
                height: root.rowHeight
                color: "transparent"
                Text {
                    anchors.centerIn: parent
                    text: "frame"
                    color: ColorPalette.neutral500
                    font.pixelSize: 11
                }
            }
            Repeater {
                model: root.buttonCols
                delegate: Rectangle {
                    required property var modelData
                    width: root.cellWidth
                    height: root.rowHeight
                    color: "transparent"
                    Text {
                        anchors.centerIn: parent
                        text: modelData.label
                        color: ColorPalette.neutral400
                        font.pixelSize: 11
                    }
                }
            }
        }

        // --- the virtualized piano-roll ---
        ListView {
            id: grid
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            focus: false                 // don't grab keyboard from the root
            keyNavigationEnabled: false   // arrow keys drive the game, not the list
            model: tas.model
            boundsBehavior: Flickable.StopAtBounds
            cacheBuffer: root.rowHeight * 24
            ScrollBar.vertical: ScrollBar {}

            delegate: Rectangle {
                id: frameRow
                required property int index
                required property int frameIndex
                required property int buttons
                required property bool isCurrent
                required property bool isKeyframe

                width: ListView.view.width
                height: root.rowHeight
                color: isCurrent ? ColorPalette.primary700
                                 : (index % 2 === 0 ? ColorPalette.neutral900
                                                    : ColorPalette.neutral1000)

                Row {
                    anchors.fill: parent

                    // frame number + greenzone-keyframe marker (also click-to-seek)
                    Rectangle {
                        width: root.frameColWidth
                        height: parent.height
                        color: "transparent"

                        Text {
                            anchors.left: parent.left
                            anchors.leftMargin: 6
                            anchors.verticalCenter: parent.verticalCenter
                            text: frameRow.frameIndex
                            color: frameRow.isCurrent ? ColorPalette.white
                                                      : ColorPalette.neutral400
                            font.pixelSize: 12
                            font.family: "monospace"
                        }
                        Rectangle {
                            visible: frameRow.isKeyframe
                            width: 6
                            height: 6
                            radius: 3
                            color: ColorPalette.accentColor
                            anchors.right: parent.right
                            anchors.rightMargin: 4
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: tas.seekTo(frameRow.frameIndex)
                        }
                    }

                    // one cell per button; click toggles it
                    Repeater {
                        model: root.buttonCols
                        delegate: Rectangle {
                            required property var modelData
                            width: root.cellWidth
                            height: frameRow.height
                            property bool pressed: (frameRow.buttons & (1 << modelData.id)) !== 0
                            color: pressed ? ColorPalette.primary400 : "transparent"
                            border.color: ColorPalette.neutral800
                            border.width: 1

                            MouseArea {
                                anchors.fill: parent
                                onClicked: tas.toggleInput(frameRow.frameIndex, modelData.id)
                            }
                        }
                    }
                }
            }
        }
    }
}
