pragma Singleton
import QtQuick
import QtQuick.Effects

Rectangle {
    id: root
    property Item target: null
    property Item blurTarget: null

    readonly property real maxOpacity: 0.6

    readonly property int transitionMs: AppStyle.durationDimmer

    property bool blurEnabled: false
    readonly property real maxBlur: 1.0
    property real currentBlur: callers.length > 0 && blurEnabled ? maxBlur : 0

    readonly property list<Item> callers: []
    readonly property Item currentCaller: callers.length > 0 ? callers[callers.length - 1] : null

    parent: target
    anchors.fill: parent
    color: "transparent"

    ShaderEffectSource {
        id: blurSource
        sourceItem: root.blurTarget
        live: root.currentBlur > 0
        anchors.fill: parent
        visible: root.currentBlur > 0

        hideSource: root.currentBlur > 0

        layer.enabled: true
        layer.effect: MultiEffect {
            blurEnabled: true
            blurMultiplier: 0
            blurMax: 64
            blur: root.currentBlur
            autoPaddingEnabled: false
        }
    }

    Rectangle {
        id: blurOverlay
        anchors.fill: parent

        color: "black"
        opacity: root.callers.length > 0 ? root.maxOpacity : 0

        Behavior on opacity {
            NumberAnimation {
                duration: root.transitionMs
                easing.type: AppStyle.easingStandard
            }
        }

        onOpacityChanged: {
            if (blurOverlay.opacity === 0) {
                callerOverlay.source = "";
            }
        }
    }

    onCurrentCallerChanged: root.redrawCaller()

    Timer {
        id: hideTimer
        interval: AppStyle.durationBase
        repeat: false
        onTriggered: root.hide()
    }

    Behavior on currentBlur {
        NumberAnimation {
            duration: root.transitionMs
            easing.type: AppStyle.easingStandard
        }
    }

    Image {
        id: callerOverlay
        parent: root.target
        z: root.z + 1
        visible: blurOverlay.opacity > 0
    }

    function redrawCaller() {
        const caller = root.currentCaller;

        if (caller === null) {
            return;
        }

        const grabbed = caller.grabToImage(function (result) {
            if (result === null || root.currentCaller !== caller) {
                return;
            }

            const position = caller.mapToItem(root.target, 0, 0);

            callerOverlay.source = result.url;
            callerOverlay.x = position.x;
            callerOverlay.y = position.y;
            callerOverlay.width = caller.width;
            callerOverlay.height = caller.height;
        });

        if (!grabbed) {
            callerOverlay.source = "";
        }
    }

    function show(caller: Item, blur :bool) {
        if (!root.blurEnabled && blur) {
            root.blurEnabled = true;
        }

        callers.push(caller ?? null);
    }

    function hide() {
        if (callers.length === 1) {
            root.blurEnabled = false;
        }
        if (callers.length > 0) {
            callers.pop();
        }
    }

    function hideWithDelay() {
        hideTimer.restart();
    }
}
