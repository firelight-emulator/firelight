import QtQuick
import Firelight 1.0

import "focus_nav.js" as Nav

// TODO
// A vertical list the cursor navigates. Same contract as FLGridView: movement
// stays inside the list, and a press that would leave it is refused so it can
// reach whatever sits beside the list
//
//   FLListView { model: games; delegate: GameListViewItem {} }
ListView {
    id: root

    // TODO
    // Answers for its children when a press asks what it can reach, so something
    // level with any part of this is level with the whole of it
    FLFocus.container: true

    // TODO
    // The ring drives scrolling with its own margins, so the view must not also
    // chase the current item — the two would fight over contentY
    highlightFollowsCurrentItem: false

    // TODO
    // Qt's own key navigation swallows the presses at both ends, which would
    // strand the cursor in the list
    keyNavigationEnabled: false
    keyNavigationWraps: false

    // TODO
    // A held direction stops at the first and last row rather than running out
    // of the list
    FLFocus.holdEdges: FLFocus.Vertical

    // TODO
    // How long after a move a held direction is ignored, so a repeat that
    // arrives faster than this does not outrun the ring's scrolling
    property int repeatInterval: 60

    // TODO
    // Moves the cursor within the list, reporting whether it consumed the press
    function moveFocus(direction: int): bool {
        root.adoptFocusedIndex();

        const next = Nav.gridStep(root.currentIndex, root.count, 1, direction);

        if (next < 0 || next === root.currentIndex) {
            return false;
        }

        root.currentIndex = next;
        root.focusCurrentItem();

        return true;
    }

    // TODO
    // Takes the current index from wherever the cursor actually is. A tap, or a
    // move in from outside the list, lands on a delegate without going through
    // this view, and stepping from a stale index jumps the cursor somewhere else
    function adoptFocusedIndex() {
        const focused = Nav.focusedDelegate(root, root.Window.activeFocusItem);

        if (focused >= 0 && focused !== root.currentIndex) {
            root.currentIndex = focused;
        }
    }

    // TODO
    // Nothing links the current index to focus on its own, and an index that has
    // not been realized has no item to focus until the view is told to reach it
    function focusCurrentItem() {
        const index = root.currentIndex;
        let item = root.itemAtIndex(index);

        if (item === null) {
            // TODO
            // Nothing to focus and nothing to scroll toward, so the view is moved to build one.
            // The cursor is told where it is going separately, so this does not decide the scroll
            root.positionViewAtIndex(index, ListView.Contain);
            root.forceLayout();
            item = root.itemAtIndex(index);
        }

        if (item !== null) {
            item.forceActiveFocus();
            return;
        }

        // TODO
        // A delegate far enough away is built with the next layout rather than this one, so the
        // focus waits for it. Dropping it here is what leaves the view moved and the cursor behind
        Qt.callLater(function () {
            if (root.currentIndex !== index) {
                return;
            }

            const built = root.itemAtIndex(index);

            if (built !== null) {
                built.forceActiveFocus();
            }
        });
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
