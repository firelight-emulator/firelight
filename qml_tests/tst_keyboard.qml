// TODO: NEEDS REVIEW
import QtQuick
import QtTest
import "qrc:/qt/qml/QMLFirelightTest/keyboard_layout.js" as KeyLayout

// The on-screen keyboard's shift rule and its character banks. Shift is one-shot until it is
// pressed a second time without a character in between, and every bank has to stay the shape the
// grid was built for
TestCase {
    id: testCase
    name: "Keyboard"

    //****************
    // shift
    //****************

    function test_one_press_arms_shift() {
        compare(KeyLayout.nextShift(KeyLayout.SHIFT_OFF), KeyLayout.SHIFT_ARMED);
    }

    function test_a_second_press_locks_shift() {
        compare(KeyLayout.nextShift(KeyLayout.SHIFT_ARMED), KeyLayout.SHIFT_LOCKED);
    }

    function test_pressing_shift_while_locked_clears_it() {
        compare(KeyLayout.nextShift(KeyLayout.SHIFT_LOCKED), KeyLayout.SHIFT_OFF);
    }

    // A one-shot shift is spent by the character it capitalised
    function test_a_character_disarms_shift() {
        compare(KeyLayout.shiftAfterCharacter(KeyLayout.SHIFT_ARMED), KeyLayout.SHIFT_OFF);
    }

    // Caps lock is what a second press bought, so characters must not clear it
    function test_a_character_does_not_clear_the_lock() {
        compare(KeyLayout.shiftAfterCharacter(KeyLayout.SHIFT_LOCKED), KeyLayout.SHIFT_LOCKED);
    }

    function test_a_character_leaves_an_unshifted_keyboard_alone() {
        compare(KeyLayout.shiftAfterCharacter(KeyLayout.SHIFT_OFF), KeyLayout.SHIFT_OFF);
    }

    // Three presses with no character in between is a full circuit
    function test_shift_cycles_back_to_off() {
        let state = KeyLayout.SHIFT_OFF;

        for (let i = 0; i < 3; i++) {
            state = KeyLayout.nextShift(state);
        }

        compare(state, KeyLayout.SHIFT_OFF);
    }

    //****************
    // banks
    //****************

    function test_shift_swaps_the_letter_rows() {
        compare(KeyLayout.characterAt("abc", false, 1, 0), "q");
        compare(KeyLayout.characterAt("abc", true, 1, 0), "Q");
    }

    // The number row is the same either way, so shift reads as a caps key rather than a US shift
    function test_shift_leaves_the_number_row_alone() {
        compare(KeyLayout.characterAt("abc", true, 0, 0), KeyLayout.characterAt("abc", false, 0, 0));
    }

    function test_the_symbol_page_replaces_the_letters() {
        compare(KeyLayout.characterAt("sym", false, 0, 0), "[");
    }

    // Shift has no bank of its own on the symbol page, so it must not change what is shown
    function test_shift_does_nothing_on_the_symbol_page() {
        compare(KeyLayout.rowsFor("sym", true), KeyLayout.rowsFor("sym", false));
    }

    //****************
    // grid shape
    //****************

    // The grid reserves 11 columns for characters over 4 rows, so a short or long row would
    // silently leave a hole in it
    function test_every_row_is_the_width_the_grid_expects() {
        const banks = [KeyLayout.rowsFor("abc", false), KeyLayout.rowsFor("abc", true), KeyLayout.rowsFor("sym", false)];

        for (let b = 0; b < banks.length; b++) {
            compare(banks[b].length, KeyLayout.CHARACTER_ROWS, "bank " + b);

            for (let r = 0; r < banks[b].length; r++) {
                compare(banks[b][r].length, KeyLayout.CHARACTER_COLUMNS, "bank " + b + " row " + r + " is '" + banks[b][r] + "'");
            }
        }
    }

    function test_the_cell_list_fills_the_grid() {
        compare(KeyLayout.characterCells().length, KeyLayout.CHARACTER_ROWS * KeyLayout.CHARACTER_COLUMNS);
    }

    // Every cell must be reachable and distinct, or a key would be drawn on top of another
    function test_every_cell_is_a_distinct_position() {
        const cells = KeyLayout.characterCells();
        let seen = {};

        for (let i = 0; i < cells.length; i++) {
            const key = cells[i].row + "," + cells[i].column;

            verify(seen[key] === undefined, "duplicate cell " + key);
            seen[key] = true;
        }
    }

    // Upper and lower have to line up position for position, or shift would move keys around
    function test_the_shifted_bank_matches_the_unshifted_one() {
        const lower = KeyLayout.rowsFor("abc", false);
        const upper = KeyLayout.rowsFor("abc", true);

        for (let r = 0; r < lower.length; r++) {
            compare(upper[r], lower[r].toUpperCase(), "row " + r);
        }
    }
}
