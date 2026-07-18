import QtQuick
import Firelight 1.0

// A small status pill. tone: neutral | accent | success | warning | danger
Rectangle {
    id: root

    property string text: ""
    property string tone: "neutral"

    readonly property color _c: tone === "accent" ? Theme.accent : tone === "success" ? Theme.success : tone === "warning" ? Theme.warning : tone === "danger" ? Theme.danger : Theme.textMuted

    implicitWidth: label.implicitWidth + AppStyle.spacingMd
    implicitHeight: label.implicitHeight + AppStyle.spacingXs
    radius: height / 2
    color: Qt.rgba(_c.r, _c.g, _c.b, 0.18)

    Text {
        id: label
        anchors.centerIn: parent
        text: root.text
        color: root._c
        font.pixelSize: AppStyle.fontSizeSmall
        font.family: Constants.regularFontFamily
        font.weight: Font.DemiBold
    }
}
