import QtQuick

// TODO
// The ring-and-dot of a radio option, as a standalone indicator so rows and
// other surfaces can show selection without rebuilding it
Item {
    id: control

    property bool selected: false
    property color activeColor: "#1bcbfd"
    property color inactiveColor: Theme.border

    implicitWidth: AppStyle.iconSizeMd
    implicitHeight: AppStyle.iconSizeMd

    Rectangle {
        anchors.fill: parent
        radius: width / 2
        color: control.selected ? control.activeColor : "transparent"
        border.width: Math.max(1, Math.round(1 * AppStyle.scale))
        border.color: control.selected ? control.activeColor : control.inactiveColor

        // Behavior on border.color {
        //     ColorAnimation {
        //         duration: AppStyle.durationSnap
        //     }
        // }
    }

    Rectangle {
        anchors.centerIn: parent
        width: parent.width * 0.4
        height: parent.height * 0.4
        radius: width / 2
        color: control.selected ? "white" : "transparent"

        // Behavior on color {
        //     ColorAnimation {
        //         duration: AppStyle.durationSnap
        //     }
        // }
    }
}
