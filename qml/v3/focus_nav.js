.pragma library

// Directional navigation helpers, kept as plain functions so the containers that
// need them can share one implementation despite having different QML bases —
// and so the index arithmetic can be tested without a window.
//
// The direction values mirror FLFocus.Edge, so the two are interchangeable.

var Up = 0x1;
var Down = 0x2;
var Left = 0x4;
var Right = 0x8;

// 0 is Qt.NoFocus, which is not reachable from a pragma-library script
var NO_FOCUS = 0;

// A child that can hold the cursor. `focusPolicy` has to exist before it is compared: a Repeater is a
// child like any other and has none, so an undefined policy would read as "not NoFocus" and hand the
// cursor to something with no size
function isFocusable(child) {
    return !!child && child.visible && child.enabled && child.focusPolicy !== undefined && child.focusPolicy !== NO_FOCUS;
}

// Where the cursor actually is among `children`, or -1 when it is elsewhere.
// activeFocus propagates through focus scopes but not through plain wrappers, so
// a child that merely contains the focused item is found by descending into it
function focusedIndex(children) {
    for (var i = 0; i < children.length; i++) {
        if (children[i] && children[i].activeFocus) {
            return i;
        }
    }

    return -1;
}

// Which delegate of `view` the cursor is on, or -1 when it is elsewhere. A view
// steps from its own current index, and a tap or a move in from outside puts the
// cursor on a delegate without touching that index
function focusedDelegate(view, focused) {
    for (var item = focused; item; item = item.parent) {
        if (item.parent === view.contentItem) {
            return view.indexAt(item.x + item.width / 2, item.y + item.height / 2);
        }
    }

    return -1;
}

// The next child in `step` direction that can take focus, or -1 when the run of
// children ends first
function nextFocusable(children, from, step) {
    for (var i = from + step; i >= 0 && i < children.length; i += step) {
        if (isFocusable(children[i])) {
            return i;
        }
    }

    return -1;
}

// The first child that can take focus, searching outward from `from` so entering
// a container lands where the cursor last was rather than always at the top
function firstFocusable(children, from) {
    var start = Math.max(0, Math.min(from, children.length - 1));

    if (isFocusable(children[start])) {
        return start;
    }

    var forward = nextFocusable(children, start, 1);

    if (forward >= 0) {
        return forward;
    }

    return nextFocusable(children, start, -1);
}

// Where a directional press moves within a grid, or -1 when it should leave.
//
// Left and Right stay inside their row rather than running on into the next one,
// so reaching a row's edge hands the press outward instead of wrapping. Down
// clamps into a short final row, which neither the hand-rolled arithmetic nor
// Qt's own moveCurrentIndexDown() does — both refuse, stranding the last row
function gridStep(index, count, columns, direction) {
    if (count <= 0 || columns <= 0 || index < 0) {
        return -1;
    }

    if (direction === Left) {
        return index % columns === 0 ? -1 : index - 1;
    }

    if (direction === Right) {
        return (index + 1) % columns === 0 || index >= count - 1 ? -1 : index + 1;
    }

    var row = Math.floor(index / columns);
    var lastRow = Math.floor((count - 1) / columns);

    if (direction === Up) {
        return row === 0 ? -1 : index - columns;
    }

    if (direction === Down) {
        if (row >= lastRow) {
            return -1;
        }

        return Math.min(index + columns, count - 1);
    }

    return -1;
}
