import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FocusScope {
    id: root

    required property string label
    required property Item control
    property string description: ""

    property var onClicked: null

    implicitHeight: button.implicitHeight
    implicitWidth: button.implicitWidth

    Button {
        id: button
        anchors.fill: parent

        background: Item {}

        contentItem: ColumnLayout {
            spacing: 0
            Rectangle {
                Layout.fillWidth: true
                Layout.maximumHeight: 1
                Layout.preferredHeight: 1
                color: ColorPalette.neutral300
                opacity: 0.25
            }
            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 60
                padding: 8

                focus: true

                property bool showGlobalCursor: true
                property var globalCursorSpacing: 1

                 HoverHandler {
                     cursorShape: Qt.PointingHandCursor
                 }


                onClicked: {
                    if (root.onClicked) {
                        root.onClicked()
                    }
                }

                background: Rectangle {
                    color: ColorPalette.neutral300
                    opacity: parent.hovered || (!InputMethodManager.usingMouse && parent.activeFocus) ? 0.2 : 0

                    Behavior on opacity {
                        NumberAnimation {
                            duration: 100
                            easing.type: Easing.InOutQuad
                        }
                    }
                }

                contentItem: RowLayout {
                    height: 48
                    Text {
                        id: labelText
                        Layout.fillWidth: true
                        text: root.label
                        color: ColorPalette.neutral100
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: 16
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        font.family: Constants.regularFontFamily
                        font.weight: Font.DemiBold
                    }
                    Pane {
                        id: controlItem
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenters
                        background: Item{}
                        padding: 0
                        contentItem: root.control
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.maximumHeight: 1
                Layout.preferredHeight: 1
                color: ColorPalette.neutral300
                opacity: 0.25
            }

            Text {
                id: descriptionText
                Layout.fillWidth: true
                Layout.topMargin: 4
                visible: root.description !== ""

                font.pixelSize: 15
                font.family: Constants.regularFontFamily
                font.weight: Font.Medium
                lineHeight: 1.2
                color: ColorPalette.neutral100
                text: root.description
                wrapMode: Text.WordWrap
            }


        }
    }
}
