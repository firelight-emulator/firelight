import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FocusScope {
    id: root

    required property string label
    required property Item control
    property string description: ""

    // Shown when this setting has an override at the current tier; emits reset()
    // so the caller can clear it (falling back to the inherited value).
    property bool resettable: false
    signal reset()

    property var onClicked: null

    implicitHeight: button.implicitHeight
    implicitWidth: button.implicitWidth

    Button {
        id: button
        anchors.fill: parent

        background: Item {}

        contentItem: ColumnLayout {
            spacing: 0
            // Rectangle {
            //     Layout.fillWidth: true
            //     Layout.maximumHeight: 1
            //     Layout.preferredHeight: 1
            //     color: ColorPalette.neutral300
            //     opacity: 0.25
            // }
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
                    opacity: parent.hovered || (!InputMethodManager.usingMouse && parent.activeFocus) ? 0.2 : 0.08

                    Behavior on opacity {
                        NumberAnimation {
                            duration: 100
                            easing.type: Easing.InOutQuad
                        }
                    }
                    radius: 8
                }

                contentItem: RowLayout {
                    height: 48
                    Text {
                        id: labelText
                        Layout.fillWidth: true
                        text: root.label
                        color: Theme.textPrimary
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: AppStyle.fontSizeMedium
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        font.family: Constants.regularFontFamily
                        font.weight: Font.DemiBold
                    }
                    Button {
                        id: resetButton
                        visible: root.resettable
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        Layout.rightMargin: 8
                        padding: 6
                        hoverEnabled: true
                        focusPolicy: Qt.NoFocus

                        onClicked: root.reset()

                        HoverHandler {
                            cursorShape: Qt.PointingHandCursor
                        }

                        background: Rectangle {
                            radius: 6
                            color: resetButton.hovered ? ColorPalette.neutral300 : "transparent"
                            opacity: resetButton.hovered ? 0.2 : 0
                        }

                        contentItem: Text {
                            text: qsTr("Reset")
                            color: Theme.textMuted
                            font.pixelSize: AppStyle.fontSizeSmall
                            font.family: Constants.regularFontFamily
                            font.weight: Font.Medium
                            verticalAlignment: Text.AlignVCenter
                        }
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

            // Rectangle {
            //     Layout.fillWidth: true
            //     Layout.maximumHeight: 1
            //     Layout.preferredHeight: 1
            //     color: ColorPalette.neutral300
            //     opacity: 0.25
            // }

            Text {
                id: descriptionText
                Layout.fillWidth: true
                Layout.topMargin: 4
                leftPadding: 8
                rightPadding: 8
                visible: root.description !== ""

                font.pixelSize: AppStyle.fontSizeMedium
                font.family: Constants.regularFontFamily
                font.weight: Font.Medium
                lineHeight: 1.2
                color: Theme.textMuted
                text: root.description
                wrapMode: Text.WordWrap
            }


        }
    }
}
