import QtQuick
import Firelight 1.0

import "focus_nav.js" as Nav

GridView {
    id: root

    FLFocus.container: true

    highlightFollowsCurrentItem: false

    keyNavigationEnabled: false
    keyNavigationWraps: false

    // populate: FLGridViewPopulateTransition {}

    // TODO
    // A held direction stops at the top and bottom rows rather than running out
    // of the grid; sideways it may leave as soon as the row ends
    FLFocus.holdEdges: FLFocus.Vertical

    property int repeatInterval: 45

    readonly property int columns: Math.max(1, Math.floor(width / Math.max(1, cellWidth)))

    function moveFocus(direction: int): bool {
        root.adoptFocusedIndex();

        const next = Nav.gridStep(root.currentIndex, root.count, root.columns, direction);

        if (next < 0 || next === root.currentIndex || next >= root.count) {
            return false;
        }

        root.currentIndex = next;
        root.focusCurrentItem();

        return true;
    }

    // TODO
    // Takes the current index from wherever the cursor actually is. A tap, or a
    // move in from outside the grid, lands on a delegate without going through
    // this view, and stepping from a stale index jumps the cursor somewhere else
    function adoptFocusedIndex() {
        const focused = Nav.focusedDelegate(root, root.Window.activeFocusItem);

        if (focused >= 0 && focused !== root.currentIndex) {
            root.currentIndex = focused;
        }
    }
    function focusCurrentItem() {
        // const index = root.currentIndex;
        // let item = root.itemAtIndex(index);
        //
        // if (item === null) {
        //     // TODO
        //     // Nothing to focus and nothing to scroll toward, so the view is moved to build one.
        //     // The cursor is told where it is going separately, so this does not decide the scroll
        //     if (index === 0) {0
        //         root.positionViewAtBeginning();
        //     } else {
        //         root.positionViewAtIndex(index, GridView.Contain);
        //     }
        //     root.forceLayout();
        //     item = root.itemAtIndex(index);
        // }
        //
        // if (item !== null) {
        //     item.forceActiveFocus();
        //     return;
        // }
        //
        // // TODO
        // // A delegate far enough away is built with a later layout rather than this one, so the
        // // focus waits for it. Dropping it here is what leaves the view moved and the cursor behind
        // Qt.callLater(function () {
        //     if (root.currentIndex !== index) {
        //         return;
        //     }
        //
        //     const built = root.itemAtIndex(index);
        //
        //     if (built !== null) {
        //         built.forceActiveFocus();
        //     }
        // });
    }

    Timer {
        id: repeatGate
        interval: root.repeatInterval
    }

    Keys.onPressed: event => {
        let direction = 0;

        if (event.key === Qt.Key_Up) {
            direction = Nav.Up;
        } else if (event.key === Qt.Key_Down) {
            direction = Nav.Down;
        } else if (event.key === Qt.Key_Left) {
            direction = Nav.Left;
        } else if (event.key === Qt.Key_Right) {
            direction = Nav.Right;
        } else {
            return;
        }

        if (event.isAutoRepeat) {
            if (repeatGate.running) {
                event.accepted = true;
                return;
            }

            repeatGate.restart();
        }

        event.accepted = root.moveFocus(direction);
    }
}
