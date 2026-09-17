// TODO: NEEDS REVIEW
import QtQuick

FLSeriesChart {
    id: root

    objectName: "AudioBufferChart"
    title: "AUDIO BUFFER"
    source: PerformanceMonitor.series("audio_buffer")
    windowSeconds: 10
    yMin: 0
    yMax: 1
    lineColor: Theme.chartSeries[1]
    unit: ""
    decimals: 2
    bands: [
        {
            "from": 0,
            "to": 0.25,
            "color": Qt.rgba(Theme.danger.r, Theme.danger.g, Theme.danger.b, 0.18)
        },
        {
            "from": 0.75,
            "to": 1,
            "color": Qt.rgba(Theme.danger.r, Theme.danger.g, Theme.danger.b, 0.18)
        }
    ]
    guideLines: [
        {
            "value": 0.5,
            "color": Theme.textMuted,
            "dashed": true
        }
    ]
}
