import QtQuick

Item {
    id: control

    property bool checked: false
    property color activeColor: "#1bcbfd"
    property color inactiveColor: Theme.border

    implicitWidth: AppStyle.iconSizeMd
    implicitHeight: AppStyle.iconSizeMd

    Rectangle {
        anchors.fill: parent
        radius: AppStyle.radiusSm
        color: "transparent"
        border.width: Math.max(1, Math.round(1 * AppStyle.scale))
        border.color: control.selected ? control.activeColor : control.inactiveColor
    }

    Rectangle {
        anchors.fill: parent
        radius: AppStyle.radiusSm
        color: control.activeColor
        scale: control.checked ? 1 : 0

        Behavior on scale {
            NumberAnimation {
                duration: AppStyle.durationVeryFast
                easing: Easing.InOutQuad
            }
        }

        Icon {
            anchors.centerIn: parent
            name: "check"
            size: AppStyle.iconSizeMd
            color: Theme.textPrimary
        }
    }

}