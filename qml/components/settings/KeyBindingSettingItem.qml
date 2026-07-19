import QtQuick

// Shows the current binding as a pill and requests a rebind when clicked. The
// owner sets `listening` while capturing and updates `binding` when done; this
// component is purely presentational
BaseSettingItem {
    id: root

    property string binding: ""
    property bool listening: false
    signal rebindRequested

    onClicked: function () {
        root.rebindRequested();
    }

    control: Rectangle {
        implicitHeight: 34
        implicitWidth: Math.max(72, keyText.implicitWidth + 24)
        radius: 6
        color: root.listening ? Theme.blend(Theme.surface, Theme.accent, 0.15) : Theme.surface
        border.width: 1
        border.color: root.listening ? Theme.accent : Theme.borderStrong

        Text {
            id: keyText
            anchors.centerIn: parent
            text: root.listening ? qsTr("Press a key…") : (root.binding === "" ? qsTr("Unbound") : root.binding)
            color: root.listening ? Theme.accent : (root.binding === "" ? Theme.textMuted : Theme.textPrimary)
            font.family: Constants.regularFontFamily
            font.pixelSize: AppStyle.fontSizeSmall
            font.weight: Font.DemiBold
        }
    }
}
