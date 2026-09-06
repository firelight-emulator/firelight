import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

Menu {
    id: control

    property int minWidth: AppStyle.defaultPopupMinimumWidth
    property alias openSound: behavior.openSound
    property alias caller: behavior.caller

    padding: AppStyle.spacingMd
    implicitWidth: Math.max(minWidth, contentItem.implicitWidth + padding * 2)
    implicitHeight: contentItem.implicitHeight + padding * 2

    Overlay.modal: Item {}

    property FLPopupBehavior behavior: FLPopupBehavior {
        id: behavior
        popup: control
    }

    function popupFor(caller: Item, desiredX, desiredY) {
        control.caller = caller;

        let finalX = desiredX;
        let finalY = desiredY;

        let windowItem = caller.Window.window.contentItem;
        let mappedPoint = caller.mapToItem(windowItem, desiredX, desiredY);

        // If X is > 0 and Y is 0, we assume the popup wants to be directly to the right of the caller
        if (desiredX > 0 && desiredY === 0) {
            let roomToRight = windowItem.width - mappedPoint.x;

            if (roomToRight < control.width) {
                finalX = -(control.width + AppStyle.spacingSm);
            }
        }

        control.popup(finalX, finalY);
    }

    background: FLPopupSurface {}
}
