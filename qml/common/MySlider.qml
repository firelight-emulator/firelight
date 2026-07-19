import QtQuick
import QtQuick.Controls

Slider {
    id: control

    property bool intervalMarkers: true
    property int intervalMarkerWidth: 2
    property int intervalMarkerHeight: 24

    property int numIntervals: (to - from) / stepSize
    property bool valuesAsPercent: false

    property int widthDiff: width - availableWidth
    property int heightDiff: height - availableHeight

    implicitHeight: 60

    handle: Rectangle {
        color: Theme.textPrimary
        width: 10
        height: 24
        radius: 2
        x: control.availableWidth * control.visualPosition - (width / 2) + control.widthDiff / 2
        y: 4
        z: 4
    }

    background: Item {
        Item {
            id: bar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 32
            Rectangle {
                height: 8
                width: control.availableWidth
                x: control.widthDiff / 2
                y: parent.height / 2 - (height / 2)
                color: Theme.border

                Repeater {
                    model: control.numIntervals + 1
                    delegate: Rectangle {
                        required property var index
                        // required property var model

                        height: control.intervalMarkerHeight
                        color: Theme.border
                        width: control.intervalMarkerWidth
                        x: (index) * (control.availableWidth / control.numIntervals) - (width / 2)
                        y: -height / 2 + (parent.height / 2)
                    }
                }

                Rectangle {
                    color: Theme.accent
                    height: parent.height
                    width: control.availableWidth * control.visualPosition
                }
            }
        }

        Item {
            anchors.top: bar.bottom
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.bottom: parent.bottom

            Repeater {
                model: control.numIntervals + 1
                delegate: Text {
                    required property var index
                    property int val: control.from + (index * control.stepSize)
                    text: (val) + "%"
                    font.pixelSize: AppStyle.fontSizeSmall
                    font.weight: Font.Medium
                    color: ColorPalette.neutral400

                    x: (control.widthDiff / 2) + (index) * (control.availableWidth / control.numIntervals) - (width / 2)
                }
            }
        }
    }
}
