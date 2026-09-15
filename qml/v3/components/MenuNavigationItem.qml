import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FLButtonBase {
    id: control

    property string label: ""
    property bool activateOnFocus: true

    Layout.fillWidth: true
    Layout.preferredHeight: 50
    variant: "subtle"
    rounded: false

    FLFocus.showCursor: true
    FLFocus.focusSound: SoundEffects.menuItemFocus

    checkedColor: Theme.switch2Color

    checkable: false

    signal activated

    onActiveFocusChanged: {
        if (activeFocus && control.activateOnFocus) {
            activated()
        }
    }

    onClicked: {
        activated()
    }

    contentItem: Row {
        spacing: 12
        Item {
            y: AppStyle.spacingMd / 2
            height: parent.height - AppStyle.spacingMd
            width: 4

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