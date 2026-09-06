import QtQuick
import QtQuick.Effects
import Firelight 1.0

Rectangle {
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
