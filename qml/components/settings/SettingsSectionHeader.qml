import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

Pane {
    id: root
    required property string sectionName
    property bool showTopPadding: true

    topPadding: root.showTopPadding ? 32 : 0
    bottomPadding: 12

    leftPadding: 0
    rightPadding: 0

    background: Item{}
    contentItem: RowLayout {
        spacing: 12
        Rectangle {
            width: 4
            implicitHeight: 24
            color: Theme.textMuted
        }
        Text {
            Layout.preferredHeight: 23
            font.pixelSize: AppStyle.fontSizeMedium
            font.family: Constants.regularFontFamily
            font.weight: Font.DemiBold
            color: Theme.textPrimary
            verticalAlignment: Text.AlignVCenter
            text: root.sectionName
        }
    }
}

