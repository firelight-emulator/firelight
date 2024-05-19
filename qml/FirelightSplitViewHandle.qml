import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


Item {
    id: handle
    implicitWidth: 9
    implicitHeight: parent.height

    Rectangle {
        anchors.centerIn: parent
        implicitWidth: 1
        implicitHeight: parent.height - 10
        color: handle.SplitHandle.pressed ? "#f1f1f1"
            : (handle.SplitHandle.hovered ? "#727272" : "transparent")

        Behavior on color {
            ColorAnimation {
                duration: 150
                easing.type: Easing.InOutQuad
            }

        }
    }
}