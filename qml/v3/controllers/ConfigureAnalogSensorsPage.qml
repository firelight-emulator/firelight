// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FLTwoColumnPage {
    id: root
    objectName: "ConfigureAnalogSensorsPage"

    required property int profileId
    property int playerNumber: 1

    headerText: qsTr("Configure Analog Sticks")
    largeMenuRows: true
    contentMaxWidth: AppStyle.readableContentWidth

    GamepadStatus {
        id: gamepadStatus
        playerIndex: root.playerNumber - 1
    }

    model: [
        {
            "type": "page",
            "key": "left-stick",
            "label": qsTr("Left Analog Stick"),
            "page": leftPage
        },
        {
            "type": "page",
            "key": "right-stick",
            "label": qsTr("Right Analog Stick"),
            "page": rightPage
        }
    ]

    component DeadzoneViz: Item {
        id: viz
        property real stickX: 0
        property real stickY: 0

        property real inner: 0
        property real outer: 0
        implicitWidth: 150
        implicitHeight: 150

        readonly property real ringRadius: Math.min(width, height) / 2 - 4
        readonly property real rawDistance: Math.sqrt(stickX * stickX + stickY * stickY)
        // TODO
        // 0 inside the inner deadzone, then stretched so the edge of the outer deadzone is full tilt
        readonly property real gameDistance: rawDistance <= inner ? 0 : Math.min(Math.max((Math.min(rawDistance, 1) - inner) / Math.max(1 - outer - inner, 0.0001), 0), 1)
        readonly property real gameX: rawDistance > 0 ? stickX / rawDistance * gameDistance : 0
        readonly property real gameY: rawDistance > 0 ? stickY / rawDistance * gameDistance : 0

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

                // TODO
                // Active region: the ring between the inner and outer deadzones, which stay unfilled
                ctx.fillStyle = "rgba(27, 203, 253, 0.28)";
                ctx.fillRule = Qt.OddEvenFill;
                ctx.beginPath();
                ctx.arc(cx, cy, R * (1 - viz.outer), 0, 2 * Math.PI);
                ctx.moveTo(cx + R * viz.inner, cy);
                ctx.arc(cx, cy, R * viz.inner, 0, 2 * Math.PI);
                ctx.fill();

                // Full-range boundary
                ctx.strokeStyle = positionIndicator.atEdge ? Theme.switch2Color : "#ffffff";
                ctx.lineWidth = 2;
                ctx.beginPath();
                ctx.arc(cx, cy, R, 0, 2 * Math.PI);
                ctx.stroke();

                // Crosshair
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

        Rectangle {
            id: rawPositionIndicator
            readonly property real clampFactor: viz.rawDistance > 1 ? 1 / viz.rawDistance : 1

            border.color: "#ffffff"
            border.width: 2
            opacity: 0.8
            color: "transparent"
            height: 18
            width: 18
            radius: width / 2

            x: viz.width / 2 + viz.stickX * clampFactor * viz.ringRadius - width / 2
            y: viz.height / 2 + viz.stickY * clampFactor * viz.ringRadius - height / 2
        }

        Rectangle {
            id: positionIndicator
            readonly property bool isLive: viz.gameDistance > 0
            readonly property bool atEdge: viz.rawDistance >= 1 - viz.outer

            onAtEdgeChanged: {
                canvas.requestPaint()
            }

            color: isLive ? Theme.switch2Color : "#ffffff"
            height: 12
            width: 12
            radius: width / 2

            x: viz.width / 2 + viz.gameX * viz.ringRadius - width / 2
            y: viz.height / 2 + viz.gameY * viz.ringRadius - height / 2
        }
    }

    component AnalogTuningPage: RowLayout {
        id: analogTuningPage
        required property int profileId
        property bool leftStick: true

        FLColumnLayout {
            id: stickPage
            Layout.fillWidth: true
            Layout.fillHeight: true

            property bool testing: false

            SettingBinding {
                id: innerDeadzoneBinding
                key: analogTuningPage.leftStick ? "left-stick-inner-deadzone" : "right-stick-inner-deadzone"
                controllerProfileId: analogTuningPage.profileId
            }

            SettingBinding {
                id: outerDeadzoneBinding
                key: analogTuningPage.leftStick ? "left-stick-outer-deadzone" : "right-stick-outer-deadzone"
                controllerProfileId: analogTuningPage.profileId
            }

            FLFocus.actions: [
                FLAction {
                    keys: [Qt.Key_PageUp]
                    label: qsTr("Start testing")
                    onTriggered: {
                        stickPage.testing = true
                    }
                }
            ]

            RowLayout {
                spacing: AppStyle.spacingLg

                DeadzoneViz {
                    id: testView

                    Layout.fillHeight: true

                    FLFocus.surface: true
                    FLFocus.surfaceActive: stickPage.testing
                    FLFocus.layer: 25
                    FLFocus.entry: catcher

                    inner: Number(innerDeadzoneBinding.value) / 32768
                    outer: Number(outerDeadzoneBinding.value) / 32768
                    stickX: analogTuningPage.leftStick ? gamepadStatus.leftStickX : gamepadStatus.rightStickX
                    stickY: analogTuningPage.leftStick ? gamepadStatus.leftStickY : gamepadStatus.rightStickY

                    Item {
                        id: catcher
                        anchors.fill: parent
                        enabled: stickPage.testing

                        Keys.onPressed: event => {
                            if (event.key === Qt.Key_Escape) {
                                stickPage.testing = false;
                            }

                            event.accepted = true;
                        }
                    }
                }

                FLColumnLayout {
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    spacing: AppStyle.spacingMd

                    Row {
                        Layout.fillWidth: true
                        Text {
                            text: qsTr("Raw position: %1, %2").arg(Math.round(testView.stickX * 100) / 100).arg(Math.round(testView.stickY * 100) / 100)
                            color: Theme.textPrimary
                            font.pixelSize: AppStyle.fontSizeMedium
                            font.family: AppStyle.fontFamily
                            font.weight: Font.Medium
                        }
                    }

                    Row {
                        Layout.fillWidth: true
                        Text {
                            text: qsTr("Game position: %1, %2").arg(Math.round(testView.gameX * 100) / 100).arg(Math.round(testView.gameY * 100) / 100)
                            color: Theme.textPrimary
                            font.pixelSize: AppStyle.fontSizeMedium
                            font.family: AppStyle.fontFamily
                            font.weight: Font.Medium
                        }
                    }

                    FLButton {
                        variant: stickPage.testing ? "primary" : "secondary"
                        text: stickPage.testing ? qsTr("Stop Testing") : qsTr("Start Testing")
                        onClicked: {
                            stickPage.testing = !stickPage.testing
                        }
                    }
                }
            }

            SettingsGroup {
                group: analogTuningPage.leftStick ? "controller-left-stick-calibration" : "controller-right-stick-calibration"
                showHeader: false
                profileId: analogTuningPage.profileId
            }

            FLColumnSpacer {}
        }
    }

    Component {
        id: leftPage

        AnalogTuningPage {
            profileId: root.profileId
            leftStick: true
        }
    }

    Component {
        id: rightPage

        AnalogTuningPage {
            profileId: root.profileId
            leftStick: false
        }
    }
}