// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FLButtonBase {
    id: control
    objectName: "MenuNavigationItem|" + control.label

    property string label: ""

    Layout.fillWidth: true
    implicitHeight: AppStyle.listRowHeight
    variant: "subtle"
    rounded: false

    FLFocus.showCursor: true
    FLFocus.focusSound: SoundEffects.menuItemFocus

    checkedColor: Theme.switch2Color

    checkable: false

    contentItem: Row {
        spacing: AppStyle.spacingMd
        Item {
            y: AppStyle.spacingMd / 2
            height: parent.height - AppStyle.spacingMd
            width: AppStyle.spacingXs

            Rectangle {
                anchors.fill: parent
                color: Theme.switch2Color
                visible: control.checked
            }
        }

        Text {
            color: control._fg
            text: control.label
            font.pixelSize: AppStyle.fontSizeMedium
            font.family: AppStyle.fontFamily
            font.weight: Font.DemiBold
            height: parent.height
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
        }
    }
}
