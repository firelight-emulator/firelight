.pragma library

// The on-screen keyboard's character banks and its shift rule. Kept out of QML so the
// parts worth asserting can be tested without building a key grid

// The grid reserves this much for characters; the remaining columns are the utility block
var CHARACTER_COLUMNS = 11;
var CHARACTER_ROWS = 4;

var SHIFT_OFF = 0;
var SHIFT_ARMED = 1;
var SHIFT_LOCKED = 2;

// One string per row, and the index in the string is the column. Every bank is the same
// shape so switching page or case never moves a key or leaves a hole
var ROWS_LOWER = ["1234567890-", "qwertyuiop/", "asdfghjkl:'", "zxcvbnm,.?!"];
var ROWS_UPPER = ["1234567890-", "QWERTYUIOP/", "ASDFGHJKL:'", "ZXCVBNM,.?!"];
var ROWS_SYMBOL = ["[]{}#%^*+=_", "<>$@&,.?!'\"", "~\\|;:/()`€£", "¥¢°•…—–«»§¶"];

// Shift uppercases the letters and leaves the digits alone, so it reads as a caps key
// rather than a US-keyboard shift
function rowsFor(page, shifted) {
    if (page === "sym") {
        return ROWS_SYMBOL;
    }

    return shifted ? ROWS_UPPER : ROWS_LOWER;
}

function characterAt(page, shifted, row, column) {
    return rowsFor(page, shifted)[row].charAt(column);
}

// Off arms, armed locks, locked clears
function nextShift(current) {
    if (current === SHIFT_OFF) {
        return SHIFT_ARMED;
    }

    if (current === SHIFT_ARMED) {
        return SHIFT_LOCKED;
    }

    return SHIFT_OFF;
}

// An armed shift lasts exactly one character; a locked one is untouched
function shiftAfterCharacter(current) {
    return current === SHIFT_ARMED ? SHIFT_OFF : current;
}

// Positions only. Built once and handed to the grid as a constant model, so no delegate is
// destroyed when the bank changes
function characterCells() {
    var out = [];

    for (var r = 0; r < CHARACTER_ROWS; ++r) {
        for (var c = 0; c < CHARACTER_COLUMNS; ++c) {
            out.push({
                row: r,
                column: c
            });
        }
    }

    return out;
}
