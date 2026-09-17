// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property bool showSampleData: false

    objectName: "PerformanceMonitorPage"

    ScrollView {
        id: scroller

        anchors.fill: parent
        anchors.margins: AppStyle.spacingLg
        contentWidth: availableWidth
        clip: true

        ColumnLayout {
            width: scroller.availableWidth
            spacing: AppStyle.spacingLg

            RowLayout {
                Layout.fillWidth: true

                Text {
                    text: "Performance monitor"
                    color: Theme.textPrimary
                    font.family: AppStyle.fontFamily
                    font.pixelSize: AppStyle.fontSizeLarge
                }

                Item {
                    Layout.fillWidth: true
                }

                Text {
                    text: root.showSampleData ? "Hide sample data" : "Show sample data"
                    color: Theme.textMuted
                    font.family: AppStyle.fontFamily
                    font.pixelSize: AppStyle.fontSizeSmall

                    TapHandler {
                        onTapped: root.showSampleData = !root.showSampleData
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: panel.implicitHeight
                color: Theme.glass
                border.color: Theme.glassBorder
                border.width: 1
                radius: AppStyle.spacingXs

                PerformanceMonitorPanel {
                    id: panel

                    width: parent.width
                }
            }

            FLSeriesChart {
                visible: root.showSampleData
                Layout.fillWidth: true
                title: "SAMPLE DATA"
                autoScale: true
                lineColor: Theme.chartSeries[2]
                unit: " h"
                decimals: 1
                points: [
                    {
                        "x": 0,
                        "y": 1.5
                    },
                    {
                        "x": 1,
                        "y": 3.2
                    },
                    {
                        "x": 2,
                        "y": 2.1
                    },
                    {
                        "x": 3,
                        "y": 4.8
                    },
                    {
                        "x": 4,
                        "y": 4.1
                    },
                    {
                        "x": 5,
                        "y": 5.6
                    },
                    {
                        "x": 6,
                        "y": 3.9
                    }
                ]
                guideLines: [
                    {
                        "value": 4,
                        "color": Theme.textMuted,
                        "dashed": true
                    }
                ]
            }
        }
    }
}
