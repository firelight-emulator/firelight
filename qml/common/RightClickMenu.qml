import QtQuick
import QtQuick.Controls
import QtQuick.Effects

Menu {
    id: control
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    padding: Math.round(6 * AppStyle.scale)
    // horizontalPadding: 8

    overlap: 4

    implicitWidth: {
        var maxWidth = 0

        for (var i = 0; i < count; i++) {
            var item = itemAt(i)
            if (item.implicitWidth > maxWidth) {
                maxWidth = item.implicitWidth
            }
        }

        return maxWidth + horizontalPadding * 2
    }
    implicitHeight: {
        var height = 0

        for (var i = 0; i < count; i++) {
            height += itemAt(i).implicitHeight
        }
        return height + verticalPadding * 2
        
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
        color: Theme.surfaceElevated
        radius: 8
        border.color: Theme.border
        border.width: 1
        // bottomRightRadius: 8
        // bottomLeftRadius: 8

        layer.enabled: true
        layer.effect: MultiEffect {
            // autoPaddingEnabled: false
            // paddingRect: Qt.rect(-16, -16, 32, 32)
            shadowEnabled: true
            shadowColor: Theme.background
            shadowBlur: 1.0
            shadowVerticalOffset: 4.0
            shadowHorizontalOffset: 4.0
        }
    }

    // background: Rectangle {
    //     color: ColorPalette.neutral900
    //     radius: 8
    //     border.color: ColorPalette.neutral700
    //     border.width: 1
    // }

    delegate: RightClickMenuItem {
        id: delegate
        text: text
    }
}