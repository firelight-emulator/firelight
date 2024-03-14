import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FirelightStyle 1.0

Option {
    id: root
    control: Switch {
        id: control
        Layout.fillHeight: true
        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
        checked: true
        onCheckedChanged: {
            console.log("Switch checked: " + checked)
        }

        indicator: Rectangle {
            implicitWidth: 50
            implicitHeight: 28
            x: control.leftPadding
            y: parent.height / 2 - height / 2
            radius: height / 2
            color: control.checked ? "#17a81a" : "#ffffff"
            border.color: control.checked ? "#17a81a" : "#cccccc"

            Behavior on color {
                ColorAnimation {
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
            }

            Rectangle {
                x: control.checked ? parent.width - width : 0
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
                color: control.down ? "#cccccc" : "#ffffff"
                border.color: control.checked ? (control.down ? "#17a81a" : "#21be2b") : "#999999"
            }
        }

        HoverHandler {
            cursorShape: Qt.PointingHandCursor
        }
    }
}