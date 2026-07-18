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

        Icon {
            Layout.fillHeight: true
            visible: root.isSubItem
            leftPadding: 8
            rightPadding: 8
            name: "subdirectory_arrow_right"
            size: 24
            color: Theme.textMuted
        }

        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.horizontalStretchFactor: 3

            Text {
                Layout.fillWidth: true
                text: root.label
                color: Theme.textPrimary
                font.pixelSize: AppStyle.fontSizeMedium
                Layout.alignment: Qt.AlignLeft
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
            }
            Text {
                Layout.fillHeight: true
                Layout.fillWidth: true
                visible: root.description !== ""
                text: root.description
                font.pixelSize: AppStyle.fontSizeSmall
                Layout.alignment: Qt.AlignLeft
                font.family: Constants.regularFontFamily
                // font.weight: Font
                wrapMode: Text.WordWrap
                color: Theme.textMuted
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.horizontalStretchFactor: 1
        }

        Loader {
            // Layout.fillHeight: true
            Layout.alignment: Qt.AlignRight | Qt.AlignTop

            sourceComponent: control
        }

    }
}
