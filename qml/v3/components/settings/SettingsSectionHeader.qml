import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Pane {
    id: root
    required property string sectionName
    property bool showTopPadding: true

    topPadding: root.showTopPadding ? 32 : 0
    bottomPadding: AppStyle.spacingMd

    leftPadding: 0
    rightPadding: 0

    background: Item {}
    contentItem: RowLayout {
        spacing: AppStyle.spacingMd
        Rectangle {
            width: 4
            implicitHeight: 24
            radius: 2
            color: Theme.accent
        }
        Text {
            Layout.preferredHeight: 23
            font.pixelSize: AppStyle.fontSizeMedium
            font.family: AppStyle.fontFamily
            font.weight: Font.DemiBold
            color: Theme.textPrimary
            verticalAlignment: Text.AlignVCenter
            text: root.sectionName
        }
    }
}
