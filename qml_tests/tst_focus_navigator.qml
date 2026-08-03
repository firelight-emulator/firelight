import QtQuick
import QtTest
import Firelight 1.0

// Directional movement against a live item tree, which is the half the C++ tests
// cannot reach: the singleton being registered, the attached metadata being read
// off real QML, and focus actually landing
TestCase {
    id: testCase

    name: "FocusNavigator"
    when: windowShown
    width: 400
    height: 300
    visible: true

    function landOn(item) {
        item.forceActiveFocus();
        verify(item.activeFocus, "could not put focus on " + item.objectName);
    }

    function press(from, key) {
        return FocusNavigator.move(from, key, false);
    }

    // A held direction is thinned to a steady cadence, so a test has to wait out
    // the last move or it is answered without anything happening
    function hold(from, key) {
        wait(80);
        return FocusNavigator.move(from, key, true);
    }

    Item {
        id: scene

        anchors.fill: parent

        Item {
            id: toolbar

            objectName: "toolbar"
            x: 0
            y: 0
            width: 40
            height: 300

            Item {
                id: toolTop

                objectName: "toolTop"
                width: 40
                height: 40
                focusPolicy: Qt.StrongFocus
            }

            Item {
                id: toolBottom

                objectName: "toolBottom"
                y: 60
                width: 40
                height: 40
                focusPolicy: Qt.StrongFocus
            }
        }

        Item {
            id: grid

            objectName: "grid"
            x: 60
            y: 0
            width: 300
            height: 300

            FLFocus.holdEdges: FLFocus.Vertical

            Item {
                id: tileTopLeft

                objectName: "tileTopLeft"
                width: 100
                height: 100
                focusPolicy: Qt.StrongFocus
            }

            Item {
                id: tileTopRight

                objectName: "tileTopRight"
                x: 110
                width: 100
                height: 100
                focusPolicy: Qt.StrongFocus
            }

            Item {
                id: tileBottomLeft

                objectName: "tileBottomLeft"
                y: 110
                width: 100
                height: 100
                focusPolicy: Qt.StrongFocus
            }

            Item {
                id: tileBottomRight

                objectName: "tileBottomRight"
                x: 110
                y: 110
                width: 100
                height: 100
                focusPolicy: Qt.StrongFocus
            }
        }
    }

    function test_moves_to_what_lies_that_way() {
        landOn(tileTopLeft);

        compare(press(tileTopLeft, Qt.Key_Right), FocusNavigator.Moved);
        verify(tileTopRight.activeFocus);

        compare(press(tileTopRight, Qt.Key_Down), FocusNavigator.Moved);
        verify(tileBottomRight.activeFocus, "Down stays in its own column");
    }

    // The whole point of the layer: neither container knows the other exists
    function test_crosses_from_the_grid_to_the_toolbar() {
        landOn(tileTopLeft);

        compare(press(tileTopLeft, Qt.Key_Left), FocusNavigator.Moved);
        verify(toolTop.activeFocus);

        compare(press(toolTop, Qt.Key_Right), FocusNavigator.Moved);
        verify(tileTopLeft.activeFocus);
    }

    function test_nothing_that_way_is_no_target() {
        landOn(toolTop);
        compare(press(toolTop, Qt.Key_Up), FocusNavigator.NoTarget);
        verify(toolTop.activeFocus);
    }

    function test_a_key_that_is_not_a_direction_does_nothing() {
        landOn(tileTopLeft);
        compare(press(tileTopLeft, Qt.Key_Select), FocusNavigator.NoTarget);
        verify(tileTopLeft.activeFocus);
    }

    // Declared on the grid in QML, read in C++: a held direction stops at the
    // edge it declared, and pressing again crosses
    function test_a_held_direction_stops_at_a_declared_edge() {
        landOn(tileTopLeft);

        compare(hold(tileTopLeft, Qt.Key_Left), FocusNavigator.Moved);
        verify(toolTop.activeFocus, "sideways is not held, so a hold crosses");

        landOn(tileBottomLeft);
        compare(hold(tileBottomLeft, Qt.Key_Up), FocusNavigator.Moved);
        verify(tileTopLeft.activeFocus, "moving inside the grid is not leaving it");
    }

    function test_a_held_direction_may_not_leave_the_grid() {
        landOn(tileTopLeft);
        grid.FLFocus.holdEdges = FLFocus.AllEdges;

        compare(hold(tileTopLeft, Qt.Key_Left), FocusNavigator.NoTarget);
        verify(tileTopLeft.activeFocus);

        compare(press(tileTopLeft, Qt.Key_Left), FocusNavigator.Moved);
        verify(toolTop.activeFocus, "pressing again crosses the edge a hold stopped at");

        grid.FLFocus.holdEdges = FLFocus.Vertical;
    }

    // Thinning must never read as having run out of room, or a held direction
    // bumps the ring every time it is thinned
    function test_a_thinned_hold_reports_a_move_without_making_one() {
        landOn(tileTopLeft);
        compare(hold(tileTopLeft, Qt.Key_Down), FocusNavigator.Moved);
        verify(tileBottomLeft.activeFocus);

        // Nothing lies below, so an unthinned press here would report NoTarget
        compare(FocusNavigator.move(tileBottomLeft, Qt.Key_Down, true), FocusNavigator.Moved);
        verify(tileBottomLeft.activeFocus, "the thinned press did not move anything");
    }

    // What keeps arrows inside an open popup
    function test_a_barrier_confines_the_press() {
        landOn(tileTopLeft);
        grid.FLFocus.barrier = true;

        compare(press(tileTopLeft, Qt.Key_Left), FocusNavigator.NoTarget);
        verify(tileTopLeft.activeFocus);

        compare(press(tileTopLeft, Qt.Key_Right), FocusNavigator.Moved);
        verify(tileTopRight.activeFocus, "movement inside the barrier still works");

        grid.FLFocus.barrier = false;
    }

    function test_something_marked_skip_is_not_a_target() {
        landOn(tileTopLeft);
        tileTopRight.FLFocus.mode = FLFocus.Skip;

        compare(press(tileTopLeft, Qt.Key_Right), FocusNavigator.NoTarget);

        tileTopRight.FLFocus.mode = FLFocus.Normal;
        compare(press(tileTopLeft, Qt.Key_Right), FocusNavigator.Moved);
        verify(tileTopRight.activeFocus);
    }

    function test_something_hidden_is_not_a_target() {
        landOn(tileTopLeft);
        tileTopRight.visible = false;

        compare(press(tileTopLeft, Qt.Key_Right), FocusNavigator.NoTarget);

        tileTopRight.visible = true;
        compare(press(tileTopLeft, Qt.Key_Right), FocusNavigator.Moved);
    }
}
