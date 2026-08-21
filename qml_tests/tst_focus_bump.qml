// TODO: NEEDS REVIEW
import QtQuick
import QtTest

import "qrc:/qt/qml/QMLFirelightTest/focus_bump.js" as Bump

// The focus cursor's edge bump: how far it carries for the item it is drawn around, and the spring
// that gets it there. A bump whose size depends on the monitor is not a tuned constant
TestCase {
    id: testCase
    name: "FocusBumpTests"

    // Runs the spring from rest at `peak` px, sampling at a fixed frame rate, and reports what was
    // actually drawn
    function fly(peak, fps, seconds) {
        let pos = 0;
        let vel = Bump.kickFor(peak);
        let highest = 0;
        let lowest = 0;
        let timeAtPeak = 0;
        let time = 0;
        const dt = 1 / fps;

        for (let i = 0; i < Math.ceil(seconds * fps); i++) {
            const next = Bump.step(pos, vel, dt);
            pos = next.pos;
            vel = next.vel;
            time += dt;

            if (Math.abs(pos) > highest) {
                highest = Math.abs(pos);
                timeAtPeak = time;
            }

            if (pos < lowest) {
                lowest = pos;
            }
        }

        return {
            peak: highest,
            timeAtPeak: timeAtPeak,
            undershoot: lowest,
            pos: pos,
            vel: vel
        };
    }

    //****************
    // asking in pixels
    //****************

    // The whole point of the conversion: what is asked for is what is drawn
    function test_aBumpPeaksWhereItWasAskedTo() {
        const wanted = [2, 4, 5, 7, 12];

        for (let i = 0; i < wanted.length; i++) {
            const flight = testCase.fly(wanted[i], 1000, 0.3);

            fuzzyCompare(flight.peak, wanted[i], wanted[i] * 0.005, "a bump asked for " + wanted[i] + "px peaked at " + flight.peak.toFixed(3));
        }
    }

    function test_theKickScalesWithWhatIsAskedFor() {
        fuzzyCompare(Bump.kickFor(10) / Bump.kickFor(5), 2, 1e-9, "the spring stopped being linear");
        compare(Bump.kickFor(0), 0);
    }

    //****************
    // the same on every display
    //****************

    // The regression this exists for: the old integrator gave 1.92px at 75Hz, 5.71px at 240Hz and
    // diverged at 60Hz, all for one constant
    function test_theBumpIsTheSameSizeAtEveryUsableFrameRate() {
        const rates = [60, 75, 90, 120, 144, 165, 240];

        for (let i = 0; i < rates.length; i++) {
            const flight = testCase.fly(5, rates[i], 0.3);

            verify(Math.abs(flight.peak - 5) < 5 * 0.08, "at " + rates[i] + "fps a 5px bump drew " + flight.peak.toFixed(3) + "px");
        }
    }

    // A slow frame rate can no longer blow the ring off the screen, even where it cannot show the
    // whole curve
    function test_aSlowFrameRateShrinksTheBumpRatherThanExplodingIt() {
        const rates = [24, 30, 40];

        for (let i = 0; i < rates.length; i++) {
            const flight = testCase.fly(5, rates[i], 1.0);

            verify(flight.peak <= 5.5, "at " + rates[i] + "fps the bump grew to " + flight.peak.toFixed(2) + "px");
            verify(Math.abs(flight.pos) < 0.05, "at " + rates[i] + "fps the bump never came back to rest");
        }
    }

    // A stalled frame decays further along the curve instead of destabilising it
    function test_aStalledFrameDoesNotDestabiliseIt() {
        let pos = 0;
        let vel = Bump.kickFor(5);
        let highest = 0;
        const frames = [1 / 144, 1 / 144, 1 / 144, 0.2, 1 / 144, 1 / 144, 0.35];

        for (let i = 0; i < frames.length; i++) {
            const next = Bump.step(pos, vel, frames[i]);
            pos = next.pos;
            vel = next.vel;
            highest = Math.max(highest, Math.abs(pos));
        }

        verify(highest <= 5.5, "a stalled frame grew the bump to " + highest.toFixed(2) + "px");
        verify(Math.abs(pos) < 0.01, "the bump did not decay through the stall");
    }

    //****************
    // the shape stays put
    //****************

    // The two numbers the spring was fitted to. They are what would change if someone edited the
    // constants without meaning to
    function test_theBumpKeepsItsShape() {
        const flight = testCase.fly(5, 1000, 0.3);

        verify(Math.abs(flight.timeAtPeak * 1000 - 15.6) < 1.0, "the bump peaked at " + (flight.timeAtPeak * 1000).toFixed(1) + "ms");

        const undershoot = 100 * Math.abs(flight.undershoot) / flight.peak;
        verify(Math.abs(undershoot - 12.2) < 1.0, "the bump came back " + undershoot.toFixed(1) + "% past rest");
    }

    function test_theSpringStaysUnderdamped() {
        verify(Bump.DAMPED_FREQ > 0, "the spring stopped oscillating");
        verify(Bump.DECAY < Math.sqrt(Bump.STIFFNESS), "the spring is no longer underdamped");
        verify(!isNaN(Bump.PEAK_PER_KICK), "the conversion is NaN, which would hide the ring entirely");
    }

    //****************
    // sized to the item
    //****************

    function test_aBiggerItemBumpsFurther() {
        const handle = Bump.peakFor(26, 26, 5);
        const row = Bump.peakFor(476, 46, 5);
        const tile = Bump.peakFor(128, 188, 5);
        const bigTile = Bump.peakFor(228, 288, 5);

        verify(handle < row, "a 26px handle did not bump less than a settings row");
        verify(row < tile, "a settings row did not bump less than a game tile");
        verify(tile < bigTile, "a small tile did not bump less than a large one");
    }

    // The shorter side is what decides it, so a wide thin row is a small thing however wide
    function test_theShorterSideDecidesIt() {
        fuzzyCompare(Bump.peakFor(476, 46, 5), Bump.peakFor(46, 46, 5), 1e-9, "width changed the bump of a short row");
        verify(Bump.peakFor(476, 46, 5) < Bump.peakFor(128, 128, 5), "a wide row out-bumped a game tile");
    }

    function test_itDoesNotCareWhichWayRoundTheSidesAre() {
        fuzzyCompare(Bump.peakFor(128, 188, 5), Bump.peakFor(188, 128, 5), 1e-9, "a rotated item bumped differently");
    }

    // A reference-sized item gets exactly what was asked for
    function test_theReferenceSizeGetsTheFullPeak() {
        fuzzyCompare(Bump.peakFor(Bump.REF_SIZE, Bump.REF_SIZE, 5), 5, 1e-9, "the reference item did not get the reference peak");
    }

    function test_theBumpStaysWithinItsRange() {
        compare(Bump.peakFor(1, 1, 5), Bump.MIN_PEAK, "a tiny item bumped below the floor");
        compare(Bump.peakFor(4000, 4000, 5), Bump.MAX_PEAK, "a huge item bumped past the ceiling");
    }

    // A collapsed item must not produce NaN, which would be written straight into the ring geometry
    function test_aCollapsedItemStillGivesANumber() {
        const peak = Bump.peakFor(0, 0, 5);

        verify(!isNaN(peak), "a zero-sized item gave NaN");
        compare(peak, Bump.MIN_PEAK);
    }

    //****************
    // knowing when to stop
    //****************

    // Absolute thresholds made a large bump last longer than a small one; relative ones do not
    function test_everyBumpLastsTheSameTime() {
        const durations = [];
        const peaks = [2, 5, 12];

        for (let i = 0; i < peaks.length; i++) {
            let pos = 0;
            let vel = Bump.kickFor(peaks[i]);
            let time = 0;
            const dt = 1 / 240;

            while (time < 1) {
                const next = Bump.step(pos, vel, dt);
                pos = next.pos;
                vel = next.vel;
                time += dt;

                if (time > 0.005 && Bump.settled(pos, vel, peaks[i])) {
                    break;
                }
            }

            durations.push(time * 1000);
        }

        for (let i = 0; i < durations.length; i++) {
            verify(durations[i] > 60 && durations[i] < 200, "a " + peaks[i] + "px bump took " + durations[i].toFixed(0) + "ms");
        }

        verify(Math.abs(durations[0] - durations[durations.length - 1]) < 15, "a 2px bump and a 12px bump lasted " + durations[0].toFixed(0) + "ms and " + durations[durations.length - 1].toFixed(0) + "ms");
    }

    function test_aBumpAtItsPeakIsNotSettled() {
        verify(!Bump.settled(5, 0, 5), "a bump sitting at its peak reported as finished");
        verify(Bump.settled(0, 0, 5), "a bump at rest reported as still going");
    }
}
