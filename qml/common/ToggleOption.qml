import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Option {
    id: root

    property bool checked: false

    control: Switch {
        id: theControl
        checked: root.checked
        onCheckedChanged: {
            root.checked = checked
        }

        indicator: Rectangle {
            implicitWidth: 50
            implicitHeight: 28
            x: theControl.leftPadding
            y: parent.height / 2 - height / 2
            radius: height / 2
            color: theControl.checked ? "#17a81a" : "#ffffff"
            border.color: theControl.checked ? "#17a81a" : "#cccccc"

            Behavior on color {
                ColorAnimation {
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
            }

            Rectangle {
                x: theControl.checked ? parent.width - width : 0
                y: (parent.height - height) / 2

                Behavior on x {
                    NumberAnimation {
                        duration: 200
                        easing.type: Easing.InOutQuad
                    }
                }

                width: 26
                height: 26
                radius: height / 2
                color: theControl.down ? "#cccccc" : "#ffffff"
                border.color: theControl.checked ? (theControl.down ? "#17a81a" : "#21be2b") : "#999999"
            }
        }

        HoverHandler {
            cursorShape: Qt.PointingHandCursor
        }
    }
}