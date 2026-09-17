// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Layouts
import Firelight 1.0

Item {
    id: control

    property real windowSeconds: 5
    property real yMaxMs: 33.4
    property bool autoScale: false
    property string boundaryKind: "run_frame"
    property real chartHeight: AppStyle.chartHeight
    property real barScale: 1
    readonly property var hoveredSpan: chart.hoveredSpan
    readonly property real targetIntervalMs: PerformanceMonitor.targetFps > 0 ? 1000 / PerformanceMonitor.targetFps : 0

    objectName: "FLFrameTimeChart"
    implicitWidth: Math.round(240 * AppStyle.scale)
    implicitHeight: layout.implicitHeight

    function colorForKind(kindId) {
        return Theme.chartSeries[(kindId - 1) % Theme.chartSeries.length];
    }

    ColumnLayout {
        id: layout

        anchors.fill: parent
        spacing: AppStyle.spacingXs

        RowLayout {
            Layout.fillWidth: true

            Text {
                text: "FRAME TIME"
                color: Theme.textMuted
                font.family: AppStyle.monoFontFamily
                font.pixelSize: AppStyle.fontSizeXSmall
                font.bold: true
            }

            Item {
                Layout.fillWidth: true
            }

            Text {
                text: chart.barCount + " bars, " + control.boundaryKind + " to " + control.boundaryKind
                color: Theme.textMuted
                font.family: AppStyle.monoFontFamily
                font.pixelSize: AppStyle.fontSizeXSmall
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: AppStyle.spacingXs

            ColumnLayout {
                Layout.preferredWidth: AppStyle.chartAxisWidth
                Layout.fillWidth: false
                Layout.fillHeight: true
                spacing: 0

                Text {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                    text: Number(chart.effectiveYMaxMs).toFixed(1) + " ms"
                    color: Theme.textMuted
                    font.family: AppStyle.monoFontFamily
                    font.pixelSize: AppStyle.fontSizeXSmall
                }

                Item {
                    Layout.fillHeight: true
                }

                Text {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignRight
                    text: "0 ms"
                    color: Theme.textMuted
                    font.family: AppStyle.monoFontFamily
                    font.pixelSize: AppStyle.fontSizeXSmall
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: control.chartHeight
                color: Theme.glass
                border.color: Theme.glassBorder
                border.width: 1
                radius: AppStyle.spacingXs
                clip: true

                SpanChartItem {
                    id: chart

                    anchors.fill: parent
                    anchors.margins: AppStyle.chartPadding
                    source: PerformanceMonitor
                    boundaryKind: control.boundaryKind
                    windowSeconds: control.windowSeconds
                    yMaxMs: control.yMaxMs
                    autoScale: control.autoScale
                    barWidth: Math.round(AppStyle.chartBarWidth * control.barScale)
                    barGap: Math.max(1, Math.round(AppStyle.chartBarGap * control.barScale))
                    seriesColors: Theme.chartSeries
                    barColor: Qt.rgba(1, 1, 1, 0.12)
                    guideLines: control.targetIntervalMs > 0 ? [
                        {
                            "value": control.targetIntervalMs,
                            "color": Theme.textMuted,
                            "dashed": true
                        }
                    ] : []
                }

                Rectangle {
                    id: tooltip

                    visible: chart.hoveredSpan.name !== undefined
                    x: Math.min(Math.max(0, chart.x + (chart.hoveredSpan.x || 0) - width / 2), parent.width - width)
                    y: AppStyle.spacingXs
                    width: tooltipText.implicitWidth + AppStyle.spacingSm * 2
                    height: tooltipText.implicitHeight + AppStyle.spacingXs * 2
                    color: Theme.surface
                    border.color: Theme.border
                    border.width: 1
                    radius: AppStyle.spacingXs

                    Text {
                        id: tooltipText

                        anchors.centerIn: parent
                        text: chart.hoveredSpan.name !== undefined ? chart.hoveredSpan.name + "  " + Number(chart.hoveredSpan.durationMs).toFixed(2) + " ms at +" + Number(chart.hoveredSpan.offsetMs).toFixed(2) + " ms" : ""
                        color: Theme.textPrimary
                        font.family: AppStyle.monoFontFamily
                        font.pixelSize: AppStyle.fontSizeXSmall
                    }
                }
            }
        }

        Flow {
            id: legend

            Layout.fillWidth: true
            spacing: AppStyle.spacingSm

            Repeater {
                model: PerformanceMonitor.kinds

                delegate: Row {
                    required property var modelData

                    visible: modelData.type === "span"
                    spacing: AppStyle.spacingXs

                    Rectangle {
                        width: AppStyle.fontSizeXSmall
                        height: AppStyle.fontSizeXSmall
                        anchors.verticalCenter: parent.verticalCenter
                        radius: 2
                        color: control.colorForKind(modelData.id)
                    }

                    Text {
                        text: modelData.name
                        color: Theme.textMuted
                        font.family: AppStyle.monoFontFamily
                        font.pixelSize: AppStyle.fontSizeXSmall
                    }
                }
            }
        }

        Text {
            Layout.fillWidth: true
            text: "Bars are frame start to frame start, not scanout; the display follows the frame"
            color: Theme.textMuted
            font.family: AppStyle.monoFontFamily
            font.pixelSize: AppStyle.fontSizeXSmall
            wrapMode: Text.WordWrap
        }
    }
}
