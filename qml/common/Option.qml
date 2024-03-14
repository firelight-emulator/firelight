import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FirelightStyle 1.0

Pane {
    id: root

    required property string label
    required property Component control

    background: Item {
    }

    padding: 8
    horizontalPadding: 10

    RowLayout {
        anchors.fill: parent

        Label {
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

            text: label
            font.pointSize: 12
            color: Constants.colorTestTextActive
            font.family: Constants.regularFontFamily
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }

        Loader {
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

            sourceComponent: control
        }


    }
}