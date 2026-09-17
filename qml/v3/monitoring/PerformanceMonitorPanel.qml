// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Layouts

Item {
    id: root

    property real chartScale: 1
    readonly property var chartScales: [0.75, 1, 1.5, 2, 3]

    objectName: "PerformanceMonitorPanel|" + (PerformanceMonitor.pacingMode || "idle")
    implicitWidth: Math.round(820 * AppStyle.scale)
    implicitHeight: content.implicitHeight + AppStyle.spacingLg * 2

    function dash(value, decimals, suffix) {
        return value > 0 ? Number(value).toFixed(decimals) + suffix : "—";
    }

    function figure(value, decimals, suffix) {
        return Number(value).toFixed(decimals) + suffix;
    }

    function size(width, height) {
        return width + " x " + height;
    }

    function stepChartScale(direction) {
        const index = root.chartScales.indexOf(root.chartScale);
        const next = Math.min(root.chartScales.length - 1, Math.max(0, (index < 0 ? 1 : index) + direction));
        root.chartScale = root.chartScales[next];
    }

    component HeaderButton: Rectangle {
        id: button

        property string label: ""
        property bool active: false
        signal clicked

        objectName: "PerformanceMonitorButton|" + label
        implicitWidth: buttonText.implicitWidth + AppStyle.spacingSm * 2
        implicitHeight: buttonText.implicitHeight + AppStyle.spacingXs * 2
        radius: AppStyle.spacingXs
        color: button.active ? Theme.danger : (buttonHover.hovered ? Theme.glassElevated : Theme.glass)
        border.color: Theme.glassBorder
        border.width: 1

        Text {
            id: buttonText

            anchors.centerIn: parent
            text: button.label
            color: Theme.textPrimary
            font.family: AppStyle.monoFontFamily
            font.pixelSize: AppStyle.fontSizeXSmall
        }

        HoverHandler {
            id: buttonHover
        }

        TapHandler {
            onTapped: button.clicked()
        }
    }

    ColumnLayout {
        id: content

        anchors.fill: parent
        anchors.margins: AppStyle.spacingLg
        spacing: AppStyle.spacingXs

        RowLayout {
            Layout.fillWidth: true
            spacing: AppStyle.spacingSm

            Text {
                text: PerformanceMonitor.paused ? "PERFORMANCE MONITOR  (paused)" : "PERFORMANCE MONITOR"
                color: PerformanceMonitor.paused ? Theme.danger : Theme.textMuted
                font.family: AppStyle.monoFontFamily
                font.pixelSize: AppStyle.fontSizeXSmall
                font.bold: true
            }

            Item {
                Layout.fillWidth: true
            }

            HeaderButton {
                label: PerformanceMonitor.paused ? "Resume" : "Pause"
                active: PerformanceMonitor.paused
                onClicked: PerformanceMonitor.togglePaused()
            }

            HeaderButton {
                label: "−"
                onClicked: root.stepChartScale(-1)
            }

            Text {
                text: "charts x" + root.chartScale
                color: Theme.textMuted
                font.family: AppStyle.monoFontFamily
                font.pixelSize: AppStyle.fontSizeXSmall
            }

            HeaderButton {
                label: "+"
                onClicked: root.stepChartScale(1)
            }
        }

        Text {
            visible: PerformanceMonitor.lappedRecords > 0 || PerformanceMonitor.droppedRecords > 0
            Layout.fillWidth: true
            text: "History lost: " + PerformanceMonitor.lappedRecords + " lapped, " + PerformanceMonitor.droppedRecords + " dropped"
            color: Theme.danger
            font.family: AppStyle.monoFontFamily
            font.pixelSize: AppStyle.fontSizeXSmall
            wrapMode: Text.WordWrap
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 3
            columnSpacing: AppStyle.spacingXl
            rowSpacing: 0

            MonitorSection {
                Layout.alignment: Qt.AlignTop
                title: "CORE AV_INFO"

                MonitorStatRow {
                    label: "Size:"
                    value: root.size(PerformanceMonitor.renderWidth, PerformanceMonitor.renderHeight)
                }

                MonitorStatRow {
                    label: " - Base:"
                    value: root.size(PerformanceMonitor.baseWidth, PerformanceMonitor.baseHeight)
                }

                MonitorStatRow {
                    label: " - Max:"
                    value: root.size(PerformanceMonitor.maxWidth, PerformanceMonitor.maxHeight)
                }

                MonitorStatRow {
                    label: "Aspect:"
                    value: root.dash(PerformanceMonitor.aspectRatio, 3, "")
                }

                MonitorStatRow {
                    label: "FPS:"
                    value: root.dash(PerformanceMonitor.coreFps, 3, "")
                }

                MonitorStatRow {
                    label: "Sample Rate:"
                    value: root.dash(PerformanceMonitor.coreSampleRate, 2, "")
                }
            }

            MonitorSection {
                Layout.alignment: Qt.AlignTop
                title: "VIDEO: " + (PerformanceMonitor.graphicsApi || "—")

                MonitorStatRow {
                    label: "Viewport:"
                    value: root.size(PerformanceMonitor.viewportWidth, PerformanceMonitor.viewportHeight)
                }

                MonitorStatRow {
                    label: "Refresh:"
                    value: root.dash(PerformanceMonitor.displayHz, 3, " Hz")
                }

                MonitorStatRow {
                    label: "Frame Rate:"
                    value: root.dash(PerformanceMonitor.frameRate, 3, " fps")
                    valueColor: PerformanceMonitor.rateMatchesTarget ? Theme.textPrimary : Theme.danger
                }

                MonitorStatRow {
                    label: " - Target:"
                    value: root.dash(PerformanceMonitor.targetFps, 3, " fps")
                }

                MonitorStatRow {
                    label: "Frame Time:"
                    value: root.dash(PerformanceMonitor.frameTimeMs, 2, " ms")
                }

                MonitorStatRow {
                    label: " - Deviation:"
                    value: root.figure(PerformanceMonitor.frameTimeDeviationPercent, 2, " %")
                }

                MonitorStatRow {
                    label: "Submit Time:"
                    value: root.dash(PerformanceMonitor.submitTimeMs, 2, " ms")
                }

                MonitorStatRow {
                    label: " - Deviation:"
                    value: root.figure(PerformanceMonitor.submitDeviationPercent, 2, " %")
                }

                MonitorStatRow {
                    label: "Vblank to submit:"
                    value: root.dash(PerformanceMonitor.vblankToSubmitMs, 2, " ms")
                }

                MonitorStatRow {
                    label: "Vblank to wait end:"
                    value: root.dash(PerformanceMonitor.vblankToWaitEndMs, 2, " ms")
                }

                MonitorStatRow {
                    label: "Vblank to frame end:"
                    value: root.dash(PerformanceMonitor.vblankToFrameEndMs, 2, " ms") + " ± " + PerformanceMonitor.frameEndSpreadMs.toFixed(2)
                }

                MonitorStatRow {
                    label: "Vblank to frame grab:"
                    value: root.dash(PerformanceMonitor.vblankToFrameGrabMs, 2, " ms") + " ± " + PerformanceMonitor.frameGrabSpreadMs.toFixed(2)
                }

                MonitorStatRow {
                    label: "Per blank:"
                    value: PerformanceMonitor.presentsPerBlank.toFixed(3) + " presents, " + PerformanceMonitor.framesPerBlank.toFixed(3) + " frames"
                }

                MonitorStatRow {
                    label: " - Blanks, no present / two:"
                    value: String(PerformanceMonitor.blanksWithoutPresent) + " / " + String(PerformanceMonitor.blanksWithTwoPresents)
                }

                MonitorStatRow {
                    label: " - Blanks, no frame / two:"
                    value: String(PerformanceMonitor.blanksWithoutFrame) + " / " + String(PerformanceMonitor.blanksWithTwoFrames)
                }

                MonitorStatRow {
                    label: "Spin margin:"
                    value: root.figure(PerformanceMonitor.spinMarginMs, 2, " ms")
                }

                MonitorStatRow {
                    label: "Wake late avg/peak:"
                    value: root.figure(PerformanceMonitor.wakeOvershootMeanMs, 2, "") + " / " + root.figure(PerformanceMonitor.wakeOvershootPeakMs, 2, " ms")
                }

                MonitorStatRow {
                    label: "Grid:"
                    value: (PerformanceMonitor.phaseLocked ? "locked " : "free ") + root.figure(PerformanceMonitor.refreshPeriodMs, 3, " ms")
                }

                MonitorStatRow {
                    label: " - Slips:"
                    value: String(PerformanceMonitor.slips)
                }

                MonitorStatRow {
                    label: " - Debt / time dropped:"
                    value: PerformanceMonitor.slipDebtMs.toFixed(2) + " ms / " + String(PerformanceMonitor.timeDrops)
                }

                MonitorStatRow {
                    label: "Frames:"
                    value: String(PerformanceMonitor.framesRun)
                }

                MonitorStatRow {
                    label: " - Lost:"
                    value: String(PerformanceMonitor.framesLost)
                }

                MonitorStatRow {
                    label: " - Shown:"
                    value: String(PerformanceMonitor.framesShown)
                }

                MonitorStatRow {
                    label: " - Not shown:"
                    value: String(PerformanceMonitor.framesNotShown)
                }

                MonitorStatRow {
                    label: " - Repeated:"
                    value: String(PerformanceMonitor.framesRepeated)
                }

                MonitorStatRow {
                    label: "   - Near a slip:"
                    value: String(PerformanceMonitor.repeatsNearSlip)
                }

                MonitorStatRow {
                    label: " - No picture:"
                    value: String(PerformanceMonitor.framesNoPicture)
                }

                MonitorStatRow {
                    label: "Blanks / presents / passes:"
                    value: String(PerformanceMonitor.blanks) + " / " + String(PerformanceMonitor.presents) + " / " + String(PerformanceMonitor.passes)
                }

                MonitorStatRow {
                    label: "Pacing:"
                    value: PerformanceMonitor.pacingMode || "—"
                }
            }

            MonitorSection {
                Layout.alignment: Qt.AlignTop
                title: "AUDIO: " + (PerformanceMonitor.audioDevice || "—")

                MonitorStatRow {
                    label: "Buffer:"
                    value: PerformanceMonitor.bufferCapacityBytes > 0 ? PerformanceMonitor.bufferCapacityBytes + " B" : "—"
                }

                MonitorStatRow {
                    label: "Saturation:"
                    value: root.figure(PerformanceMonitor.bufferSaturationPercent, 2, " %")
                }

                MonitorStatRow {
                    label: "Deviation:"
                    value: root.figure(PerformanceMonitor.bufferDeviationPercent, 2, " %")
                }

                MonitorStatRow {
                    label: "Underrun:"
                    value: root.figure(PerformanceMonitor.closeToUnderrunPercent, 2, " %")
                }

                MonitorStatRow {
                    label: "Blocking:"
                    value: root.figure(PerformanceMonitor.closeToBlockingPercent, 2, " %")
                }

                MonitorStatRow {
                    label: "Correction:"
                    value: (PerformanceMonitor.correctionPercent >= 0 ? "+" : "") + root.figure(PerformanceMonitor.correctionPercent, 4, " %")
                }

                MonitorStatRow {
                    label: "Rate ratio:"
                    value: "x" + root.figure(PerformanceMonitor.audioRatio, 4, "")
                }

                MonitorStatRow {
                    label: "Samples:"
                    value: String(PerformanceMonitor.samplesDelivered)
                }
            }
        }

        FLFrameTimeChart {
            Layout.fillWidth: true
            Layout.topMargin: AppStyle.spacingSm
            chartHeight: Math.round(AppStyle.chartHeight * root.chartScale)
            barScale: root.chartScale
        }

        AudioBufferChart {
            Layout.fillWidth: true
            Layout.topMargin: AppStyle.spacingSm
            chartHeight: Math.round(AppStyle.chartHeight * root.chartScale)
        }
    }
}
