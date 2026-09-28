import QtQuick
import Firelight 1.0

FLGridView {
    id: root

    readonly property int liftOriginIndex: root._liftOriginIndex
    property int _liftOriginIndex: -1

    signal placed(int from, int to)
    signal cancelled

    function beginReorder(index: int) {
        root.lift(index);

        if (root.lifted) {
            root._liftOriginIndex = index;
        }
    }

    function place() {
        const from = root._liftOriginIndex;
        const to = root.liftedIndex;
        root._liftOriginIndex = -1;
        root.drop();
        root.placed(from, to);
    }

    function cancelReorder() {
        const origin = root._liftOriginIndex;

        if (root.liftedIndex !== origin) {
            root.moveRequested(root.liftedIndex, origin);
        }

        root._liftOriginIndex = -1;
        root.currentIndex = origin;
        root.drop();
        root.focusCurrentItem();
        root.cancelled();
    }

    // A lift the base view let go of on its own, when focus left, counts as placed where it is
    onLiftedChanged: {
        if (root.lifted || root._liftOriginIndex === -1) {
            return;
        }

        const from = root._liftOriginIndex;
        root._liftOriginIndex = -1;
        root.placed(from, root.currentIndex);
    }

    FLFocus.actions: [
        FLAction {
            keys: [Qt.Key_Back, Qt.Key_Escape]
            label: qsTr("Cancel")
            sound: SoundEffects.back
            enabled: root.lifted
            hidden: !root.lifted
            onTriggered: root.cancelReorder()
        }
    ]

    move: Transition {
        NumberAnimation {
            properties: "x,y"
            duration: AppStyle.durationBase
            easing.type: AppStyle.easingStandard
        }
    }

    displaced: Transition {
        NumberAnimation {
            properties: "x,y"
            duration: AppStyle.durationBase
            easing.type: AppStyle.easingStandard
        }
    }
}
