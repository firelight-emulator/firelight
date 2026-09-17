// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Layouts

RowLayout {
    id: root

    property string label: ""
    property string value: ""
    property color valueColor: Theme.textPrimary

    objectName: "MonitorStatRow|" + label
    Layout.fillWidth: true
    spacing: AppStyle.spacingLg

    Text {
        text: root.label
        color: Theme.textMuted
        font.family: AppStyle.monoFontFamily
        font.pixelSize: AppStyle.fontSizeXSmall
    }

    Text {
        Layout.fillWidth: true
        horizontalAlignment: Text.AlignRight
        text: root.value
        color: root.valueColor
        font.family: AppStyle.monoFontFamily
        font.pixelSize: AppStyle.fontSizeXSmall
        font.features: ({
                "tnum": 1
            })
    }
}
