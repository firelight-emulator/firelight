import QtQuick
import Firelight 1.0

// TODO
// Icon-only button; square, sized to the button height. Subtle by default
//
//   FLIconButton { iconName: "settings"; tooltipText: "Settings" }
//   FLIconButton { iconName: "grid_view"; checkable: true }
FLButtonBase {
    id: control

    objectName: "FLIconButton|" + iconName

    property string iconName: ""
    property color iconColor: control._fg
    FLFocus.focusSound: SoundEffects.iconButtonFocus

    variant: "subtle"
    implicitWidth: implicitHeight

    contentItem: Icon {
        anchors.centerIn: parent
        name: control.iconName
        size: AppStyle.iconSizeButton
        color: control.iconColor
    }
}
