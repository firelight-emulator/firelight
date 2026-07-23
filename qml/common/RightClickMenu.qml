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
        var maxWidth = 0;

        for (var i = 0; i < count; i++) {
            var item = itemAt(i);
            if (item.implicitWidth > maxWidth) {
                maxWidth = item.implicitWidth;
            }
        }

        return maxWidth + horizontalPadding * 2;
    }
    implicitHeight: {
        var height = 0;

        for (var i = 0; i < count; i++) {
            height += itemAt(i).implicitHeight;
        }
        return height + verticalPadding * 2;
    }

    currentIndex: 0

    // overlap: 10

    function popupNormal() {
        control.modal = false;
        control.popup();
    }

    function popupModal(x, y) {
        control.modal = true;
        control.popup(x, y);
        // print all keys in first item in content data
        control.itemAt(0).forceActiveFocus();
        currentIndex = 0;
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

    background: Rectangle {
        color: Theme.surfaceElevated
        radius: 8
        border.color: Theme.border
        border.width: 1

        layer.enabled: true
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowColor: Theme.shadow
            shadowBlur: AppStyle.elevationBlur
            shadowVerticalOffset: AppStyle.elevationOffset
            shadowHorizontalOffset: AppStyle.elevationOffset
        }
    }

    delegate: RightClickMenuItem {
        id: delegate
        text: text
    }
}
