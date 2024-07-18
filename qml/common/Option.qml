import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Pane {
    id: root

    required property string label
    property string description
    required property Component control

    background: Item {
    }

    verticalPadding: 8
    horizontalPadding: 0

    RowLayout {
        anchors.fill: parent

        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.horizontalStretchFactor: 3
            Text {
                Layout.fillWidth: true
                text: root.label
                color: "white"
                font.pointSize: 12
                Layout.alignment: Qt.AlignLeft
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
            }
            Text {
                Layout.fillHeight: true
                Layout.fillWidth: true
                visible: root.description !== ""
                text: root.description
                font.pointSize: 11
                Layout.alignment: Qt.AlignLeft
                font.family: Constants.regularFontFamily
                wrapMode: Text.WordWrap
                color: "#c1c1c1"
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.horizontalStretchFactor: 1
        }

        // Label {
        //     Layout.fillHeight: true
        //     Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
        //
        //     text: label
        //     font.pointSize: 12
        //     color: Constants.colorTestTextActive
        //     font.family: Constants.regularFontFamily
        //     horizontalAlignment: Text.AlignLeft
        //     verticalAlignment: Text.AlignVCenter
        // }

        Loader {
            // Layout.fillHeight: true
            Layout.alignment: Qt.AlignRight | Qt.AlignTop

            sourceComponent: control
        }


    }
}