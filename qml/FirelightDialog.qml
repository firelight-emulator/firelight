import QtQuick
import QtQuick.Controls
// import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

Dialog {
    id: control
    property string text
    property bool showButtons: true
    modal: true
    // parent: Overlay.overlay
    anchors.centerIn: Overlay.overlay
    padding: 24

    width: 520
    height: 240

    background: Rectangle {
        color: ColorPalette.neutral900
    }

    // header: Text {
    //     text: "Closing game"
    //     font.family: Constants.regularFontFamily
    //     font.pixelSize: 20
    //     padding: 24
    //     font.weight: Font.ExtraLight
    //     color: "white"
    //     horizontalAlignment: Text.AlignLeft
    //     verticalAlignment: Text.AlignVCenter
    // }

    contentItem: Text {
        // anchors.centerIn: parent
        color: "white"
        font.family: Constants.regularFontFamily
        text: control.text
        font.weight: Font.Light
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: 18
    }

    footer: FocusScope {
        height: 66
        focus: true
        RowLayout {
            spacing: 12
            visible: control.showButtons
            anchors.fill: parent

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            FirelightButton {
                id: cancelButton
                focus: true
                Layout.alignment: Qt.AlignHCenter | Qt.AlignTop

                KeyNavigation.right: acceptButton

                label: "Cancel"
                onClicked: control.reject()
            }

            FirelightButton {
                id: acceptButton
                Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                label: "Yes"
                onClicked: control.accept()
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }

    // footer: DialogButtonBox {
    //     focus: true
    //     spacing: 12
    //     visible: control.showButtons
    //     background: Rectangle {
    //         color: "transparent"
    //     }
    //     Button {
    //         anchors.verticalCenter: parent.verticalCenter
    //         width: 200
    //         height: 42
    //         property bool showGlobalCursor: true
    //         Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
    //
    //         background: Rectangle {
    //             color: parent.activeFocus ? ColorPalette.neutral100 : ColorPalette.neutral800
    //             radius: height / 2
    //         }
    //         DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
    //
    //         contentItem: Text {
    //             text: "Cancel"
    //             font.family: Constants.regularFontFamily
    //             font.pixelSize: 14
    //             // opacity: parent.checked ? 1 : 0.5
    //             horizontalAlignment: Text.AlignHCenter
    //             verticalAlignment: Text.AlignVCenter
    //             color: parent.activeFocus ? "black" : "white"
    //         }
    //
    //         HoverHandler {
    //             cursorShape: Qt.PointingHandCursor
    //         }
    //     }
    //
    //     Button {
    //         anchors.verticalCenter: parent.verticalCenter
    //         focus: true
    //         width: 200
    //         height: 42
    //         property bool showGlobalCursor: true
    //         Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
    //
    //         background: Rectangle {
    //             color: parent.activeFocus ? ColorPalette.neutral100 : ColorPalette.neutral800
    //             radius: height / 2
    //         }
    //         DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
    //
    //         contentItem: Text {
    //             text: "Yes"
    //             font.family: Constants.regularFontFamily
    //             font.pixelSize: 14
    //             // opacity: parent.checked ? 1 : 0.5
    //             horizontalAlignment: Text.AlignHCenter
    //             verticalAlignment: Text.AlignVCenter
    //             color: parent.activeFocus ? "black" : "white"
    //         }
    //
    //         HoverHandler {
    //             cursorShape: Qt.PointingHandCursor
    //         }
    //     }
    // }

    Overlay.modal: Rectangle {
        color: "black"
        anchors.fill: parent
        opacity: 0.4

        Behavior on opacity {
            NumberAnimation {
                duration: 200
            }
        }
    }

    enter: Transition {
        NumberAnimation {
            property: "opacity";
            from: 0.0;
            to: 1.0
            duration: 200
        }
        NumberAnimation {
            property: "scale";
            from: 0.9;
            to: 1.0
            duration: 200
        }
    }

    exit: Transition {
        NumberAnimation {
            property: "opacity";
            from: 1.0;
            to: 0.0;
            duration: 200
        }
        NumberAnimation {
            property: "scale";
            from: 1.0;
            to: 0.9
            duration: 200
        }
    }
}