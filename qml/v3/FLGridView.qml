import QtQuick
import Firelight 1.0

import "focus_nav.js" as Nav

// TODO
// A grid the cursor navigates. Movement stays inside the grid, and a press that
// would leave it is refused rather than swallowed, so it can reach whatever sits
// beside the grid
//
//   FLGridView { model: games; delegate: GameTile {} }
GridView {
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
    // Qt's own key navigation wraps between rows and strands a short final row,
    // so moveFocus() replaces it entirely
    keyNavigationEnabled: false
    keyNavigationWraps: false

    // TODO
    // A held direction stops at the top and bottom rows rather than running out
    // of the grid; sideways it may leave as soon as the row ends
    FLFocus.holdEdges: FLFocus.Vertical

    // TODO
    // How long after a move a held direction is ignored, so a repeat that
    // arrives faster than this does not outrun the ring's scrolling
    property int repeatInterval: 60

    readonly property int columns: Math.max(1, Math.floor(width / Math.max(1, cellWidth)))

    // TODO
    // Moves the cursor within the grid, reporting whether it consumed the press.
    // Refusing is how a press reaches the rest of the screen
    function moveFocus(direction: int): bool {
        root.adoptFocusedIndex();

        const next = Nav.gridStep(root.currentIndex, root.count, root.columns, direction);

        if (next < 0 || next === root.currentIndex) {
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

    // TODO
    // Nothing links the current index to focus on its own, and an index that has
    // not been realized has no item to focus until the view is told to reach it
    function focusCurrentItem() {
        let item = root.itemAtIndex(root.currentIndex);

        if (item === null) {
            root.positionViewAtIndex(root.currentIndex, GridView.Contain);
            item = root.itemAtIndex(root.currentIndex);
        }

        if (item !== null) {
            item.forceActiveFocus();
        }
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

        // TODO
        // A held direction is consumed while the gate runs, so it neither moves
        // nor falls through to the edge bump
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
