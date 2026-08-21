import QtQuick
import QtTest
import Firelight 1.0

// The way back out of a container, which geometry cannot answer on its own: a
// short button entering a tall tile leaves from the tile's middle, and the button
// nearest that middle is not the one it came from
TestCase {
    id: testCase

    name: "FocusReturn"
    when: windowShown
    width: 400
    height: 500
    visible: true

    function landOn(item) {
        item.forceActiveFocus();
        verify(item.activeFocus, "could not put focus on " + item.objectName);
    }

    function press(from, key) {
        return FocusNavigator.move(from, key, false);
    }

    Item {
        id: scene

        anchors.fill: parent

        Item {
            id: railTop

            objectName: "railTop"
            width: 40
            height: 40
            focusPolicy: Qt.StrongFocus
        }

        Item {
            id: railMiddle

            objectName: "railMiddle"
            y: 60
            width: 40
            height: 40
            focusPolicy: Qt.StrongFocus
        }

        // Level with the lower tile, since a press only reaches what shares its lane
        Item {
            id: railBottom

            objectName: "railBottom"
            y: 240
            width: 40
            height: 40
            focusPolicy: Qt.StrongFocus
        }

        Item {
            id: tile

            objectName: "tile"
            x: 60
            width: 200
            height: 200
            focusPolicy: Qt.StrongFocus
        }

        Item {
            id: lowerTile

            objectName: "lowerTile"
            x: 60
            y: 210
            width: 200
            height: 200
            focusPolicy: Qt.StrongFocus
        }
    }

    // What the layout does without any memory, which the tests below have to differ from
    function test_geometry_alone_lands_nearest_the_tiles_middle() {
        landOn(tile);
        compare(press(tile, Qt.Key_Left), FocusNavigator.Moved);
        verify(railMiddle.activeFocus);
    }

    function test_reversing_returns_to_where_it_started() {
        landOn(railTop);

        compare(press(railTop, Qt.Key_Right), FocusNavigator.Moved);
        verify(tile.activeFocus);

        compare(press(tile, Qt.Key_Left), FocusNavigator.Moved);
        verify(railTop.activeFocus, "went to the nearest button instead of the one it came from");
    }

    // The way back is only good while it is still true
    function test_moving_on_forgets_the_way_back() {
        landOn(railTop);
        compare(press(railTop, Qt.Key_Right), FocusNavigator.Moved);

        landOn(lowerTile);

        compare(press(lowerTile, Qt.Key_Left), FocusNavigator.Moved);
        verify(railBottom.activeFocus, "should be the button nearest where the cursor now is");
    }

    // Wandering off and coming back to the same place does not bring it back
    function test_returning_to_the_same_tile_does_not_restore_the_way_back() {
        landOn(railTop);
        compare(press(railTop, Qt.Key_Right), FocusNavigator.Moved);

        landOn(lowerTile);
        landOn(tile);

        compare(press(tile, Qt.Key_Left), FocusNavigator.Moved);
        verify(railMiddle.activeFocus, "the way back should have been dropped when the cursor moved on");
    }

    function test_a_different_direction_is_not_a_return() {
        landOn(railTop);
        compare(press(railTop, Qt.Key_Right), FocusNavigator.Moved);

        compare(press(tile, Qt.Key_Down), FocusNavigator.Moved);
        verify(lowerTile.activeFocus);
    }

    // Reversing twice walks back out the way it came
    function test_the_way_back_is_recorded_again_on_the_way_back() {
        landOn(railTop);
        compare(press(railTop, Qt.Key_Right), FocusNavigator.Moved);
        compare(press(tile, Qt.Key_Left), FocusNavigator.Moved);
        verify(railTop.activeFocus);

        compare(press(railTop, Qt.Key_Right), FocusNavigator.Moved);
        verify(tile.activeFocus);
    }
}
