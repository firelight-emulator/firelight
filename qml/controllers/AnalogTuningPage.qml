import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// Standalone editor for a profile's analog stick/trigger tuning. Bind
// `profileId` to the profile being edited. Every change is persisted
// immediately via AnalogSettingsModel.
FocusScope {
    id: root

    property int profileId: -1

    AnalogSettingsModel {
        id: analog
        profileId: root.profileId
    }

    // A labeled slider row with a live value readout.
    component TuningSlider: ColumnLayout {
        id: ts
        property string label: ""
        property real value: 0
        property real from: 0
        property real to: 1
        property real step: 0.01
        property bool percent: true
        signal moved(real v)

        Layout.fillWidth: true
        spacing: 2

        RowLayout {
            Layout.fillWidth: true
            Text {
                text: ts.label
                color: ColorPalette.neutral100
                font.family: Constants.regularFontFamily
                font.pixelSize: 15
                Layout.fillWidth: true
            }
            Text {
                text: ts.percent ? Math.round(ts.value * 100) + "%" : ts.value.toFixed(2)
                color: ColorPalette.neutral300
                font.family: Constants.regularFontFamily
                font.pixelSize: 15
            }
        }

        Slider {
            id: slider
            Layout.fillWidth: true
            from: ts.from
            to: ts.to
            stepSize: ts.step
            value: ts.value
            onMoved: ts.moved(slider.value)
        }
    }

    // Circular visualization of a stick's inner/outer deadzone.
    component DeadzoneViz: Item {
        id: viz
        property real inner: 0
        property real outer: 0
        implicitWidth: 150
        implicitHeight: 150

        onInnerChanged: canvas.requestPaint()
        onOuterChanged: canvas.requestPaint()

        Canvas {
            id: canvas
            anchors.fill: parent
            onPaint: {
                const ctx = getContext("2d");
                ctx.reset();
                const cx = width / 2;
                const cy = height / 2;
                const R = Math.min(width, height) / 2 - 4;

                // Active region (outer boundary minus outer deadzone).
                ctx.fillStyle = "rgba(209, 76, 32, 0.28)";
                ctx.beginPath();
                ctx.arc(cx, cy, R * (1 - viz.outer), 0, 2 * Math.PI);
                ctx.fill();

                // Inner deadzone disc.
                ctx.fillStyle = "rgba(255, 255, 255, 0.14)";
                ctx.beginPath();
                ctx.arc(cx, cy, R * viz.inner, 0, 2 * Math.PI);
                ctx.fill();

                // Full-range boundary.
                ctx.strokeStyle = "#888888";
                ctx.lineWidth = 2;
                ctx.beginPath();
                ctx.arc(cx, cy, R, 0, 2 * Math.PI);
                ctx.stroke();

                // Crosshair.
                ctx.strokeStyle = "#555555";
                ctx.lineWidth = 1;
                ctx.beginPath();
                ctx.moveTo(cx - R, cy);
                ctx.lineTo(cx + R, cy);
                ctx.moveTo(cx, cy - R);
                ctx.lineTo(cx, cy + R);
                ctx.stroke();
            }
        }
    }

    // A full tuning panel for one stick (sliders + visualizer).
    component StickPanel: RowLayout {
        id: panel
        property string title: ""
        // Bound to the model's properties by the caller.
        property real inner: 0
        property real outer: 0
        property real sensitivity: 1
        property real antiDeadzone: 0
        property real curveExponent: 1
        property int curve: 0
        signal innerMoved(real v)
        signal outerMoved(real v)
        signal sensitivityMoved(real v)
        signal antiMoved(real v)
        signal curveExponentMoved(real v)
        signal curveSelected(int v)

        Layout.fillWidth: true
        spacing: 24

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6

            Text {
                text: panel.title
                color: ColorPalette.neutral100
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
                font.pixelSize: 17
                Layout.bottomMargin: 4
            }
            TuningSlider {
                label: "Inner deadzone"
                value: panel.inner
                onMoved: panel.innerMoved(v)
            }
            TuningSlider {
                label: "Outer deadzone"
                value: panel.outer
                onMoved: panel.outerMoved(v)
            }
            TuningSlider {
                label: "Sensitivity"
                from: 0.1
                to: 2.0
                value: panel.sensitivity
                percent: false
                onMoved: panel.sensitivityMoved(v)
            }
            TuningSlider {
                label: "Anti-deadzone"
                value: panel.antiDeadzone
                onMoved: panel.antiMoved(v)
            }

            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: "Response curve"
                    color: ColorPalette.neutral100
                    font.family: Constants.regularFontFamily
                    font.pixelSize: 15
                    Layout.fillWidth: true
                }
                ComboBox {
                    model: ["Linear", "Exponential"]
                    currentIndex: panel.curve
                    onActivated: panel.curveSelected(currentIndex)
                }
            }
            TuningSlider {
                label: "Curve strength"
                from: 0.5
                to: 3.0
                value: panel.curveExponent
                percent: false
                enabled: panel.curve === 1
                opacity: panel.curve === 1 ? 1 : 0.4
                onMoved: panel.curveExponentMoved(v)
            }
        }

        DeadzoneViz {
            Layout.alignment: Qt.AlignTop
            inner: panel.inner
            outer: panel.outer
        }
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        clip: true

        ColumnLayout {
            width: root.width
            spacing: 28

            Text {
                text: "Analog Tuning"
                color: ColorPalette.neutral100
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
                font.pixelSize: 22
                Layout.topMargin: 8
            }

            StickPanel {
                title: "Left Stick"
                inner: analog.leftStickInnerDeadzone
                outer: analog.leftStickOuterDeadzone
                sensitivity: analog.leftStickSensitivity
                antiDeadzone: analog.leftStickAntiDeadzone
                curve: analog.leftStickCurve
                curveExponent: analog.leftStickCurveExponent
                onInnerMoved: analog.leftStickInnerDeadzone = v
                onOuterMoved: analog.leftStickOuterDeadzone = v
                onSensitivityMoved: analog.leftStickSensitivity = v
                onAntiMoved: analog.leftStickAntiDeadzone = v
                onCurveSelected: analog.leftStickCurve = v
                onCurveExponentMoved: analog.leftStickCurveExponent = v
            }

            StickPanel {
                title: "Right Stick"
                inner: analog.rightStickInnerDeadzone
                outer: analog.rightStickOuterDeadzone
                sensitivity: analog.rightStickSensitivity
                antiDeadzone: analog.rightStickAntiDeadzone
                curve: analog.rightStickCurve
                curveExponent: analog.rightStickCurveExponent
                onInnerMoved: analog.rightStickInnerDeadzone = v
                onOuterMoved: analog.rightStickOuterDeadzone = v
                onSensitivityMoved: analog.rightStickSensitivity = v
                onAntiMoved: analog.rightStickAntiDeadzone = v
                onCurveSelected: analog.rightStickCurve = v
                onCurveExponentMoved: analog.rightStickCurveExponent = v
            }

            Text {
                text: "Triggers"
                color: ColorPalette.neutral100
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
                font.pixelSize: 17
            }
            TuningSlider {
                label: "Left trigger press threshold"
                value: analog.leftTriggerThreshold
                onMoved: analog.leftTriggerThreshold = v
            }
            TuningSlider {
                label: "Right trigger press threshold"
                value: analog.rightTriggerThreshold
                onMoved: analog.rightTriggerThreshold = v
            }

            Button {
                text: "Reset to defaults"
                onClicked: analog.resetToDefault()
                Layout.topMargin: 8
                Layout.bottomMargin: 20
            }
        }
    }
}
