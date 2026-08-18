// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls
import Firelight 1.0

import "slider_slide.js" as Slide

Slider {
    id: control

    // Gamepad and keyboard focus has to be visible; a mouse user already knows
    // where they are, so the ring is suppressed for them
    readonly property bool _focusRing: activeFocus && !InputMethodManager.usingMouse

    readonly property int _handle: Math.max(AppStyle.minTarget / 2, Math.round(26 * AppStyle.scale))

    implicitWidth: AppStyle.inputWidth
    implicitHeight: Math.max(AppStyle.minTarget, AppStyle.controlHeight)
    opacity: control.enabled ? 1 : 0.4

    FLFocus.proxy: theHandle
    FLFocus.showCursor: true
    FLFocus.spacing: -1

    // TODO
    // Which way a held arrow is carrying the handle, and whether the hold has lasted long enough to
    // slide rather than step
    property int _direction: 0
    property bool _sliding: false

    // TODO
    // Both arrows tracked separately, so releasing one hands back to the other rather than stopping
    property bool _leftHeld: false
    property bool _rightHeld: false
    property int _lastPressed: 0

    // TODO
    // Whether this arrival at an end has already shown itself, so holding against one bumps once
    property bool _bumped: false

    // TODO
    // Whether the handle is being worked right now, by a held arrow or by the pointer. The two slider
    // sounds are the edges of this rather than of any one input, so a drag sounds like a hold and
    // leaving a slider nobody touched stays quiet
    readonly property bool _inMotion: control._direction !== 0 || control.pressed

    on_InMotionChanged: {
        if (control._inMotion) {
            SoundEffects.startSliderMove.play();
            return;
        }

        SoundEffects.stopSliderMove.play();
    }

    // TODO
    // Shows the handle going nowhere when it is carried past an end, once per arrival
    function bumpAtEnd(direction: int) {
        if (Slide.shouldBump(control._bumped, control.value, control.from, control.to, direction)) {
            control._bumped = true;
            FocusCursor.bump(direction < 0 ? Qt.Key_Left : Qt.Key_Right);
            return;
        }

        control._bumped = Slide.atEnd(control.value, control.from, control.to, direction);
    }

    // TODO
    // Puts the handle at `target` without reporting it, for a value that came from elsewhere
    function seat(target: real) {
        control.value = Math.max(control.from, Math.min(control.to, target));
    }

    // TODO
    // Moves the handle one step from the step it is in
    function nudge(direction: int) {
        control.value = Slide.nudged(control.value, control.from, control.to, control.stepSize, direction);
        control.moved();
    }

    // TODO
    // Carries the handle `seconds` further, at the rate that crosses the whole track in
    // AppStyle.sliderTraverse whatever the range is
    function advance(seconds: real) {
        const delta = Slide.delta(control._direction, control.from, control.to, seconds, AppStyle.sliderTraverse);

        control.value = Math.max(control.from, Math.min(control.to, control.value + delta));
        control.moved();
        control.bumpAtEnd(control._direction);
    }

    // TODO
    // Stops the arrows reaching the slider's own key handling, which moves by a step per press
    Keys.priority: Keys.BeforeItem

    // TODO
    // Works the slider from a key and reports whether it did. Functions rather than handlers because the
    // row this sits in is what holds the focus, so the keys arrive there
    function pressKey(key: int, isAutoRepeat: bool): bool {
        if (key !== Qt.Key_Left && key !== Qt.Key_Right) {
            return false;
        }

        // TODO
        // A held key repeats; the slide runs off the frame clock, so the repeats are dropped
        if (isAutoRepeat) {
            return true;
        }

        const pressed = key === Qt.Key_Left ? -1 : 1;

        if (pressed === -1) {
            control._leftHeld = true;
        } else {
            control._rightHeld = true;
        }

        control._lastPressed = pressed;
        control._direction = pressed;
        control.nudge(pressed);
        control.bumpAtEnd(pressed);
        holdTimer.restart();
        return true;
    }

    function releaseKey(key: int, isAutoRepeat: bool): bool {
        if (key !== Qt.Key_Left && key !== Qt.Key_Right) {
            return false;
        }

        if (isAutoRepeat) {
            return true;
        }

        if (key === Qt.Key_Left) {
            control._leftHeld = false;
        } else {
            control._rightHeld = false;
        }

        const remaining = Slide.heldDirection(control._leftHeld, control._rightHeld, control._lastPressed);

        // TODO
        // The other arrow is still down, so the slide carries on its way rather than ending
        if (remaining !== 0) {
            control._direction = remaining;
            return true;
        }

        control.rest();
        return true;
    }

    Keys.onPressed: event => {
        event.accepted = control.pressKey(event.key, event.isAutoRepeat);
    }

    Keys.onReleased: event => {
        event.accepted = control.releaseKey(event.key, event.isAutoRepeat);
    }

    // TODO
    // Emitted when the handle stops moving, whether it was held, tapped or dragged
    signal settled

    // TODO
    // Ends a hold, leaving the handle where it stopped
    function rest() {
        holdTimer.stop();
        control._leftHeld = false;
        control._rightHeld = false;
        control._lastPressed = 0;
        control._bumped = false;
        control._sliding = false;
        control._direction = 0;
        control.settled();
    }

    onActiveFocusChanged: {
        if (!control.activeFocus) {
            control.rest();
        }
    }

    Timer {
        id: holdTimer
        interval: AppStyle.sliderHoldDelay
        onTriggered: control._sliding = true
    }

    FrameAnimation {
        running: control._sliding && control._direction !== 0
        onTriggered: control.advance(frameTime)
    }

    HoverHandler {
        id: sliderHover
        enabled: control.enabled
        cursorShape: Qt.PointingHandCursor
    }

    background: Rectangle {
        x: control.leftPadding
        y: control.topPadding + control.availableHeight / 2 - height / 2
        width: control.availableWidth
        height: Math.round(6 * AppStyle.scale)
        radius: height / 2
        color: Theme.backgroundInset

        Rectangle {
            width: control.visualPosition * parent.width
            height: parent.height
            radius: height / 2
            color: Theme.switch2Color
        }
    }

    handle: Rectangle {
        id: theHandle
        x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
        y: control.topPadding + control.availableHeight / 2 - height / 2
        // width: control.pressed ? Math.round(control._handle * 1.15) : control._handle
        width: control._handle
        height: width
        radius: width / 2
        color: Theme.textPrimary
        border.width: 1
        border.color: control._focusRing || control.pressed || sliderHover.hovered ? "transparent" : Theme.textMuted

        Rectangle {
            width: parent.width + 6
            height: parent.height + 6
            radius: width / 2
            anchors.centerIn: parent
            z: theHandle.z - 1
            color: "black"
            visible: control._focusRing
        }

        Behavior on width {
            NumberAnimation {
                duration: 80
            }
        }
    }
}
