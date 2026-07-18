import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

BaseSettingItem {
    id: root
    property alias checked: theControl.checked

    control: Switch {
        id: theControl

        enabled: root.enabled
        focusPolicy: Qt.NoFocus
        implicitWidth: 44
        implicitHeight: 24

        indicator: Rectangle {
            implicitWidth: 44
            implicitHeight: 24
            radius: height / 2
            anchors.verticalCenter: parent.verticalCenter
            color: theControl.checked ? "#35c46b" : "#5a5751"

            Behavior on color {
                ColorAnimation {
                    duration: 160
                    easing.type: Easing.InOutQuad
                }
            }

            Rectangle {
                width: 18
                height: 18
                radius: height / 2
                x: theControl.checked ? parent.width - width - 3 : 3
                anchors.verticalCenter: parent.verticalCenter
                color: "#ffffff"

                Behavior on x {
                    NumberAnimation {
                        duration: 160
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        }

        HoverHandler {
            cursorShape: Qt.PointingHandCursor
        }
    }
}
