import QtQuick
import QtQuick.Layouts
import Firelight 1.0

import "focus_nav.js" as Nav

// TODO
// A row whose children the cursor walks with Left and Right. The mirror of
// FLColumnLayout, and refuses at its ends the same way
RowLayout {
    id: root

    // TODO
    // Answers for its children when a press asks what it can reach, so something
    // level with any part of this is level with the whole of it
    FLFocus.container: true
    focus: true

    // TODO
    // Where the cursor was last, so entering the row again returns to it rather
    // than always to the start
    property int currentIndex: 0

    // TODO
    // Moves the cursor within the row, reporting whether it consumed the press
    function moveFocus(direction: int): bool {
        if (direction !== Nav.Left && direction !== Nav.Right) {
            return false;
        }

        const step = direction === Nav.Left ? -1 : 1;
        const focused = Nav.focusedIndex(root.children);
        const from = focused >= 0 ? focused : root.currentIndex - step;
        const next = Nav.nextFocusable(root.children, from, step);

        if (next < 0) {
            return false;
        }

        root.currentIndex = next;
        root.children[next].forceActiveFocus();

        return true;
    }

    // TODO
    // Focus arriving on the row itself is handed to a child: the one the cursor
    // was last on, else the nearest that can take it
    function focusFirstChild(from: int) {
        const index = Nav.firstFocusable(root.children, from);

        if (index < 0) {
            return;
        }

        root.currentIndex = index;
        root.children[index].forceActiveFocus();
    }

    Keys.onPressed: event => {
        if (event.key === Qt.Key_Left) {
            event.accepted = root.moveFocus(Nav.Left);
        } else if (event.key === Qt.Key_Right) {
            event.accepted = root.moveFocus(Nav.Right);
        }
    }

    onChildrenChanged: {
        if (root.activeFocus) {
            root.focusFirstChild(root.currentIndex);
        }
    }

    onActiveFocusChanged: {
        if (root.activeFocus) {
            root.focusFirstChild(root.currentIndex);
        }
    }
}
