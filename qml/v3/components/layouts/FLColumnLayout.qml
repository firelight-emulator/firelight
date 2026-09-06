import QtQuick
import QtQuick.Layouts
import Firelight 1.0

import "focus_nav.js" as Nav

ColumnLayout {
    id: root

    FLFocus.container: true
    focus: true

    property int currentIndex: 0

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

    function focusFirstChild(from: int) {
        const index = Nav.firstFocusable(root.children, from);

        if (index < 0) {
            return;
        }

        root.currentIndex = index;

        const child = root.children[index];

        if (child.enterFrom !== undefined) {
            child.enterFrom(1);
        } else {
            child.forceActiveFocus();
        }
    }

    Keys.onPressed: event => {
        if (event.accepted) {
            return;
        }

        if (event.key === Qt.Key_Up) {
            event.accepted = root.moveFocus(Nav.Up);
        } else if (event.key === Qt.Key_Down) {
            event.accepted = root.moveFocus(Nav.Down);
        }
    }

    Connections {
        target: root

        function onChildrenChanged() {
            if (root.activeFocus) {
                root.focusFirstChild(root.currentIndex);
            }
        }

        function onActiveFocusChanged() {
            if (root.activeFocus) {
                root.focusFirstChild(root.currentIndex);
            }
        }
    }
}
