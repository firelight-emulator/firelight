pragma Singleton
import QtQuick

// TODO
// App-wide handle on the focus ring, so anything can drive the cursor without
// reaching for the FLFocusHighlight instance. Main4 registers the instance once
//
//   FocusCursor.blink(AppStyle.durationBase)   // hide, restore after a beat
//   FocusCursor.startBlink() ... endBlink()    // hide across an unknown span
QtObject {
    id: root

    // TODO
    // The registered FLFocusHighlight, or null before Main4 registers it
    readonly property Item highlight: _highlight
    property Item _highlight: null

    readonly property bool blinking: _highlight !== null && _highlight.blinking

    // TODO
    // Called once by the window that owns the ring
    function register(instance: Item) {
        root._highlight = instance;
    }

    // TODO
    // Hides the ring for `duration` ms, or durationBase when none is given; it
    // returns on whatever holds focus by then
    function blink(duration: int) {
        if (root._highlight !== null) {
            root._highlight.blink(duration);
        }
    }

    // TODO
    // Hides the ring until endBlink()
    function startBlink() {
        if (root._highlight !== null) {
            root._highlight.startBlink();
        }
    }

    // TODO
    // Brings the ring back on whatever holds focus now
    function endBlink() {
        if (root._highlight !== null) {
            root._highlight.endBlink();
        }
    }
}
