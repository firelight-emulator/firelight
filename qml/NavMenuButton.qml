import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Button {
    id: control

    signal rightClicked

    autoExclusive: true
    checkable: true
    property string labelText
    property string labelIcon
    padding: 8
    contentItem: RowLayout {
        spacing: 8

        Icon {
            id: buttonIcon
            glyph: control.labelIcon
            visible: labelIcon !== ""
            Layout.fillHeight: true

            size: 24
            weight: Font.ExtraLight
            color: control.enabled ? "white" : "#aaaaaa"
        }
        Text {
            id: buttonText
            visible: control.width > 52
            text: control.labelText
            Layout.fillHeight: true
            Layout.fillWidth: true
            font.pixelSize: AppStyle.fontSizeSmall
            font.family: Constants.regularFontFamily
            color: control.enabled ? "white" : "#aaaaaa"
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }
    }

    background: Rectangle {
        color: control.enabled ? (mouse.containsMouse ? "#404143" : (control.checked ? "#58595b" : "transparent")) : "transparent"
        // opacity: control.enabled ? (control.checked ? 1.0 : mouse.containsMouse ? 0.2 : 0.0) : 0.0
        // color: control.checked ?
        //     (mouse.pressed ?
        //         "#2b2b2b"
        //         : ((mouse.containsMouse ?
        //             "#393939"
        //             : "#232323")))
        //     : (mouse.pressed ?
        //         "#000000"
        //         : (mouse.containsMouse ?
        //             "#1a1a1a"
        //             : "transparent"))
        radius: 4
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        hoverEnabled: true
        onClicked: function (event) {
            control.rightClicked();
        }
        cursorShape: control.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
    }
}
