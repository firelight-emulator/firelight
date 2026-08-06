import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

Popup {
    id: control
    property int minWidth: AppStyle.defaultPopupMinimumWidth

    padding: AppStyle.spacingSm
    implicitWidth: Math.max(minWidth, contentItem.implicitWidth + padding * 2)

    onOpened: {
        // TODO
        // Arrows may not leave the popup while it is up. It is declared here
        // rather than on the popup itself because navigation walks items, and a
        // Popup is not one
        contentItem.FLFocus.barrier = true;

        contentItem.forceActiveFocus();
        SoundEffects.openPopup.play();
    }

    // TODO
    // Answers a directional press the popup's own controls refused, and reports
    // whether it consumed it. A popup sits outside the window's content item, so
    // the handler there never sees these keys — the content item calls this:
    //
    //   Keys.onPressed: event => event.accepted = control.navigate(event.key, event.isAutoRepeat)
    function navigate(key: int, isAutoRepeat: bool): bool {
        const focused = control.contentItem.Window.activeFocusItem;

        if (FocusNavigator.move(focused, key, isAutoRepeat) !== FocusNavigator.NoTarget) {
            return true;
        }

        return FocusCursor.bump(key);
    }

    // TODO
    // Focus goes back to whatever opened the popup, which is not the cursor
    // travelling anywhere, so the ring is hidden across the whole close and put
    // back in place once focus has settled. It is a Connections rather than an
    // onAboutToHide handler because a popup declaring its own would replace this
    Connections {
        target: control

        function onAboutToHide() {
            FocusCursor.startBlink();
        }

        function onClosed() {
            Qt.callLater(() => FocusCursor.endBlink());
        }
    }

    background: Rectangle {
        color: Theme.surface
        radius: AppStyle.radiusMd
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
}
