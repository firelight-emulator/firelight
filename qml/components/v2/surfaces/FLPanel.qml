import QtQuick
import QtQuick.Controls

// Surface card. variant: solid (functional, opaque) | glass (translucent over
// the blurred background). Put content inside as children — it's a Pane
//
//   FLPanel { ColumnLayout { ... } }
//   FLPanel { variant: "glass"; ... }
Pane {
    id: root

    // solid | glass
    property string variant: "solid"

    padding: AppStyle.spacingLg

    background: Rectangle {
        radius: AppStyle.radiusLg
        color: root.variant === "glass" ? Theme.glass : Theme.surface
        border.width: 1
        border.color: root.variant === "glass" ? Theme.glassBorder : Theme.border
    }
}
