import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQml.Models
import QtQuick.Layouts 1.0
import QtNetwork
import QtMultimedia
import QtQuick.Effects
import Firelight 1.0

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
        font.family: Constants.regularFontFamily
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
    FLIcon {
        anchors.left: headerText.right
        anchors.top: headerText.top
        anchors.leftMargin: 4
        anchors.topMargin: 2
        anchors.bottom: headerText.bottom
        visible: root.selected
        icon: root.sortAscending ? "arrow-down" : "arrow-up"
        size: 22
        color: Theme.textPrimary
    }
}
