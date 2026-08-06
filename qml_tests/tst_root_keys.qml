import QtQuick
import QtQuick.Controls
import QtTest

// Where a key ends up when nothing below claims it. One handler on the window's
// root item covers the whole content tree; popups are separate, because Qt stops
// their keys at their own popup item so they cannot reach what is behind.
TestCase {
    id: testCase

    name: "RootKeys"
    when: windowShown
    width: 300
    height: 200
    visible: true

    property int rootSaw: 0
    property int childSaw: 0
    property bool childConsumes: false
    property int popSaw: 0
    property bool rootAccepts: false
    property int lastKey: 0
    property int popItemSaw: 0

    Item {
        id: child

        objectName: "child"
        width: 40
        height: 40
        focusPolicy: Qt.StrongFocus

        Keys.onPressed: event => {
            testCase.childSaw++;
            event.accepted = testCase.childConsumes;
        }
    }

    function initTestCase() {
        // Attaching Keys to an item this file did not declare, from script
        testCase.Window.window.contentItem.Keys.pressed.connect(function (event) {
            testCase.rootSaw++;
            testCase.lastKey = event.key;

            if (testCase.rootAccepts) {
                event.accepted = true;
            }
        });
    }

    function init() {
        testCase.rootSaw = 0;
        testCase.childSaw = 0;
        testCase.childConsumes = false;
        testCase.popSaw = 0;
        testCase.popItemSaw = 0;
        testCase.rootAccepts = false;
        testCase.lastKey = 0;
    }

    Popup {
        id: pop

        width: 100
        height: 100
        focus: true

        contentItem: FocusScope {
            Item {
                id: popChild

                objectName: "popChild"
                width: 40
                height: 40
                focusPolicy: Qt.StrongFocus

                Keys.onPressed: event => {
                    testCase.popSaw++;
                    event.accepted = false;
                }
            }
        }
    }

    function test_the_root_item_sees_a_key_the_focused_item_declined() {
        child.forceActiveFocus();
        verify(child.activeFocus);

        keyPress(Qt.Key_Down);

        compare(testCase.childSaw, 1);
        compare(testCase.rootSaw, 1, "the root item should have been reached");
    }

    // Qt stops a popup's keys at its own popup item so they cannot leak to the
    // content behind, so one handler on the root item cannot cover popups. The
    // popup item is reachable though, and sees the key before the popup takes it
    function test_a_popups_keys_stop_at_its_popup_item() {
        pop.open();
        tryVerify(function () {
            return pop.opened;
        });

        pop.contentItem.parent.Keys.pressed.connect(function (event) {
            testCase.popItemSaw++;
        });

        popChild.forceActiveFocus();
        verify(popChild.activeFocus, "focus should be inside the popup");

        keyPress(Qt.Key_Down);

        compare(testCase.popSaw, 1, "the focused item sees it");
        compare(testCase.popItemSaw, 1, "and so does the popup item, which is where a handler goes");
        compare(testCase.rootSaw, 0, "but it never reaches the root item");

        pop.close();
        tryVerify(function () {
            return !pop.visible;
        });
    }

    // Whether the root handler marks the event handled should not matter: it is
    // already the last thing in the chain
    function test_the_root_handler_can_read_and_accept_the_event() {
        child.forceActiveFocus();
        testCase.rootAccepts = true;

        keyPress(Qt.Key_Down);

        compare(testCase.rootSaw, 1);
        compare(testCase.lastKey, Qt.Key_Down, "the handler should see which key it was");
    }

    function test_the_root_item_does_not_see_a_key_something_below_took() {
        child.forceActiveFocus();
        testCase.childConsumes = true;

        keyPress(Qt.Key_Down);

        compare(testCase.childSaw, 1);
        compare(testCase.rootSaw, 0, "a handled key should not reach the root item");
    }
}
