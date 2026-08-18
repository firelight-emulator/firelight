import QtQuick
import QtQuick.Layouts
import Firelight 1.0

import "focus_nav.js" as Nav

// TODO
// A column whose children the cursor walks with Up and Down. Running out of
// children refuses the press rather than swallowing it, so it reaches whatever
// sits above or below the column
ColumnLayout {
    id: root

    // TODO
    // Answers for its children when a press asks what it can reach, so something
    // level with any part of this is level with the whole of it
    FLFocus.container: true
    focus: true

    // TODO
    // Where the cursor was last, so entering the column again returns to it
    // rather than always to the top
    property int currentIndex: 0

    // TODO
    // Moves the cursor within the column, reporting whether it consumed the
    // press. It starts from where focus actually is, not where this column last
    // put it — anything else may have moved focus since
    function moveFocus(direction: int): bool {
        if (direction !== Nav.Up && direction !== Nav.Down) {
            return false;
        }

        const step = direction === Nav.Up ? -1 : 1;
        const focused = Nav.focusedIndex(root.children);
        const from = focused >= 0 ? focused : root.currentIndex - step;
        const next = Nav.nextFocusable(root.children, from, step);

        if (next < 0) {
            return false;
        }

        root.currentIndex = next;
        const child = root.children[next];
        if (child.enterFrom !== undefined) {
            child.enterFrom(step);
        } else {
            child.forceActiveFocus();
        }

        return true;
    }

    // TODO
    // Entered directionally from the column above or below: land on the child at the matching end and
    // pass the direction on, so a nested column lands at that same end rather than always its top
    function enterFrom(step: int) {
        const index = step < 0 ? Nav.nextFocusable(root.children, root.children.length, -1) : Nav.nextFocusable(root.children, -1, 1);

        if (index < 0) {
            return;
        }

        root.currentIndex = index;
        const child = root.children[index];

        if (child.enterFrom !== undefined) {
            child.enterFrom(step);
        } else {
            child.forceActiveFocus();
        }
    }

    // TODO
    // Focus arriving on the column itself is handed to a child: the one the
    // cursor was last on, else the nearest that can take it
    function focusFirstChild(from: int) {
        const index = Nav.firstFocusable(root.children, from);

        if (index < 0) {
            return;
        }

        root.currentIndex = index;

        // TODO
        // A child that says how to be entered is entered at its top, rather than by forcing focus on
        // it and leaving Qt to choose a descendant
        const child = root.children[index];

        if (child.enterFrom !== undefined) {
            child.enterFrom(1);
        } else {
            child.forceActiveFocus();
        }
    }

    Keys.onPressed: event => {
        if (event.key === Qt.Key_Up) {
            event.accepted = root.moveFocus(Nav.Up);
        } else if (event.key === Qt.Key_Down) {
            event.accepted = root.moveFocus(Nav.Down);
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
