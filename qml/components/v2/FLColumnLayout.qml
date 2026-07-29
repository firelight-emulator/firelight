import QtQuick
import QtQuick.Layouts

ColumnLayout {
    id: root
    focus: true

    property int currentIndex: 0

    Keys.onPressed: function(event) {
        console.log("Key pressed: " + event.key + ", currentIndex: " + root.currentIndex + ", current child: " + (root.children[root.currentIndex] ? root.children[root.currentIndex] : "none"));
        console.log("all children: " + root.children.length);
        if (event.key === Qt.Key_Up) {
            let newIndex = root.currentIndex - 1;
            if (newIndex < 0) {
                return;
            }

            let found = false;
            while (!found) {
                if (newIndex < 0) {
                    return;
                }
                if (root.children && root.children[newIndex] && root.children[newIndex].visible && root.children[newIndex].enabled && root.children[newIndex].focusPolicy !== Qt.NoFocus) {
                    found = true;
                } else {
                    newIndex--;
                }
            }

            root.currentIndex = newIndex;
            root.children[newIndex].forceActiveFocus();
            event.accepted = true;
        } else if (event.key === Qt.Key_Down) {
            let newIndex = root.currentIndex + 1;
            if (newIndex >= root.children.length) {
                return;
            }

            let found = false;
            while (!found) {
                if (newIndex >= root.children.length) {
                    return;
                }
                if (root.children && root.children[newIndex] && root.children[newIndex].visible && root.children[newIndex].enabled && root.children[newIndex].focusPolicy !== Qt.NoFocus) {
                    found = true;
                } else {
                    newIndex++;
                }
            }

            root.currentIndex = newIndex;
            root.children[newIndex].forceActiveFocus();
            event.accepted = true;
        }
    }

    onChildrenChanged: {
        if (activeFocus) {
            for (let i = currentIndex; i < root.children.length; i++) {
                if (root.children[i].visible && root.children[i].enabled && root.children[i].focusPolicy !== Qt.NoFocus) {
                    root.children[i].forceActiveFocus();
                    currentIndex = i;
                    break;
                }
            }
        }
    }

    onActiveFocusChanged: {
        if (activeFocus) {
            for (let i = 0; i < root.children.length; i++) {
                if (root.children[i].visible && root.children[i].enabled && root.children[i].focusPolicy !== Qt.NoFocus) {
                    root.children[i].forceActiveFocus();
                    currentIndex = i;
                    break;
                }
            }
        }
    }
}