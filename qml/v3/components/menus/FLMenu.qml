import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

FLPopup {
    id: control

    default property alias content: column.data
    overlap: -AppStyle.spacingXs

    closePolicy: Popup.CloseOnPressOutsideParent
    clip: true

    modal: !InputMethodManager.usingMouse

    // TODO
    // A row owning a submenu has to register it, or Qt treats the two menus as
    // unrelated and closes this one when that one opens. Rows that need it say
    // so, rather than each call site wiring it
    function adoptRows() {
        for (let i = 0; i < column.children.length; i++) {
            const child = column.children[i];

            if (child.registerIn !== undefined) {
                child.registerIn(control);
            }
        }
    }

    Component.onCompleted: control.adoptRows()

    // TODO
    // A submenu left open when this closes is restored on the next open, opening
    // two menus for one press. Rows shut theirs on the way out
    onClosed: {
        for (let i = 0; i < column.children.length; i++) {
            const child = column.children[i];

            if (child.closeSubmenu !== undefined) {
                child.closeSubmenu();
            }
        }

        contentFlickable.contentY = 0;
    }

    // TODO
    // A Connections rather than onChildrenChanged: the layout declares its own
    // handler and an instance one would replace it
    Connections {
        target: column

        function onChildrenChanged() {
            control.adoptRows();
        }
    }

    function focusFirstChild() {
        column.focusFirstChild(-1);
    }

    contentItem: Flickable {
        id: contentFlickable
        implicitWidth: column.implicitWidth
        implicitHeight: Math.min(600, column.implicitHeight)
        contentHeight: column.implicitHeight

        onActiveFocusChanged: {
            if (activeFocus) {
                control.focusFirstChild();
            }
        }

        Keys.onPressed: event => {
            if (event.key === Qt.Key_Back || event.key === Qt.Key_Escape) {
                event.accepted = true;
                SoundEffects.back.play();
                control.close();
            }
        }

        FLColumnLayout {
            id: column
            anchors.fill: parent
            spacing: AppStyle.spacingXs
            focus: true
        }
    }
}
