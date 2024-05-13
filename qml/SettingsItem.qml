import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FirelightStyle 1.0

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
                color: "white"
                font.pointSize: 12
                Layout.alignment: Qt.AlignLeft
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
            }
            Text {
                Layout.fillHeight: true
                visible: control.description !== ""
                text: control.description
                font.pointSize: 11
                Layout.alignment: Qt.AlignLeft
                font.family: Constants.regularFontFamily
                wrapMode: Text.WordWrap
                color: "#c1c1c1"
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