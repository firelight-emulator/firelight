import QtQuick
import QtQuick.Layouts

ColumnLayout {
    id: root
    focus: true

    property int currentIndex: 0

    // TODO
    // A child that can hold the cursor
    function isFocusable(child): bool {
        return child && child.visible && child.enabled && child.focusPolicy !== Qt.NoFocus;
    }

    // TODO
    // Where the cursor actually is, which is not necessarily where this layout
    // last put it — anything else may have moved focus since
    function focusedIndex(): int {
        for (let i = 0; i < root.children.length; i++) {
            if (root.children[i] && root.children[i].activeFocus) {
                return i;
            }
        }

        return root.currentIndex;
    }

    // TODO
    // Moves the cursor to the next focusable child in `step` direction, and
    // reports whether one was found
    function moveFocus(step: int): bool {
        let index = root.focusedIndex() + step;

        while (index >= 0 && index < root.children.length) {
            if (root.isFocusable(root.children[index])) {
                root.currentIndex = index;
                root.children[index].forceActiveFocus();
                return true;
            }

            index += step;
        }

        return false;
    }

    Keys.onPressed: function (event) {
        if (event.key === Qt.Key_Up) {
            event.accepted = root.moveFocus(-1);
        } else if (event.key === Qt.Key_Down) {
            event.accepted = root.moveFocus(1);
        }
    }

    // TODO
    // Focus arriving on the layout itself is handed to a child: the one the
    // cursor was last on, else the first that can take it
    function focusFirstChild(from: int) {
        for (let i = Math.max(0, from); i < root.children.length; i++) {
            if (root.isFocusable(root.children[i])) {
                root.currentIndex = i;
                root.children[i].forceActiveFocus();
                return;
            }
        }
    }

    onChildrenChanged: {
        if (activeFocus) {
            root.focusFirstChild(root.currentIndex);
        }
    }

    onActiveFocusChanged: {
        if (activeFocus) {
            root.focusFirstChild(root.currentIndex);
        }
    }
}