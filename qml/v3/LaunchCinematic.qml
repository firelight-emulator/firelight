// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls
import Firelight 1.0

// TODO
// The launch sequence: a feathered copy of the focused tile expands and fades while the screen goes to
// black. GameplayLayer mounts the game behind the full black, then calls reveal()
Item {
    id: root
    anchors.fill: parent

    property int expandMs: 500
    property int copyFadeDelayMs: 360
    property int copyFadeMs: 400
    property int blackStartMs: 450
    property int blackMs: 320
    property int revealMs: 240
    property real expandScale: 1.4

    signal blackFull
    property Item theItem: null

    readonly property bool active: launchAnim.running || black.visible

    Window {
        id: controlsWindow
        width: 400
        height: 400

        visible: true

        Column {
            anchors.fill: parent
            spacing: 0

            Slider {
                id: expandMsSlider
                from: 100
                to: 2000
                value: root.expandMs
                live: false
                onValueChanged: {
                    root.expandMs = value
                    console.log("expandMsSlider.value: " + value)
                }
            }

            Slider {
                id: copyFadeMsSlider
                from: 100
                to: 2000
                value: root.copyFadeMs
                live: false
                onValueChanged: {
                    root.copyFadeMs = value
                    console.log("copyFadeMsSlider.value: " + value)
                }
            }

            Slider {
                id: copyFadeDelayMsSlider
                from: 0
                to: 1000
                value: root.copyFadeDelayMs
                live: false
                onValueChanged: {
                    root.copyFadeDelayMs = value
                    console.log("copyFadeDelayMsSlider.value: " + value)
                }
            }

            Slider {
                id: blackStartMsSlider
                from: 0
                to: 1000
                value: root.blackStartMs
                live: false
                onValueChanged: {
                    root.blackStartMs = value
                    console.log("blackStartMsSlider.value: " + value)
                }
            }

            Slider {
                id: blackMsSlider
                from: 0
                to: 1000
                value: root.blackMs
                live: false
                onValueChanged: {
                    root.blackMs = value
                    console.log("blackMsSlider.value: " + value)
                }
            }

            Button {
                text: "Launch"
                onClicked: {
                    launchFrom(FocusCursor.highlight.target);
                }
            }

            Button {
                text: "Reveal"
                onClicked: {
                    reveal();
                }
            }

            Slider {
                id: featherSlider
                from: 0
                to: 10
                value: featherRect.feather
                live: false
                onValueChanged: {
                    console.log("featherSlider.value: " + value)
                    featherRect.feather = value
                }
            }

            Slider {
                id: thicknessSlider
                from: 0
                to: 10
                live: false
                value: featherRect.thickness
                onValueChanged: {
                    console.log("thicknessSlider.value: " + value)
                    featherRect.thickness = value
                }
            }

            Item {
                height: 100
                width: 1
            }

            Rectangle {
                id: featherRect

                property real feather: 3.25
                property real thickness: 1.5
                radius: AppStyle.radiusMd

                width: 200
                height: 200
                color: "red"

                layer.enabled: true
                layer.smooth: true
                layer.effect: FLFeatheredMask {
                    radius: AppStyle.radiusMd
                    feather: Math.min(width, height) / featherRect.feather
                    thickness: Math.min(width, height) / featherRect.thickness
                }
            }
        }
    }

    // TODO
    // A null item skips the flourish and only fades
    function launchFrom(item: Item) {
        if (item !== null && item !== undefined) {
            console.log("launchFrom: " + item.objectName + " (" + item.width + "x" + item.height + ")");
            const r = item.mapToItem(root, 0, 0, item.width, item.height);
            copy.x = r.x;
            copy.y = r.y;
            copy.width = r.width;
            copy.height = r.height;
            copy.scale = 1;
            copy.opacity = 0.8
            snapshot.sourceItem = item;
            copy.visible = true;
        } else {
            copy.scale = 1;
            copy.visible = true;
            copy.opacity = 0.8;
        }

        black.opacity = 0;
        black.visible = true;

        revealAnim.stop();
        launchAnim.restart();
    }

    function reveal() {
        launchAnim.stop();
        // copy.visible = false;
        // snapshot.sourceItem = null;

        if (black.visible) {
            revealAnim.restart();
        }
    }

    Item {
        id: copy
        visible: false
        transformOrigin: Item.Center

        layer.enabled: true
        layer.smooth: true
        layer.effect: FLFeatheredMask {
            radius: AppStyle.radiusMd
            feather: Math.min(width, height) / 3.25
            thickness: Math.min(width, height) / 1.5
        }

        ShaderEffectSource {
            id: snapshot
            anchors.fill: parent
            live: true
            hideSource: false
        }
    }

    Rectangle {
        id: black
        anchors.fill: parent
        color: "black"
        opacity: 0
        visible: false
    }

    SequentialAnimation {
        id: launchAnim

        ParallelAnimation {
            SequentialAnimation {
                NumberAnimation {
                    target: copy
                    property: "scale"
                    from: 1
                    to: root.expandScale
                    duration: root.expandMs
                    easing.type: Easing.BezierSpline
                    easing.bezierCurve: [0,1,0.1,.9, 1, 1]
                }
            }
            SequentialAnimation {
                PauseAnimation {
                    duration: root.copyFadeDelayMs
                }

                NumberAnimation {
                    target: copy
                    property: "opacity"
                    to: 0
                    duration: copyFadeMs
                    easing.type: Easing.InQuad
                }
            }

            SequentialAnimation {
                PauseAnimation {
                    duration: root.blackStartMs
                }

                NumberAnimation {
                    target: black
                    property: "opacity"
                    to: 1
                    duration: blackMs
                    easing.type: Easing.InOutQuad
                }
            }
        }

        // ParallelAnimation {
        //     NumberAnimation {
        //         target: copy
        //         property: "opacity"
        //         to: 0
        //         duration: 600
        //         easing.type: Easing.Linear
        //     }
        //
        //     SequentialAnimation {
        //         PauseAnimation {
        //             duration: root.blackStartMs
        //         }
        //         NumberAnimation {
        //             target: black
        //             property: "opacity"
        //             to: 1
        //             duration: root.blackMs
        //             easing.type: Easing.Linear
        //         }
        //         ScriptAction {
        //             script: {
        //                 copy.visible = false;
        //                 snapshot.sourceItem = null;
        //                 root.blackFull();
        //             }
        //         }
        //     }
        // }
    }

    NumberAnimation {
        id: revealAnim
        target: black
        property: "opacity"
        to: 0
        duration: root.revealMs
        easing.type: Easing.InOutQuad
        onFinished: black.visible = false
    }
}
