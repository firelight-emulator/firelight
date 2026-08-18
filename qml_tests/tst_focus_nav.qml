import QtQuick
import QtTest

import "qrc:/qt/qml/QMLFirelightTest/focus_nav.js" as Nav

// The index arithmetic behind grid navigation, tested without a view. The cases
// that matter are the edges: a press that should leave the grid must report so,
// and a short final row must stay reachable
TestCase {
    id: testCase
    name: "FocusNav"

    // 10 items, 4 across: rows are 0-3, 4-7, 8-9
    readonly property int count: 10
    readonly property int columns: 4

    function step(index, direction) {
        return Nav.gridStep(index, testCase.count, testCase.columns, direction);
    }

    function test_moves_within_a_row() {
        compare(step(1, Nav.Right), 2);
        compare(step(2, Nav.Left), 1);
    }

    // Reaching a row's edge must refuse, so the press can leave the grid
    function test_left_at_the_first_column_refuses() {
        compare(step(0, Nav.Left), -1);
        compare(step(4, Nav.Left), -1);
        compare(step(8, Nav.Left), -1);
    }

    function test_right_at_the_last_column_refuses() {
        compare(step(3, Nav.Right), -1);
        compare(step(7, Nav.Right), -1);
    }

    function test_right_at_the_last_item_refuses() {
        compare(step(9, Nav.Right), -1);
    }

    function test_moves_between_rows() {
        compare(step(0, Nav.Down), 4);
        compare(step(4, Nav.Down), 8);
        compare(step(4, Nav.Up), 0);
        compare(step(8, Nav.Up), 4);
    }

    function test_up_from_the_first_row_refuses() {
        compare(step(0, Nav.Up), -1);
        compare(step(3, Nav.Up), -1);
    }

    function test_down_from_the_last_row_refuses() {
        compare(step(8, Nav.Down), -1);
        compare(step(9, Nav.Down), -1);
    }

    // The case both the hand-rolled arithmetic and Qt's moveCurrentIndexDown()
    // get wrong: index 6 sits above a row that only reaches column 1, and both
    // refuse rather than clamping, stranding the last row
    function test_down_into_a_short_final_row_clamps() {
        compare(step(6, Nav.Down), 9);
        compare(step(7, Nav.Down), 9);
        compare(step(5, Nav.Down), 9);
        compare(step(4, Nav.Down), 8);
    }

    function test_a_single_column_grid_walks_vertically() {
        compare(Nav.gridStep(0, 3, 1, Nav.Down), 1);
        compare(Nav.gridStep(2, 3, 1, Nav.Down), -1);
        compare(Nav.gridStep(1, 3, 1, Nav.Up), 0);
        compare(Nav.gridStep(1, 3, 1, Nav.Left), -1);
        compare(Nav.gridStep(1, 3, 1, Nav.Right), -1);
    }

    function test_a_full_final_row_needs_no_clamping() {
        compare(Nav.gridStep(4, 8, 4, Nav.Down), -1);
        compare(Nav.gridStep(0, 8, 4, Nav.Down), 4);
    }

    function test_an_empty_grid_refuses() {
        compare(Nav.gridStep(0, 0, 4, Nav.Down), -1);
        compare(Nav.gridStep(0, 0, 4, Nav.Right), -1);
    }

    function test_nonsense_arguments_refuse() {
        compare(Nav.gridStep(-1, 10, 4, Nav.Down), -1);
        compare(Nav.gridStep(0, 10, 0, Nav.Down), -1);
        compare(Nav.gridStep(0, 10, 4, 0), -1);
    }

    // Non-focusable rows exist to be skipped: separators and section labels sit
    // in the same list as the controls
    function test_skips_children_that_cannot_take_focus() {
        const children = [
            {
                visible: true,
                enabled: true,
                focusPolicy: 0
            },
            {
                visible: true,
                enabled: true,
                focusPolicy: 2
            },
            {
                visible: false,
                enabled: true,
                focusPolicy: 2
            },
            {
                visible: true,
                enabled: false,
                focusPolicy: 2
            },
            {
                visible: true,
                enabled: true,
                focusPolicy: 2
            }
        ];

        compare(Nav.nextFocusable(children, -1, 1), 1);
        compare(Nav.nextFocusable(children, 1, 1), 4);
        compare(Nav.nextFocusable(children, 4, 1), -1);
        compare(Nav.nextFocusable(children, 4, -1), 1);
        compare(Nav.firstFocusable(children, 0), 1);
        compare(Nav.firstFocusable(children, 3), 4);
    }

    // A Repeater is a child of the column like any other and has no focusPolicy at all. Comparing an
    // undefined policy against NoFocus reads as "focusable", which hands the cursor to something with
    // no size sitting at children[0]
    function test_aChildWithNoFocusPolicyIsNotFocusable() {
        const repeaterLike = {
            visible: true,
            enabled: true
        };

        verify(!Nav.isFocusable(repeaterLike), "a child with no focusPolicy reported as focusable");
    }

    function test_aRealControlIsStillFocusable() {
        const control = {
            visible: true,
            enabled: true,
            focusPolicy: Qt.StrongFocus
        };

        verify(Nav.isFocusable(control), "a StrongFocus control stopped being focusable");
    }

    function test_anExplicitlyUnfocusableControlIsStillSkipped() {
        const control = {
            visible: true,
            enabled: true,
            focusPolicy: Qt.NoFocus
        };

        verify(!Nav.isFocusable(control), "a NoFocus control reported as focusable");
    }
}
