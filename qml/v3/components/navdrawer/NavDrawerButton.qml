import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FLButtonBase {
    id: root

    implicitHeight: 50
    variant: "subtle"
    rounded: false

    // FLFocus.focusSound: SoundEffects.menuItemFocus

    checkedColor: Theme.switch2Color

    checkable: false

    contentItem: Row {
        Text {
            id: buttonLabel
            color: root._fg
            text: root.text
            font.pixelSize: AppStyle.fontSizeMedium
            font.family: AppStyle.fontFamily
            font.weight: Font.DemiBold
            height: parent.height
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
        }
    }
}