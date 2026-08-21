// TODO: NEEDS REVIEW
import QtQuick
import QtTest
import Firelight 1.0

// Modifiers reaching an action through a real key press rather than a direct lookup. The item below
// is the shape every dispatcher in the app uses: hand the pressed key and the modifiers held with it
// to getActionFor, and run whatever answers
TestCase {
    id: testCase
    name: "ActionModifiers"

    width: 200
    height: 200

    visible: true
    when: windowShown

    property int played: 0
    property int rangeSelected: 0

    Item {
        id: target

        anchors.fill: parent
        focusPolicy: Qt.StrongFocus

        FLFocus.actions: [
            FLAction {
                keys: [Qt.Key_Return, Qt.Key_Enter]
                label: "Play"
                onTriggered: testCase.played++
            },
            FLAction {
                keys: [Qt.Key_Return]
                modifiers: Qt.ShiftModifier
                label: "Select range"
                onTriggered: testCase.rangeSelected++
            }
        ]

        Keys.onPressed: event => {
            const action = target.FLFocus.getActionFor(event.key, event.modifiers);

            if (action !== null) {
                action.triggered();
                event.accepted = true;
            }
        }
    }

    function init() {
        testCase.played = 0;
        testCase.rangeSelected = 0;
        target.forceActiveFocus();
    }

    function test_a_bare_press_reaches_the_bare_action() {
        keyClick(Qt.Key_Return);

        compare(testCase.played, 1);
        compare(testCase.rangeSelected, 0);
    }

    function test_a_modified_press_reaches_the_modified_action() {
        keyClick(Qt.Key_Return, Qt.ShiftModifier);

        compare(testCase.rangeSelected, 1);
        compare(testCase.played, 0);
    }

    // Nothing declared this combination, so the press goes on to whatever is behind the item
    function test_a_modifier_no_action_declared_reaches_nothing() {
        keyClick(Qt.Key_Return, Qt.ControlModifier);

        compare(testCase.played, 0);
        compare(testCase.rangeSelected, 0);
    }

    // The declaration says Shift, not Shift and Control
    function test_extra_modifiers_are_not_close_enough() {
        keyClick(Qt.Key_Return, Qt.ShiftModifier | Qt.ControlModifier);

        compare(testCase.rangeSelected, 0);
        compare(testCase.played, 0);
    }
}
