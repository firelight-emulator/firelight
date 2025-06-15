import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FocusScope {
    id: root

    required property string label
    // required property Component control
    property string description: ""

    implicitHeight: button.implicitHeight + descriptionText.implicitHeight + 4
    implicitWidth: button.implicitWidth

    Button {
        id: button
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.left: parent.left
        height: 68
        padding: 12

        focus: true

        property bool showGlobalCursor: true

        background: Rectangle {
            color: ColorPalette.neutral300
            radius: 8
            border.color: ColorPalette.neutral500
            opacity: parent.hovered || (!InputMethodManager.usingMouse && root.activeFocus) ? 0.2 : 0.1

            Behavior on opacity {
                NumberAnimation {
                    duration: 100
                    easing.type: Easing.InOutQuad
                }
            }
        }

        contentItem: RowLayout {
            Text {
                id: labelText
                Layout.fillWidth: true
                // Layout.preferredHeight: 48
                text: root.label
                color: ColorPalette.neutral100
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 16
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
            }
        }
    }

    Text {
        id: descriptionText
        anchors.top: button.bottom
        anchors.topMargin: 4
        anchors.left: parent.left
        anchors.leftMargin: button.leftPadding
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
