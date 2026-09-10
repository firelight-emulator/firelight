// TODO: NEEDS REVIEW
import QtQuick
import Firelight 1.0

import "focus_nav.js" as Nav

ListView {
    id: root

    FLFocus.container: true

    focus: true

    onActiveFocusChanged: {
        if (!root.activeFocus) {
            return;
        }

        root.adoptFocusedIndex();

        if (Nav.focusedDelegate(root, root.Window.activeFocusItem) < 0) {
            root.focusCurrentItem();
        }
    }

    boundsBehavior: Flickable.StopAtBounds

    highlightFollowsCurrentItem: false

    keyNavigationEnabled: false
    keyNavigationWraps: false

    FLFocus.holdEdges: FLFocus.Vertical

    // TODO
    // How long a held direction waits between moves. Declared where every other region declares
    // it, though this view paces itself rather than going through the navigator
    FLFocus.repeatInterval: 45
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
    function resetCursor() {
        root.currentIndex = 0;
        root.positionViewAtBeginning();
    }

    function adoptFocusedIndex() {
        const focused = Nav.focusedDelegate(root, root.Window.activeFocusItem);

        if (focused >= 0 && focused !== root.currentIndex) {
            root.currentIndex = focused;
        }
    }
    function focusCurrentItem() {
        if (root.count === 0) {
            return;
        }

        const index = Math.min(Math.max(root.currentIndex, 0), root.count - 1);
        root.currentIndex = index;
        let item = root.itemAtIndex(index);

        if (item === null) {
            root.positionViewAtIndex(index, ListView.Contain);
            root.forceLayout();
            item = root.itemAtIndex(index);
        }

        if (item !== null) {
            item.forceActiveFocus();
            return;
        }

        // Called later in case the item isn't built yet
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
        interval: root.FLFocus.repeatInterval
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
