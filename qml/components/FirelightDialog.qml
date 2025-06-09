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

    property string headerText: ""

    property string text
    property bool showButtons: true
    property bool centerButtons: true
    property bool showCancel: true

    property string acceptText: "Yes"
    property string rejectText: "Cancel"

    modal: true
    // parent: Overlay.overlay
    anchors.centerIn: Overlay.overlay
    focus: true
    padding: 24

    property var doOnAccepted

    function openAndDoOnAccepted(doOnAccepted) {
        control.doOnAccepted = doOnAccepted
        control.open()
    }

    implicitWidth: Math.min(parent.width, 560)
    implicitHeight: Math.min(parent.height, Math.max(240, contentItem.implicitHeight + footer.height + spacing + verticalPadding * 2 + header.height))

    header: Text {
        text: control.headerText

        visible: control.headerText !== ""

        font.family: Constants.regularFontFamily
        font.pixelSize: 18
        leftPadding: 24
        topPadding: 16
        rightPadding: 24
        bottomPadding: 0
        color: "white"
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }

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
        // font.weight: Font.Light
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: 18
    }

    footer: FocusScope {
        height: 66
        visible: control.showButtons
        focus: control.showButtons
        RowLayout {
            spacing: 12
            anchors.fill: parent

            anchors.rightMargin: control.centerButtons ? 0 : 12

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            Keys.onPressed: function (event) {
                if (event.key === Qt.Key_Back) {
                    control.reject();
                }
            }

            FirelightButton {
                id: cancelButton
                visible: control.showCancel
                focus: control.showCancel
                Layout.alignment: Qt.AlignHCenter | Qt.AlignTop

                KeyNavigation.right: acceptButton

                label: control.rejectText
                onClicked: control.reject()
            }

            FirelightButton {
                id: acceptButton
                Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                focus: !control.showCancel
                label: control.acceptText
                onClicked: function() {
                    if (control.doOnAccepted) {
                        control.doOnAccepted()
                        control.doOnAccepted = null
                    }
                    control.accept()
                }
            }

            Item {
                Layout.fillWidth: control.centerButtons
                Layout.fillHeight: control.centerButtons
            }
        }
    }

    Overlay.modal: Rectangle {
        color: "black"
        anchors.fill: parent
        opacity: 0.5

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