// TODO: NEEDS REVIEW
.pragma library

// The arithmetic behind holding an arrow on a slider, kept as plain functions so the rate and the
// rounding can be tested without a window, and so neither depends on a style singleton.

// The value rounded to the nearest step, clamped to the range. A step of 0 or less leaves the value
// alone, which is a slider with no steps to land on
function stepped(value, from, to, stepSize) {
    if (stepSize <= 0) {
        return value;
    }

    const steps = Math.round((value - from) / stepSize);
    return Math.max(from, Math.min(to, from + steps * stepSize));
}

// How far the handle travels in `seconds`, as a change in value. The rate is a fraction of the whole
// range per second, so a slider crosses its track in `traverseMs` whatever its range or step
function delta(direction, from, to, seconds, traverseMs) {
    return direction * (to - from) * seconds / (traverseMs / 1000);
}

// One step from the step the handle is in. A slider with no steps moves a tenth of its range
function nudged(value, from, to, stepSize, direction) {
    const step = stepSize > 0 ? stepSize : (to - from) / 10;
    return Math.max(from, Math.min(to, stepped(value, from, to, stepSize) + direction * step));
}

// Whether a value from elsewhere disagrees with what the handle shows by enough to move it. Anything
// within half a step is the slider's own report coming back
function shouldReseat(storedValue, shownStepped, stepSize) {
    return Math.abs(storedValue - shownStepped) > stepSize / 2;
}

// Whether the handle can go no further the way it is being carried. An end is only an end in the
// direction that leads out of the range
function atEnd(value, from, to, direction) {
    if (direction < 0) {
        return value <= from;
    }

    if (direction > 0) {
        return value >= to;
    }

    return false;
}

// Whether arriving at an end should show itself. Held against an end this is asked every frame, and
// only the first arrival is answered
function shouldBump(alreadyBumped, value, from, to, direction) {
    return !alreadyBumped && atEnd(value, from, to, direction);
}

// Which way two held arrows carry the handle: while both are down the one pressed most recently
// wins, and releasing it hands back to the one still held rather than stopping
function heldDirection(leftHeld, rightHeld, lastPressed) {
    if (leftHeld && rightHeld) {
        return lastPressed;
    }

    if (leftHeld) {
        return -1;
    }

    if (rightHeld) {
        return 1;
    }

    return 0;
}
