import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    id: root

    height: col.height

    property string label
    property string description

    property string currentValue
    required property var model

    ColumnLayout {
        id: col
        width: root.width
        // anchors.fill: parent

        Text {
            Layout.fillWidth: true
            text: root.label
            color: ColorPalette.neutral100
            font.pixelSize: 15
            Layout.alignment: Qt.AlignLeft
            font.family: Constants.regularFontFamily
            font.weight: Font.DemiBold
            verticalAlignment: Text.AlignVCenter
        }
        // Text {
        //     Layout.fillWidth: true
        //     // visible: root.description !== ""
        //     text: root.description
        //     font.pixelSize: 13
        //     Layout.alignment: Qt.AlignLeft
        //     font.family: Constants.regularFontFamily
        //     // font.weight: Font.
        //     wrapMode: Text.WordWrap
        //     color: ColorPalette.neutral300
        // }

        Repeater {
            model: root.model
            delegate: RadioButton {
                id: control

                required property var model
                required property var index

                Layout.fillWidth: true
                Layout.preferredHeight: 60
                // checked: true
                // Layout.fillWidth: true
                // Layout.preferredHeight: 60

                background: Rectangle {
                    radius: 8
                    color: ColorPalette.neutral800
                }

                indicator: Rectangle {
                    implicitWidth: 26
                    implicitHeight: 26
                    x: control.leftPadding
                    y: parent.height / 2 - height / 2
                    radius: 13
                    border.color: control.down ? "#17a81a" : "#21be2b"

                    Rectangle {
                        width: 14
                        height: 14
                        x: 6
                        y: 6
                        radius: 7
                        color: control.down ? "#17a81a" : "#21be2b"
                        visible: control.checked
                    }
                }

                contentItem: ColumnLayout {
                    Text {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        leftPadding: control.indicator.width + control.spacing
                        text: model.label
                        color: ColorPalette.neutral100
                        font.pixelSize: 15
                        Layout.alignment: Qt.AlignLeft
                        font.family: Constants.regularFontFamily
                        font.weight: Font.DemiBold
                        verticalAlignment: Text.AlignVCenter
                    }
                    Text {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        // visible: root.description !== ""
                        text: model.description
                        leftPadding: control.indicator.width + control.spacing
                        font.pixelSize: 13
                        Layout.alignment: Qt.AlignLeft
                        font.family: Constants.regularFontFamily
                        // font.weight: Font.
                        wrapMode: Text.WordWrap
                        color: ColorPalette.neutral300
                    }
                }
            }
        }
    }


}