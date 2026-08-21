// TODO: NEEDS REVIEW
.pragma library

// The focus cursor's edge bump: a damped spring, and how far it should carry for the item the ring
// is drawn around. Kept as plain functions so the spring and the size law can be tested without a
// window, and so neither reaches a style singleton.

// Fitted to a measured Switch 2 bump: peak at ~15.6ms, ~12% undershoot, settled by ~110ms
var STIFFNESS = 5700;
var DAMPING = 84;

var DECAY = DAMPING / 2;

// Guarded, so constants that stop being underdamped give a real frequency rather than NaN geometry
var DAMPED_FREQ = Math.sqrt(Math.max(1e-9, STIFFNESS - DECAY * DECAY));

// When the impulse response peaks, and how far it travels there per unit of kick
var PEAK_TIME = Math.atan(DAMPED_FREQ / DECAY) / DAMPED_FREQ;
var PEAK_PER_KICK = Math.exp(-DECAY * PEAK_TIME) * Math.sin(DAMPED_FREQ * PEAK_TIME) / DAMPED_FREQ;

// The item size the full peak belongs to, and the range a bump is allowed to land in
var REF_SIZE = 128;
var MIN_PEAK = 2;
var MAX_PEAK = 12;

// The kick that makes the spring peak `peakPx` from rest
function kickFor(peakPx) {
    return peakPx / PEAK_PER_KICK;
}

// How far the bump carries on an item this size, driven by its shorter side: a wide thin row is a
// small thing however wide. Square root rather than linear so the items this app actually has span
// the useful range instead of sitting on the clamps
function peakFor(width, height, refPeak) {
    var size = Math.max(0, Math.min(width, height));

    return Math.max(MIN_PEAK, Math.min(MAX_PEAK, refPeak * Math.sqrt(size / REF_SIZE)));
}

// The spring `dt` later, evaluated rather than integrated, so the curve is the same whatever the
// frame rate and a stalled frame decays further instead of destabilising it
function step(pos, vel, dt) {
    var decay = Math.exp(-DECAY * dt);
    var c = Math.cos(DAMPED_FREQ * dt);
    var s = Math.sin(DAMPED_FREQ * dt);

    return {
        pos: decay * (pos * c + ((vel + DECAY * pos) / DAMPED_FREQ) * s),
        vel: decay * (vel * c - ((STIFFNESS * pos + DECAY * vel) / DAMPED_FREQ) * s)
    };
}

// Whether the bump is over, measured against its own size so a small bump and a large one last the
// same time
function settled(pos, vel, peakPx) {
    return Math.abs(pos) < 0.012 * peakPx && Math.abs(vel) < 0.485 * peakPx;
}
