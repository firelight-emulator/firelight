import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

Popup {
    id: control
    property int minWidth: 0

    padding: AppStyle.spacingSm
    implicitWidth: Math.max(minWidth, contentItem.implicitWidth + padding * 2)

    onOpened: {
        contentItem.forceActiveFocus();
        SoundEffects.openPopup.play();
    }

    onAboutToHide: {
        // SoundEffects.closePopup.play();
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
