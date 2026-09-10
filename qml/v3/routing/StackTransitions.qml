// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls
import Firelight 1.0

// TODO
// One StackView's live copies of the named transition presets, and the focus ring while they run.
// Instances are built once and kept: a StackView reads its transition properties when an operation
// starts, so the object has to outlive the animation it is running
QtObject {
    id: root

    required property StackView view

    // TODO
    // Whether moves on this stack hide the focus ring while they run
    property bool blinksCursor: true

    property var _enter: ({})
    property var _exit: ({})
    property bool _blinked: false

    // TODO
    // Puts a preset's animations on the stack and answers the operation to hand replaceCurrentItem.
    // The entering item is reset first: it may be a cached page still carrying the offset an earlier
    // move left on it
    function apply(name: string, item: Item): int {
        if (item !== null) {
            item.x = 0;
            item.y = 0;
            item.scale = 1;
            item.opacity = 1;
        }

        if (!PageTransitions.has(name)) {
            return StackView.Immediate;
        }

        if (root._enter[name] === undefined) {
            root._enter[name] = PageTransitions.createEnter(name, root.view);
            root._exit[name] = PageTransitions.createExit(name, root.view);
        }

        root.view.replaceEnter = root._enter[name];
        root.view.replaceExit = root._exit[name];

        return StackView.ReplaceTransition;
    }

    property Connections _busy: Connections {
        target: root.view

        function onBusyChanged() {
            if (root.view.busy) {
                root._blinked = root.blinksCursor;

                if (root._blinked) {
                    FocusCursor.startBlink();
                }

                return;
            }

            if (root._blinked) {
                root._blinked = false;
                FocusCursor.endBlink();
            }
        }
    }
}
