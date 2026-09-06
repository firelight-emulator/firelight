import QtQuick
import Firelight 1.0

import "focus_nav.js" as Nav

GridView {
    id: root

    FLFocus.container: true

    focus: true

    onActiveFocusChanged: {
        if (!root.activeFocus) {
            root._liftedIndex = -1;
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

    readonly property int liftedIndex: root._liftedIndex
    readonly property bool lifted: root._liftedIndex !== -1

    property int _liftedIndex: -1

    function lift(index: int) {
        if (index < 0 || index >= root.count) {
            return;
        }

        root._liftedIndex = index;
        root.currentIndex = index;
    }

    function drop() {
        root._liftedIndex = -1;
    }

    readonly property bool canMoveLeft: root.lifted && Nav.gridStep(root.liftedIndex, root.count, root.columns, Nav.Left) >= 0
    readonly property bool canMoveRight: root.lifted && Nav.gridStep(root.liftedIndex, root.count, root.columns, Nav.Right) >= 0
    readonly property bool canMoveUp: root.lifted && Nav.gridStep(root.liftedIndex, root.count, root.columns, Nav.Up) >= 0
    readonly property bool canMoveDown: root.lifted && Nav.gridStep(root.liftedIndex, root.count, root.columns, Nav.Down) >= 0

    signal moveRequested(int from, int to)

    Item {
        id: reorderArrows
        x: root.currentItem?.x ?? 0
        y: root.currentItem?.y ?? 0
        width: root.currentItem?.width ?? 0
        height: root.currentItem?.height ?? 0
        visible: root.lifted

        transform: Translate {
            y: root.lifted ? -AppStyle.reorderingLiftHeight : 0

            Behavior on y {
                NumberAnimation {
                    duration: AppStyle.durationFast
                    easing.type: Easing.Linear
                }
            }
        }

        readonly property var _arrowColor: Theme.switch2Color
        readonly property var _arrowSize: Math.min(width, height) * 0.24
        readonly property var _arrowSpacing: AppStyle.spacingSm

        Icon {
            id: leftArrow
            anchors.right: parent.left
            anchors.rightMargin: parent._arrowSpacing
            anchors.verticalCenter: parent.verticalCenter
            visible: root.canMoveLeft

            name: "arrow_back_2"
            filled: true
            size: parent._arrowSize
            color: parent._arrowColor
        }

        Icon {
            id: rightArrow
            anchors.left: parent.right
            anchors.leftMargin: parent._arrowSpacing
            anchors.verticalCenter: parent.verticalCenter
            visible: root.canMoveRight

            name: "arrow_back_2"
            filled: true
            size: parent._arrowSize
            color: parent._arrowColor
            rotation: 180
        }

        Icon {
            id: upArrow
            anchors.bottom: parent.top
            anchors.bottomMargin: parent._arrowSpacing
            anchors.horizontalCenter: parent.horizontalCenter
            visible: root.canMoveUp

            name: "arrow_back_2"
            filled: true
            size: parent._arrowSize
            color: parent._arrowColor
            rotation: 90
        }

        Icon {
            id: downArrow
            anchors.top: parent.bottom
            anchors.topMargin: parent._arrowSpacing
            anchors.horizontalCenter: parent.horizontalCenter
            visible: root.canMoveDown

            name: "arrow_back_2"
            filled: true
            size: parent._arrowSize
            color: parent._arrowColor
            rotation: 270
        }
    }

    function focusFirstItem() {
        root.currentIndex = 0;
        root.focusCurrentItem();
    }

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

    function resetCursor() {
        root._liftedIndex = -1;
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
            if (index === 0) {
                root.positionViewAtBeginning();
            } else {
                root.positionViewAtIndex(index, GridView.Contain);
            }
            root.forceLayout();
            item = root.itemAtIndex(index);
        }

        if (item !== null) {
            item.forceActiveFocus();
            return;
        }

        // Do later in case the delegate isn't built yet
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

        if (root.lifted) {
            const to = Nav.gridStep(root._liftedIndex, root.count, root.columns, direction);

            if (to >= 0) {
                root.moveRequested(root._liftedIndex, to);
                root._liftedIndex = to;
                root.currentIndex = to;
                root.focusCurrentItem();
            }

            event.accepted = true;
            return;
        }

        event.accepted = root.moveFocus(direction);
    }
}
