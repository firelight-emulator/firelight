import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Button {
    id: root

    property string label
    property string description: ""
    property bool showGlobalCursor: true
    property alias model: comboBox.model
    property alias currentIndex: comboBox.currentIndex
    property alias currentValue: comboBox.currentValue
    horizontalPadding: 12
    verticalPadding: 12

    // implicitHeight: Math.max(72, theColumn.)
    // hoverEnabled: true

    background: Rectangle {
        color: "white"
        radius: 2
        opacity: root.hovered ? 0.1 : 0
    }

    contentItem: RowLayout {
        ColumnLayout {
            id: theColumn
            Layout.fillHeight: true
            Layout.fillWidth: true
            spacing: 8

            Text {
                id: labelText
                Layout.fillWidth: true
                text: root.label
                color: ColorPalette.neutral100
                font.pixelSize: 16
                Layout.alignment: Qt.AlignLeft
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
            }
            Text {
                id: descriptionText
                Layout.fillHeight: true
                Layout.fillWidth: true
                visible: root.description !== ""
                text: root.description
                font.pixelSize: 15
                lineHeight: 1.2
                Layout.alignment: Qt.AlignLeft
                font.family: Constants.regularFontFamily
                // font.weight: Font.
                wrapMode: Text.WordWrap
                color: ColorPalette.neutral300
            }
        }

        MyComboBox {
            id: comboBox
            textRole: "text"
            valueRole: "value"

            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
        }
    }
}