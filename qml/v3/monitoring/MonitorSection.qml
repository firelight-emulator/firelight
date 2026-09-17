// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Layouts

ColumnLayout {
    id: root

    property string title: ""
    default property alias content: rows.data

    objectName: "MonitorSection|" + title
    Layout.fillWidth: true
    spacing: AppStyle.spacingXs

    Text {
        Layout.topMargin: AppStyle.spacingSm
        text: root.title
        color: Theme.textMuted
        font.family: AppStyle.monoFontFamily
        font.pixelSize: AppStyle.fontSizeXSmall
        font.bold: true
    }

    ColumnLayout {
        id: rows

        Layout.fillWidth: true
        spacing: AppStyle.spacingXs
    }
}
