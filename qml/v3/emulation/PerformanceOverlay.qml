// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    objectName: "PerformanceOverlay|" + (PerformanceStats.pacingMode || "idle")

    visible: PerformanceStats.visible
    implicitWidth: content.implicitWidth + AppStyle.spacingLg * 2
    implicitHeight: content.implicitHeight + AppStyle.spacingLg * 2

    color: Qt.rgba(Theme.surface.r, Theme.surface.g, Theme.surface.b, 0.82)
    border.color: Theme.border
    border.width: 1
    radius: AppStyle.spacingXs

    component SectionLabel: Text {
        Layout.columnSpan: 2
        Layout.topMargin: AppStyle.spacingSm
        color: Theme.textMuted
        font.family: "Consolas, Courier New, monospace"
        font.pixelSize: AppStyle.fontSizeXSmall
        font.bold: true
    }

    component StatName: Text {
        color: Theme.textMuted
        font.family: "Consolas, Courier New, monospace"
        font.pixelSize: AppStyle.fontSizeXSmall
    }

    component StatValue: Text {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignRight
        horizontalAlignment: Text.AlignRight
        color: Theme.textPrimary
        font.family: "Consolas, Courier New, monospace"
        font.pixelSize: AppStyle.fontSizeXSmall
    }

    GridLayout {
        id: content

        anchors.fill: parent
        anchors.margins: AppStyle.spacingLg
        columns: 2
        columnSpacing: AppStyle.spacingLg
        rowSpacing: AppStyle.spacingXs

        SectionLabel {
            text: "CORE AV_INFO"
        }

        StatName {
            text: "Size:"
        }

        StatValue {
            text: PerformanceStats.renderSize
        }

        StatName {
            text: " - Base:"
        }

        StatValue {
            text: PerformanceStats.coreBase
        }

        StatName {
            text: " - Max:"
        }

        StatValue {
            text: PerformanceStats.coreMax
        }

        StatName {
            text: "Aspect:"
        }

        StatValue {
            text: PerformanceStats.aspectRatio
        }

        StatName {
            text: "FPS:"
        }

        StatValue {
            text: PerformanceStats.coreFps
        }

        StatName {
            text: "Sample Rate:"
        }

        StatValue {
            text: PerformanceStats.sampleRate
        }

        SectionLabel {
            text: "VIDEO: " + PerformanceStats.graphicsApi
        }

        StatName {
            text: "Viewport:"
        }

        StatValue {
            text: PerformanceStats.viewport
        }

        StatName {
            text: "Refresh:"
        }

        StatValue {
            text: PerformanceStats.refreshRate
        }

        StatName {
            text: "Frame Rate:"
        }

        StatValue {
            text: PerformanceStats.frameRate
        }

        StatName {
            text: "Frame Time:"
        }

        StatValue {
            text: PerformanceStats.frameTime
        }

        StatName {
            text: " - Deviation:"
        }

        StatValue {
            text: PerformanceStats.frameTimeDeviation
        }

        StatName {
            text: "Submit Time:"
        }

        StatValue {
            text: PerformanceStats.submitTime
        }

        StatName {
            text: " - Deviation:"
        }

        StatValue {
            text: PerformanceStats.submitDeviation
        }

        StatName {
            text: "Spin margin:"
        }

        StatValue {
            text: PerformanceStats.spinMargin
        }

        StatName {
            text: "Wake late avg/peak:"
        }

        StatValue {
            text: PerformanceStats.wakeOvershoot
        }

        StatName {
            text: "Frames:"
        }

        StatValue {
            text: PerformanceStats.framesRun
        }

        StatName {
            text: " - Lost:"
        }

        StatValue {
            text: PerformanceStats.framesLost
        }

        StatName {
            text: "Pacing:"
        }

        StatValue {
            text: PerformanceStats.pacingMode
        }

        SectionLabel {
            text: "AUDIO: " + PerformanceStats.audioDevice
        }

        StatName {
            text: "Buffer:"
        }

        StatValue {
            text: PerformanceStats.bufferCapacity
        }

        StatName {
            text: "Saturation:"
        }

        StatValue {
            text: PerformanceStats.saturation
        }

        StatName {
            text: "Deviation:"
        }

        StatValue {
            text: PerformanceStats.saturationDeviation
        }

        StatName {
            text: "Underrun:"
        }

        StatValue {
            text: PerformanceStats.closeToUnderrun
        }

        StatName {
            text: "Blocking:"
        }

        StatValue {
            text: PerformanceStats.closeToBlocking
        }

        StatName {
            text: "Correction:"
        }

        StatValue {
            text: PerformanceStats.correction
        }

        StatName {
            text: "Rate ratio:"
        }

        StatValue {
            text: PerformanceStats.audioRatio
        }

        StatName {
            text: "Samples:"
        }

        StatValue {
            text: PerformanceStats.samples
        }
    }
}
