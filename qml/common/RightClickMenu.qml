import QtQuick
import QtQuick.Controls


Menu {
    id: control
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

    padding: 8
    // horizontalPadding: 8

    implicitWidth: 300
    implicitHeight: {
        var height = 16

        for (var i = 0; i < count; i++) {
            height += itemAt(i).implicitHeight
        }
        return height
        
    }

    currentIndex: 0

    // overlap: 10

    function popupNormal() {
        control.modal = false
        control.popup()
    }

    function popupModal(x, y) {
        control.modal = true
        control.popup(x, y)
        // print all keys in first item in content data
        control.itemAt(0).forceActiveFocus()
        currentIndex = 0
    }

    Overlay.modal: Rectangle {
        color: "black"
        opacity: 0.45

        Behavior on opacity {
            NumberAnimation {
                duration: 100
                easing.type: Easing.InOutQuad
            }
        }
    }

    // contentWidth: 260
    // contentHeight: Constants.rightClickMenuItem_DefaultHeight

    // implicitContentWidth: 260
    // // implicitContentHeight: control.count * Constants.rightClickMenuItem_DefaultHeight

    background: Rectangle {
        color: ColorPalette.neutral900
        radius: 8
        border.color: ColorPalette.neutral700
        border.width: 1
    }

    delegate: RightClickMenuItem {
        id: delegate
        text: text
    }
}