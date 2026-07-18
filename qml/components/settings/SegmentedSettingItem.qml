import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// Pick one of a few options, all visible at once. `options` is [{label, value}];
// `currentValue` is the selected value; emits `changed` on selection. Sits to
// the side for 2 options, full-width below for 3+
BaseSettingItem {
    id: root

    property var options: []
    property string currentValue: ""
    signal changed(string value)

    controlBelow: (options ? options.length : 0) >= 5

    control: Rectangle {
        id: segBox
        implicitHeight: AppStyle.controlHeight
        implicitWidth: segRow.implicitWidth + 4
        radius: AppStyle.radiusMd
        color: Theme.surface
        border.width: 1
        border.color: Theme.border

        RowLayout {
            id: segRow
            anchors.fill: parent
            anchors.margins: 2
            spacing: 2

            Repeater {
                model: root.options
                delegate: Button {
                    id: seg
                    required property var modelData
                    readonly property bool on: root.currentValue === modelData.value

                    Layout.fillWidth: root.controlBelow
                    // Fills the box so segment height tracks the scaled box
                    Layout.fillHeight: true
                    implicitWidth: segLabel.implicitWidth + AppStyle.spacingMd * 2
                    focusPolicy: Qt.NoFocus
                    hoverEnabled: true
                    onClicked: root.changed(modelData.value)

                    HoverHandler {
                        cursorShape: Qt.PointingHandCursor
                    }

                    contentItem: Text {
                        id: segLabel
                        text: seg.modelData.label
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        color: seg.on ? Theme.onAccent : Theme.textMuted
                        font.family: Constants.regularFontFamily
                        font.pixelSize: AppStyle.fontSizeSmall
                        font.weight: Font.DemiBold
                    }

                    background: Rectangle {
                        radius: AppStyle.radiusSm
                        color: seg.on ? Theme.accent : (seg.hovered ? Theme.surfaceHover : "transparent")
                    }
                }
            }
        }
    }
}
