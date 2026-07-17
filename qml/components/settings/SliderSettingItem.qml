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
        spacing: AppStyle.spacingMd

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
            // Fractional sliders (e.g. tint at step 0.01) need decimals; integer
            // sliders (volume, tile size) shouldn't read "100.00".
            Layout.preferredWidth: Math.round(52 * AppStyle.scale)
            text: theControl.value.toFixed(root.stepSize < 1 ? 2 : 0)
            color: Theme.textPrimary
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: AppStyle.fontSizeMedium
            font.family: Constants.regularFontFamily
            font.weight: Font.DemiBold
        }
    }
}
