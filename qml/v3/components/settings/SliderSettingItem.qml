// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

import "slider_slide.js" as Slide

FLMenuItem {
    id: root

    controlBelow: true

    property real from: 0
    property real to: 10
    property real stepSize: 1
    property alias value: theControl.value

    // TODO
    // What the setting holds. The handle follows it only once the two disagree by more than half a
    // step, so a value written back does not drag the handle onto it
    property real storedValue: 0

    property bool _ready: false

    signal moved(real value)

    // TODO
    // Where the handle is now, reported on every change and never written anywhere, so anything
    // wanting to show the value as it moves can follow without paying for a write
    signal slid(real value)

    // TODO
    // The handle has stopped and `moved` has had its say, so a preview can hand back to the setting
    signal settled

    Component.onCompleted: {
        theControl.seat(root.storedValue);
        root._ready = true;
    }

    onStoredValueChanged: {
        if (!root._ready) {
            return;
        }

        const shown = Slide.stepped(theControl.value, root.from, root.to, root.stepSize);

        if (Slide.shouldReseat(root.storedValue, shown, root.stepSize)) {
            theControl.seat(root.storedValue);
        }
    }

    // TODO
    // Writing a setting is a database write and a relayout of everything bound to it, so the handle
    // moves freely and only what it lands on is written
    function commit() {
        if (!root._ready) {
            return;
        }

        // TODO
        // Writing back what the setting already holds would record an override where the value was
        // only ever inherited
        if (theControl.shownValue === root.storedValue) {
            return;
        }

        root.moved(theControl.shownValue);
    }

    // TODO
    // Writes what the handle landed on, then releases whatever was following it
    function finish() {
        root.commit();
        root.settled();
    }

    // TODO
    // The row holds the focus and the cursor draws on the handle, so there is one stop here rather than
    // a row and a slider the cursor visits in turn
    FLFocus.proxy: theControl.handle

    controlItem: RowLayout {
        spacing: AppStyle.spacingMd

        FLSlider {
            id: theControl
            Layout.fillWidth: true
            enabled: root.enabled
            focusPolicy: Qt.NoFocus
            from: root.from
            to: root.to
            stepSize: root.stepSize
            snapMode: Slider.SnapOnRelease

            FLFocus.focusSound: SoundEffects.menuNavigate

            // TODO
            // A drag reports as it happens rather than only on release, so a preview can follow it
            live: true

            readonly property real shownValue: Slide.stepped(theControl.value, root.from, root.to, root.stepSize)

            onShownValueChanged: {
                if (root._ready) {
                    root.slid(theControl.shownValue);
                }
            }

            // TODO
            // The handle has stopped: a hold ended, a tap finished, or a drag was let go
            onSettled: root.finish()

            // TODO
            // The row's own tap never arrives here: the slider takes the press for dragging, which
            // cancels it. Pressing the slider is the click, and the slider is what should end up
            // focused, so it takes focus itself rather than by way of the row
            onPressedChanged: {
                if (theControl.pressed) {
                    theControl.forceActiveFocus(Qt.MouseFocusReason);
                    return;
                }

                root.finish();
            }
        }

        Text {
            // Fractional sliders (e.g. tint at step 0.01) need decimals; integer
            // sliders (volume, tile size) shouldn't read "100.00"
            Layout.preferredWidth: Math.round(52 * AppStyle.scale)
            text: theControl.shownValue.toFixed(root.stepSize < 1 ? 2 : 0)
            color: Theme.textPrimary
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: AppStyle.fontSizeMedium
            font.family: AppStyle.fontFamily
            font.weight: Font.DemiBold
        }
    }

    // TODO
    // TODO
    // The arrows arrive at the row, so the row is what works the slider
    Keys.onPressed: event => {
        event.accepted = theControl.pressKey(event.key, event.isAutoRepeat);
    }

    Keys.onReleased: event => {
        event.accepted = theControl.releaseKey(event.key, event.isAutoRepeat);
    }

    onActiveFocusChanged: {
        if (!root.activeFocus) {
            theControl.rest();
        }
    }
}
