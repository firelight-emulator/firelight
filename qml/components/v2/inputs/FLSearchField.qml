import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Search input: leading magnifier, placeholder, trailing clear. size sm
// (toolbar) | lg (page header). Pill-shaped; scales with the UI
FocusScope {
    id: root

    property string placeholder: qsTr("Search")
    property alias text: field.text
    // sm | lg
    property string size: "sm"
    signal accepted

    readonly property int _font: size === "lg" ? AppStyle.fontSizeLarge : AppStyle.fontSizeMedium
    readonly property int _icon: size === "lg" ? AppStyle.iconSizeMd : AppStyle.iconSizeSm

    implicitWidth: Math.round(280 * AppStyle.scale)
    implicitHeight: Math.max(AppStyle.minTarget, row.implicitHeight)

    function clear() {
        field.text = "";
        field.forceActiveFocus();
    }

    Rectangle {
        anchors.fill: parent
        radius: AppStyle.radiusMd
        // color: field.activeFocus ? Theme.surfaceHover : Theme.surfaceElevated
        color: Theme.backgroundInset
        // border.color: field.activeFocus ? Theme.accent : Theme.border
        border.color: Theme.border
    }

    RowLayout {
        id: row
        anchors.fill: parent
        anchors.leftMargin: AppStyle.spacingMd
        anchors.rightMargin: AppStyle.spacingSm
        spacing: AppStyle.spacingSm

        Icon {
            name: "search"
            size: AppStyle.iconSizeMd
            color: Theme.textMuted
            Layout.alignment: Qt.AlignVCenter
        }
        TextField {
            id: field
            padding: 0
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredHeight: AppStyle.controlHeight
            placeholderText: root.placeholder
            font.weight: Font.Normal
            color: Theme.textPrimary
            placeholderTextColor: Theme.textMuted
            font.pixelSize: root._font
            font.family: AppStyle.fontFamily
            selectByMouse: true
            background: Item {}
            onAccepted: root.accepted()
            Keys.onEscapePressed: field.text = ""
        }
        FLIconButton {
            visible: field.text !== ""
            iconName: "close"
            compact: true
            Layout.alignment: Qt.AlignVCenter
            onClicked: root.clear()
        }
    }
}
