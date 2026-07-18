import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    id: root
    required property string label

    implicitHeight: theLabel.implicitHeight + thePane.implicitHeight
    implicitWidth: theLabel.implicitWidth + thePane.implicitWidth

    default property alias content: stuffCol.children

    Text {
        id: theLabel
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        text: root.label
        leftPadding: 12

        font.pixelSize: AppStyle.fontSizeMedium
        font.family: Constants.regularFontFamily
        font.weight: Font.Normal
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        color: Theme.textMuted
        Layout.bottomMargin: 4
    }

    Pane {
        id: thePane
        anchors.top: theLabel.bottom
        anchors.topMargin: 8
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        topPadding: stuffCol.spacing
        bottomPadding: topPadding * 3

        background: Rectangle {
            radius: 8
            color: Theme.surface
        }

        contentItem: ColumnLayout {
            id: stuffCol

        }
    }
}

