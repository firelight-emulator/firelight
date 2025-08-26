import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Button {
    id: root

    property string label
    property string description: ""
    property bool showGlobalCursor: true
    checkable: true
    horizontalPadding: 12
    verticalPadding: 12

    // onClicked: {
    //     if (root.checked) {
    //         sfx_player.play("switchon")
    //     } else {
    //         sfx_player.play("switchoff")
    //     }
    // }

    // implicitHeight: Math.max(72, theColumn.)
    hoverEnabled: true

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

        Switch {
            id: theControl
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

            enabled: root.enabled

            checked: root.checked

            onCheckedChanged: {
                root.checked = theControl.checked
            }

            indicator: Rectangle {
                implicitWidth: 50
                implicitHeight: 28
                x: theControl.leftPadding
                y: parent.height / 2 - height / 2
                radius: height / 2
                color: theControl.checked ? "#17a81a" : "#ffffff"
                border.color: theControl.checked ? "#17a81a" : "#cccccc"

                Behavior on color {
                    ColorAnimation {
                        duration: 200
                        easing.type: Easing.InOutQuad
                    }
                }

                Rectangle {
                    x: theControl.checked ? parent.width - width : 0
                    y: (parent.height - height) / 2

                    Behavior on x {
                        NumberAnimation {
                            duration: 200
                            easing.type: Easing.InOutQuad
                        }
                    }

                    width: 26
                    height: 26
                    radius: height / 2
                    color: theControl.down ? "#cccccc" : "#ffffff"
                    border.color: theControl.checked ? (theControl.down ? "#17a81a" : "#21be2b") : "#999999"
                }
            }

            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
        }
    }
}