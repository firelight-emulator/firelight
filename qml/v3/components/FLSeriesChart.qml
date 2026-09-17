// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Layouts
import Firelight 1.0

//   FLSeriesChart { title: "Audio buffer"; source: PerformanceMonitor.series("audio_buffer"); yMax: 1 }
//   FLSeriesChart { title: "Playtime"; points: [{ x: 0, y: 3 }, { x: 1, y: 5 }]; autoScale: true }
Item {
    id: control

    property string title: ""
    property var source: null
    property var points: []
    property real windowSeconds: 10
    property real yMin: 0
    property real yMax: 1
    property bool autoScale: false
    property color lineColor: Theme.chartSeries[0]
    property color fillColor: Qt.rgba(lineColor.r, lineColor.g, lineColor.b, 0.2)
    property var guideLines: []
    property var bands: []
    property string unit: ""
    property int decimals: 2
    property real chartHeight: AppStyle.chartHeight
    readonly property var hoveredPoint: chart.hoveredPoint

    objectName: "FLSeriesChart|" + title
    implicitWidth: Math.round(240 * AppStyle.scale)
    implicitHeight: layout.implicitHeight

    function formatValue(value) {
        return Number(value).toFixed(control.decimals) + control.unit;
    }

    ColumnLayout {
        id: layout

        anchors.fill: parent
        spacing: AppStyle.spacingXs

        Text {
            visible: control.title !== ""
            text: control.title
            color: Theme.textMuted
            font.family: AppStyle.monoFontFamily
            font.pixelSize: AppStyle.fontSizeXSmall
            font.bold: true
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
                    text: control.formatValue(chart.effectiveYMax)
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
                    text: control.formatValue(chart.effectiveYMin)
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

                SeriesChartItem {
                    id: chart

                    anchors.fill: parent
                    anchors.margins: AppStyle.chartPadding
                    source: control.source
                    points: control.points
                    windowSeconds: control.windowSeconds
                    yMin: control.yMin
                    yMax: control.yMax
                    autoScale: control.autoScale
                    lineColor: control.lineColor
                    fillColor: control.fillColor
                    guideLines: control.guideLines
                    bands: control.bands
                }

                Rectangle {
                    id: tooltip

                    visible: chart.hoveredPoint.value !== undefined
                    x: Math.min(Math.max(0, chart.x + (chart.hoveredPoint.x || 0) - width / 2), parent.width - width)
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
                        text: chart.hoveredPoint.value !== undefined ? control.formatValue(chart.hoveredPoint.value) + "  " + Number(chart.hoveredPoint.secondsAgo).toFixed(1) + "s ago" : ""
                        color: Theme.textPrimary
                        font.family: AppStyle.monoFontFamily
                        font.pixelSize: AppStyle.fontSizeXSmall
                    }
                }
            }
        }
    }
}
