import QtQuick
import QtQuick.Controls
import Firelight 1.0

// Tab strip with an underline indicator. Feed `tabs` a list of labels; the
// selected index is `currentIndex`. Metrics from AppStyle, colors from Theme.
//
//   FLTabBar { tabs: ["Quick Menu", "Achievements", "Settings"] }
TabBar {
    id: control

    property var tabs: []
    spacing: AppStyle.spacingMd

    background: Item {}

    Repeater {
        model: control.tabs
        delegate: TabButton {
            id: tab
            required property int index
            required property string modelData
            width: implicitWidth
            implicitHeight: Math.max(AppStyle.minTarget, label.implicitHeight + AppStyle.spacingSm * 2)
            padding: AppStyle.spacingSm
            property bool showGlobalCursor: true

            HoverHandler { cursorShape: Qt.PointingHandCursor }

            contentItem: Text {
                id: label
                text: tab.modelData
                font.pixelSize: AppStyle.fontSizeMedium
                font.family: Constants.regularFontFamily
                font.weight: Font.Medium
                color: tab.checked ? Theme.textPrimary : Theme.textMuted
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                color: "transparent"
                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width
                    height: Math.max(2, Math.round(2 * AppStyle.scale))
                    radius: height / 2
                    color: Theme.accent
                    opacity: tab.checked ? 1 : 0
                    Behavior on opacity { NumberAnimation { duration: 100 } }
                }
            }
        }
    }
}
