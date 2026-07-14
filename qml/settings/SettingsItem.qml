import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: control
    required property string label
    property string description
    required property Component thing

    color: "transparent"

    RowLayout {
        anchors.fill: parent
        ColumnLayout {
            Layout.fillHeight: true
            Text {
                text: control.label
                color: Theme.textPrimary
                font.pixelSize: AppStyle.fontSizeMedium
                Layout.alignment: Qt.AlignLeft
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
            }
            Text {
                Layout.fillHeight: true
                visible: control.description !== ""
                text: control.description
                font.pixelSize: AppStyle.fontSizeMedium
                Layout.alignment: Qt.AlignLeft
                font.family: Constants.regularFontFamily
                wrapMode: Text.WordWrap
                color: Theme.textMuted
            }
        }
        Item {
            Layout.fillWidth: true
        }
        Loader {
            sourceComponent: thing
        }
    }
}