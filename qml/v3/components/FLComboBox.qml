import QtQuick
import Firelight 1.0
import QtQuick.Controls

ComboBox {
    id: control

    objectName: "FLComboBox"

    font.pixelSize: AppStyle.fontSizeMedium
    font.family: AppStyle.fontFamily

    implicitContentWidthPolicy: ComboBox.WidestText
    implicitWidth: Math.max(AppStyle.inputWidth, implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(AppStyle.minTarget, AppStyle.controlHeight)
    leftPadding: AppStyle.spacingMd
    rightPadding: AppStyle.spacingMd

    background: Item {}

    contentItem: Text {
        text: control.displayText
        color: Theme.textMuted
        font: control.font
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignRight
        elide: Text.ElideRight
    }

    indicator: Item {}

    popup: FLRadioMenu {
        x: -(width - control.width)
        model: control.model

        textRole: control.textRole
        valueRole: control.valueRole

        currentValue: control.currentValue

        onActivated: value => {
            control.currentValue = value;
        }
    }
}
