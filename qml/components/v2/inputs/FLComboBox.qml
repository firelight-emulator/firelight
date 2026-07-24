import QtQuick
import QtQuick.Controls

// Themed dropdown. Respects textRole via textAt(); metrics from AppStyle,
// colors from Theme
ComboBox {
    id: control

    // No stable identifying value — displayText changes with the selection, so
    // call sites that need a fixed handle should set their own
    objectName: "FLComboBox"

    font.pixelSize: AppStyle.fontSizeMedium
    font.family: AppStyle.fontFamily
    // TODO
    // Size to the widest option (not the current text), floored so it stays
    // usable in a right-aligned settings slot
    implicitContentWidthPolicy: ComboBox.WidestText
    implicitWidth: Math.max(AppStyle.inputWidth, implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(AppStyle.minTarget, AppStyle.controlHeight)
    leftPadding: AppStyle.spacingMd
    rightPadding: AppStyle.iconSizeMd + AppStyle.spacingMd

    contentItem: Text {
        text: control.displayText
        color: Theme.textPrimary
        font: control.font
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    indicator: Icon {
        x: control.width - width - AppStyle.spacingSm
        y: control.topPadding + (control.availableHeight - height) / 2
        name: "arrow_drop_down"
        size: AppStyle.iconSizeMd
        color: Theme.textMuted
    }

    background: Rectangle {
        radius: AppStyle.radiusMd
        color: control.activeFocus ? Theme.surfaceHover : Theme.surfaceElevated
        border.width: control.activeFocus ? 2 : 1
        border.color: control.activeFocus ? Theme.accent : Theme.border
    }

    delegate: ItemDelegate {
        id: item
        required property int index
        width: control.width
        height: Math.max(AppStyle.minTarget, AppStyle.controlHeight)
        highlighted: control.highlightedIndex === index
        contentItem: Text {
            text: control.textAt(item.index)
            color: Theme.textPrimary
            font: control.font
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        background: Rectangle {
            color: item.highlighted ? Theme.surfaceHover : "transparent"
        }
    }

    popup: Popup {
        y: control.height + AppStyle.spacingXs
        width: control.width
        implicitHeight: Math.min(contentItem.implicitHeight, Math.round(300 * AppStyle.scale))
        padding: 1

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            ScrollIndicator.vertical: ScrollIndicator {}
        }
        background: Rectangle {
            radius: AppStyle.radiusMd
            color: Theme.surfaceElevated
            border.width: 1
            border.color: Theme.border
        }
    }
}
