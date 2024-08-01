import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Pane {
    id: root

    required property string label
    property string description
    // required property Component control
    property Component control

    property bool isSubItem: false

    background: Item {
    }

    verticalPadding: 8
    horizontalPadding: 0

    RowLayout {
        anchors.fill: parent

        Text {
            Layout.fillHeight: true
            visible: root.isSubItem
            leftPadding: 8
            rightPadding: 8
            text: "\ue5da"
            font.family: Constants.symbolFontFamily
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 24
            color: ColorPalette.neutral600
        }

        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.horizontalStretchFactor: 3

            Text {
                Layout.fillWidth: true
                text: root.label
                color: ColorPalette.neutral100
                font.pixelSize: 15
                Layout.alignment: Qt.AlignLeft
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
            }
            Text {
                Layout.fillHeight: true
                Layout.fillWidth: true
                visible: root.description !== ""
                text: root.description
                font.pixelSize: 13
                Layout.alignment: Qt.AlignLeft
                font.family: Constants.regularFontFamily
                // font.weight: Font.
                wrapMode: Text.WordWrap
                color: ColorPalette.neutral300
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