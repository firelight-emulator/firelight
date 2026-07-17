import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// Search input: leading magnifier, placeholder, trailing clear. size sm
// (toolbar) | lg (page header). Pill-shaped; scales with the UI.
FocusScope {
    id: root

    property string placeholder: qsTr("Search")
    property alias text: field.text
    // sm | lg
    property string size: "sm"
    signal accepted()

    readonly property int _font: size === "lg" ? AppStyle.fontSizeLarge : AppStyle.fontSizeMedium
    readonly property int _icon: size === "lg" ? AppStyle.iconSizeMd : AppStyle.iconSizeSm

    implicitWidth: Math.round(240 * AppStyle.scale)
    implicitHeight: Math.max(AppStyle.minTarget, row.implicitHeight + AppStyle.spacingSm * 2)

    function clear() { field.text = ""; field.forceActiveFocus(); }

    Rectangle {
        anchors.fill: parent
        radius: height / 2
        color: field.activeFocus ? Theme.surfaceHover : Theme.surfaceElevated
        border.width: field.activeFocus ? 2 : 1
        border.color: field.activeFocus ? Theme.accent : Theme.border
    }

    RowLayout {
        id: row
        anchors.fill: parent
        anchors.leftMargin: AppStyle.spacingMd
        anchors.rightMargin: AppStyle.spacingSm
        spacing: AppStyle.spacingSm

        Icon {
            name: "search"
            size: root._icon
            color: Theme.textMuted
            Layout.alignment: Qt.AlignVCenter
        }
        TextField {
            id: field
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            placeholderText: root.placeholder
            color: Theme.textPrimary
            placeholderTextColor: Theme.textMuted
            font.pixelSize: root._font
            font.family: Constants.regularFontFamily
            selectByMouse: true
            background: Item {}
            onAccepted: root.accepted()
            Keys.onEscapePressed: field.text = ""
        }
        FLIconButton {
            visible: field.text !== ""
            iconName: "close"
            size: "sm"
            Layout.alignment: Qt.AlignVCenter
            onClicked: root.clear()
        }
    }
}
