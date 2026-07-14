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
        Component.onCompleted: tas.loadDemo(240)
    }

    // Keep the playhead in view as playback/seek moves it.
    Connections {
        target: tas
        function onPlayheadChanged() {
            grid.positionViewAtIndex(tas.currentFrame, ListView.Contain);
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

            Item { Layout.fillWidth: true }

            Button { text: "◀◀"; onClicked: tas.seekTo(0) }
            Button { text: "◀"; onClicked: tas.stepBackward() }
            Button {
                text: tas.playing ? "‖" : "▶"
                onClicked: tas.togglePlay()
            }
            Button { text: "▶"; onClicked: tas.stepForward() }
            Button { text: "Reload demo"; onClicked: tas.loadDemo(240) }

            Text {
                text: "frame " + tas.currentFrame + " / " + tas.frameCount
                      + "    rerecords " + tas.rerecordCount
                color: ColorPalette.fontColorTwo
                font.pixelSize: 14
                font.family: "monospace"
            }
        }

        // --- live-video placeholder (docked in-app; deferred here) ---
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 180
            color: ColorPalette.neutral900
            border.color: ColorPalette.neutral700
            radius: 4
            Text {
                anchors.centerIn: parent
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
