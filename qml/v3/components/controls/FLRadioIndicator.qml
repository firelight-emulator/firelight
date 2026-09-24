import QtQuick

Item {
    id: control

    property bool selected: false
    property color activeColor: Theme.switch2Color // TODO
    property color inactiveColor: Theme.border

    implicitWidth: AppStyle.iconSizeMd
    implicitHeight: AppStyle.iconSizeMd

    Rectangle {
        anchors.fill: parent
        radius: width / 2
        color: control.selected ? control.activeColor : "transparent"
        border.width: Math.max(1, Math.round(1 * AppStyle.scale))
        border.color: control.selected ? control.activeColor : control.inactiveColor
    }

    Rectangle {
        anchors.centerIn: parent
        width: parent.width * 0.4
        height: parent.height * 0.4
        radius: width / 2
        color: control.selected ? "white" : "transparent"
    }
}
