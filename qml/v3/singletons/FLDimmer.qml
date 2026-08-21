pragma Singleton
import QtQuick

Rectangle {
    id: root
    property Item target: null

    readonly property real maxOpacity: 0.75

    readonly property list<Item> callers: []
    readonly property Item currentCaller: callers.length > 0 ? callers[callers.length - 1] : null

    parent: target
    anchors.fill: parent
    color: "black"
    opacity: callers.length > 0 ? maxOpacity : 0

    onCurrentCallerChanged: root.redrawCaller()

    onOpacityChanged: {
        if (root.opacity === 0) {
            callerOverlay.source = "";
        }
    }

    Timer {
        id: hideTimer
        interval: AppStyle.durationBase
        repeat: false
        onTriggered: root.hide()
    }

    Behavior on opacity {
        NumberAnimation {
            duration: AppStyle.durationBase
            easing.type: AppStyle.easingStandard
        }
    }

    // The caller sits under the dim and cannot be raised out of it, so what shows through is a
    // picture of it drawn on top
    Image {
        id: callerOverlay
        parent: root.target
        z: root.z + 1
        visible: root.opacity > 0
    }

    // Grabs the current caller and draws it over the dim. The grab finishes later, by which time
    // the caller may have changed or gone, so the result is dropped unless it is still wanted
    function redrawCaller() {
        const caller = root.currentCaller;

        if (caller === null) {

            // Only clear the overlay if the dim is NOT being removed - we want the overlay to be cleared
            // after the dim is removed for the last caller
            if (callers.length > 1) {
                callerOverlay.source = "";
            }
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

    function show(caller: Item) {
        callers.push(caller ?? null);
    }

    function hide() {
        if (callers.length > 0) {
            callers.pop();
        }
    }

    function hideWithDelay() {
        hideTimer.restart();
    }
}
