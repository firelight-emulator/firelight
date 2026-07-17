import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

BaseSettingItem {
    id: root

    controlBelow: false

    property real from: 0
    property real to: 10
    property real stepSize: 1
    property alias value: theControl.value

    signal moved(real value)

    control: RowLayout {
        spacing: 12

        Slider {
            id: theControl
            Layout.fillWidth: true
            enabled: root.enabled
            focusPolicy: Qt.NoFocus
            from: root.from
            to: root.to
            stepSize: root.stepSize
            snapMode: Slider.SnapAlways

            palette.midlight: Theme.textPrimary
            palette.dark: Theme.accent

            onMoved: root.moved(value)

            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
        }

        Text {
            Layout.preferredWidth: 40
            text: theControl.value.toFixed(2)
            color: Theme.textPrimary
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: AppStyle.fontSizeMedium
            font.family: Constants.regularFontFamily
            font.weight: Font.DemiBold
        }
    }
}
