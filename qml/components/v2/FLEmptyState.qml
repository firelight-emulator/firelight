import QtQuick
import QtQuick.Layouts
import Firelight 1.0

// Centered placeholder for an empty view: optional icon, title, subtitle
ColumnLayout {
    id: root

    property string iconName: ""
    property string title: ""
    property string subtitle: ""

    spacing: AppStyle.spacingMd

    Icon {
        visible: root.iconName !== ""
        Layout.alignment: Qt.AlignHCenter
        name: root.iconName
        size: AppStyle.iconSizeLg * 2
        color: Theme.textMuted
    }
    Text {
        visible: root.title !== ""
        Layout.alignment: Qt.AlignHCenter
        text: root.title
        color: Theme.textPrimary
        font.pixelSize: AppStyle.fontSizeLarge
        font.family: Constants.regularFontFamily
        font.weight: Font.DemiBold
        horizontalAlignment: Text.AlignHCenter
    }
    Text {
        visible: root.subtitle !== ""
        Layout.alignment: Qt.AlignHCenter
        Layout.maximumWidth: Math.round(360 * AppStyle.scale)
        text: root.subtitle
        color: Theme.textMuted
        font.pixelSize: AppStyle.fontSizeMedium
        font.family: Constants.regularFontFamily
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
    }
}
