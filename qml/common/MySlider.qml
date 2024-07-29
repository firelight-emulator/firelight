import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

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
        color: ColorPalette.neutral100
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
                color: ColorPalette.neutral500

                // Rectangle {
                //     height: 24
                //     color: "grey"
                //     width: 2
                //     x: -(width / 2)
                //     y: -height / 2 + (parent.height / 2)
                // }

                Repeater {
                    model: control.numIntervals + 1
                    delegate: Rectangle {
                        required property var index
                        // required property var model

                        height: control.intervalMarkerHeight
                        color: ColorPalette.neutral500
                        width: control.intervalMarkerWidth
                        x: (index) * (control.availableWidth / control.numIntervals) - (width / 2)
                        y: -height / 2 + (parent.height / 2)
                    }
                }


                // Rectangle {
                //     height: 24
                //     color: "grey"
                //     width: 2
                //     x: control.availableWidth / 3 - (width / 2)
                //     y: -height / 2 + (parent.height / 2)
                // }

                Rectangle {
                    color: "lightblue"
                    height: parent.height
                    width: control.availableWidth * control.visualPosition
                }
            }
        }

        // Rectangle {
        //     height: 24
        //     color: "grey"
        //     width: 2
        //     x: control.widthDiff / 2 - (width / 2)
        //     y: -height / 2 + (parent.height / 2)
        // }

        Item {
            anchors.top: bar.bottom
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            // height: parent.height / 3
            // y: parent.height * 2 / 3
            // x: control.widthDiff / 2

            Repeater {
                model: control.numIntervals + 1
                delegate: Text {
                    required property var index
                    property int val: control.from + (index * control.stepSize)
                    text: (val) + "%"
                    font.pointSize: 10
                    font.weight: Font.Medium
                    color: ColorPalette.neutral400

                    x: (control.widthDiff / 2) + (index) * (control.availableWidth / control.numIntervals) - (width / 2)
                }

                //     Rectangle {
                //     required property var index
                //     // required property var model
                //
                //     height: control.intervalMarkerHeight
                //     color: "grey"
                //     width: control.intervalMarkerWidth
                //     x: (index + 1) * (control.availableWidth / control.numIntervals) - (width / 2)
                //     y: -height / 2 + (parent.height / 2)
                // }
            }
        }


        // Rectangle {
        //     height: 24
        //     color: "grey"
        //     width: 2
        //     x: control.availableWidth / 3 - (width / 2)
        //     y: -height / 2 + (parent.height / 2)
        // }

        // Item {
        //     width: parent.width
        //     height: parent.height / 3
        //     y: parent.height * 2 / 3
        //
        //     Text {
        //         text: "None"
        //         font.pointSize: 8
        //         color: "white"
        //         x: -(width / 2)
        //     }
        //
        //     Text {
        //         text: "Light"
        //         font.pointSize: 8
        //         color: "white"
        //         x: control.availableWidth / 3 - (width / 2)
        //     }
        //
        //     Text {
        //         text: "Compatible"
        //         font.pointSize: 8
        //         color: "white"
        //         x: control.availableWidth * 2 / 3 - (width / 2)
        //     }
        //
        //     Text {
        //         text: "Max"
        //         font.pointSize: 8
        //         color: "white"
        //         x: control.availableWidth - (width / 2)
        //     }
        // }
    }
}