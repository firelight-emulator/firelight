import QtQuick

Item {
    id: root

    required property string text
    required property string sortRole
    property bool sortAscending: false

    property bool selected: false

    signal tapped

    Text {
        id: headerText
        height: parent.height
        text: root.text
        font.pixelSize: AppStyle.fontSizeMedium
        font.weight: root.selected ? Font.Bold : Font.DemiBold
        font.family: AppStyle.fontFamily
        color: root.selected ? Theme.textPrimary : Theme.textMuted
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        TapHandler {
            onTapped: root.tapped()
        }
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.NoButton
            cursorShape: Qt.PointingHandCursor
        }
    }
    Icon {
        anchors.left: headerText.right
        anchors.top: headerText.top
        anchors.leftMargin: AppStyle.spacingXs
        anchors.topMargin: 2
        anchors.bottom: headerText.bottom
        visible: root.selected
        name: root.sortAscending ? "arrow-down" : "arrow-up"
        size: AppStyle.iconSizeMd
        color: Theme.textPrimary
    }
}
