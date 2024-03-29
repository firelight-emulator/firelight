import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FirelightStyle 1.0

Rectangle {
    id: control
    required property string label
    required property Component thing

    color: "transparent"

    RowLayout {
        anchors.fill: parent
        Text {
            text: control.label
            color: Constants.colorTestTextActive
            font.pointSize: 12
            Layout.alignment: Qt.AlignLeft
            font.family: Constants.regularFontFamily
            padding: 12
        }
        Item {
            Layout.fillWidth: true
        }
        Loader {
            sourceComponent: thing
        }
    }
}