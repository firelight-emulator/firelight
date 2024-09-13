import QtQuick
import QtQuick.Controls


Menu {
    id: control

    padding: 4


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
        control.contentData[0].forceActiveFocus()
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
        id: bg
        implicitWidth: 240
        implicitHeight: 60
        color: ColorPalette.neutral1000
        border.color: ColorPalette.neutral800
        radius: 2
    }

    delegate: RightClickMenuItem {
        id: delegate
        text: text
    }
}