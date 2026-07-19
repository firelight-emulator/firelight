import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// A precise number nudged up or down. `value` is the current number; emits
// `changed` with the new value, clamped to [from, to]. `suffix` is an optional
// unit (e.g. "s")
BaseSettingItem {
    id: root

    property real from: 0
    property real to: 100
    property real stepSize: 1
    property real value: 0
    property string suffix: ""

    signal changed(real value)

    function step(delta) {
        var next = Math.max(root.from, Math.min(root.to, root.value + delta));
        if (next !== root.value) {
            root.changed(next);
        }
    }

    control: Rectangle {
        implicitHeight: AppStyle.controlHeight
        implicitWidth: Math.round(128 * AppStyle.scale)
        radius: AppStyle.radiusMd
        color: Theme.surface
        border.width: 1
        border.color: Theme.border

        RowLayout {
            anchors.fill: parent
            spacing: 0

            Button {
                Layout.preferredWidth: AppStyle.controlHeight
                Layout.fillHeight: true
                enabled: root.enabled && root.value > root.from
                focusPolicy: Qt.NoFocus
                onClicked: root.step(-root.stepSize)
                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }
                background: Item {}
                contentItem: Icon {
                    name: "remove"
                    size: AppStyle.iconSizeSm
                    color: enabled ? Theme.textPrimary : Theme.textMuted
                }
            }

            Text {
                Layout.fillWidth: true
                text: root.value + (root.suffix !== "" ? " " + root.suffix : "")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color: Theme.textPrimary
                font.family: Constants.regularFontFamily
                font.pixelSize: AppStyle.fontSizeSmall
                font.weight: Font.DemiBold
            }

            Button {
                Layout.preferredWidth: AppStyle.controlHeight
                Layout.fillHeight: true
                enabled: root.enabled && root.value < root.to
                focusPolicy: Qt.NoFocus
                onClicked: root.step(root.stepSize)
                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }
                background: Item {}
                contentItem: Icon {
                    name: "add"
                    size: AppStyle.iconSizeSm
                    color: enabled ? Theme.textPrimary : Theme.textMuted
                }
            }
        }
    }
}
