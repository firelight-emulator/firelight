// TODO: NEEDS REVIEW
import QtQuick
import QtTest
import QtQuick.Controls
import QtQuick.Layouts

import "qrc:/qt/qml/QMLFirelightTest/slider_slide.js" as Slide

// What a Slider does with arrow keys, and what is left for anything above it. A slider that keeps a
// key it was never meant to handle strands the cursor on the row it sits in
TestCase {
    id: testCase
    name: "SliderKeyTests"

    width: 400
    height: 200

    // Keys only reach a visible item
    visible: true
    when: windowShown

    // A bare Slider, to find out what Qt handles before anything is overridden
    Component {
        id: plainSlider

        Item {
            width: 400
            height: 200

            property alias slider: theSlider
            property int parentSawUp: 0
            property int parentSawLeft: 0

            Keys.onPressed: event => {
                if (event.key === Qt.Key_Up) {
                    parentSawUp++;
                } else if (event.key === Qt.Key_Left) {
                    parentSawLeft++;
                }
            }

            Slider {
                id: theSlider
                anchors.fill: parent
                focus: true
                focusPolicy: Qt.StrongFocus
                from: 0
                to: 10
                stepSize: 1
                value: 5
            }
        }
    }

    // The same slider with the arrows taken over ahead of it, which is the whole of "the built-in key
    // handlers should not do anything"
    Component {
        id: overriddenSlider

        Item {
            width: 400
            height: 200

            property alias slider: theSlider
            property int parentSawUp: 0
            property int ourPresses: 0

            Keys.onPressed: event => {
                if (event.key === Qt.Key_Up) {
                    parentSawUp++;
                }
            }

            Slider {
                id: theSlider
                anchors.fill: parent
                focus: true
                focusPolicy: Qt.StrongFocus
                from: 0
                to: 10
                stepSize: 1
                value: 5

                Keys.priority: Keys.BeforeItem

                Keys.onPressed: event => {
                    if (event.key !== Qt.Key_Left && event.key !== Qt.Key_Right) {
                        event.accepted = false;
                        return;
                    }

                    theSlider.parent.ourPresses++;
                    event.accepted = true;
                }
            }
        }
    }

    // The settings row: a focus scope that hands focus on to the control inside it, which is how the
    // cursor lands on the row while the arrows reach the slider
    Component {
        id: rowForwardingFocus

        FocusScope {
            id: outer
            width: 400
            height: 200

            property alias row: theRow
            property alias slider: theSlider

            FocusScope {
                id: theRow
                anchors.fill: parent
                focusPolicy: Qt.StrongFocus

                onActiveFocusChanged: {
                    if (theRow.activeFocus) {
                        theSlider.forceActiveFocus();
                    }
                }

                Slider {
                    id: theSlider
                    anchors.fill: parent
                    focusPolicy: Qt.StrongFocus
                    from: 0
                    to: 10
                    stepSize: 1
                    value: 5

                    Keys.priority: Keys.BeforeItem

                    Keys.onPressed: event => {
                        if (event.key !== Qt.Key_Left && event.key !== Qt.Key_Right) {
                            event.accepted = false;
                            return;
                        }

                        theSlider.value += (event.key === Qt.Key_Left ? -1 : 1);
                        event.accepted = true;
                    }
                }
            }
        }
    }

    // The row as it is actually built: the control is assigned to a property and reparented into a
    // slot by a Binding, inside a section that is itself a focus scope
    Component {
        id: realisticRow

        FocusScope {
            id: section
            width: 400
            height: 200

            property alias row: theRow
            property alias slider: theSlider

            FocusScope {
                id: theRow
                anchors.fill: parent
                focusPolicy: Qt.StrongFocus

                property Item control: RowLayout {
                    Slider {
                        id: theSlider
                        Layout.fillWidth: true
                        focusPolicy: Qt.StrongFocus
                        from: 0
                        to: 10
                        stepSize: 1
                        value: 5

                        Keys.priority: Keys.BeforeItem

                        Keys.onPressed: event => {
                            if (event.key !== Qt.Key_Left && event.key !== Qt.Key_Right) {
                                event.accepted = false;
                                return;
                            }

                            theSlider.value += (event.key === Qt.Key_Left ? -1 : 1);
                            event.accepted = true;
                        }
                    }
                }

                onActiveFocusChanged: {
                    if (theRow.activeFocus) {
                        theSlider.forceActiveFocus();
                    }
                }

                Binding {
                    target: theRow.control
                    property: "parent"
                    value: slot
                    when: theRow.control !== null
                }

                Item {
                    id: slot
                    anchors.fill: parent
                }
            }
        }
    }

    function test_aReparentedControlStillTakesTheArrows() {
        const scene = createTemporaryObject(realisticRow, testCase);
        verify(scene !== null);

        scene.row.forceActiveFocus();

        verify(scene.slider.activeFocus, "the reparented slider never took focus");

        keyClick(Qt.Key_Right);

        compare(scene.slider.value, 6, "the arrows did not reach the reparented slider");
    }

    function test_focusingTheRowReachesTheSlider() {
        const scene = createTemporaryObject(rowForwardingFocus, testCase);
        verify(scene !== null);

        scene.row.forceActiveFocus();

        verify(scene.row.activeFocus, "the row never took focus");
        verify(scene.slider.activeFocus, "the row did not hand focus to the slider");

        keyClick(Qt.Key_Right);

        compare(scene.slider.value, 6, "the arrows did not reach the slider");
    }

    function test_qtHandlesLeftRightItself() {
        const scene = createTemporaryObject(plainSlider, testCase);
        verify(scene !== null);
        verify(scene.slider.activeFocus, "the slider never took focus");

        keyClick(Qt.Key_Left);

        compare(scene.slider.value, 4, "Qt did not move the value by one step");
        compare(scene.parentSawLeft, 0, "Left reached the parent, so Qt did not consume it");
    }

    // The one that decides whether focus can leave a settings row that holds a slider
    function test_qtLeavesUpDownAloneOnAHorizontalSlider() {
        const scene = createTemporaryObject(plainSlider, testCase);
        verify(scene !== null);
        verify(scene.slider.horizontal);

        const before = scene.slider.value;
        keyClick(Qt.Key_Up);

        compare(scene.slider.value, before, "a horizontal Slider changed value on Up");
        compare(scene.parentSawUp, 1, "Up did not reach the parent, so focus cannot leave the slider");
    }

    function test_beforeItemSilencesQt() {
        const scene = createTemporaryObject(overriddenSlider, testCase);
        verify(scene !== null);
        verify(scene.slider.activeFocus, "the slider never took focus");

        keyClick(Qt.Key_Left);

        compare(scene.ourPresses, 1, "our handler never ran");
        compare(scene.slider.value, 5, "Qt moved the value anyway, so it was not silenced");
    }

    function test_declinedKeysStillReachTheParent() {
        const scene = createTemporaryObject(overriddenSlider, testCase);
        verify(scene !== null);

        keyClick(Qt.Key_Up);

        compare(scene.parentSawUp, 1, "declining a key did not let it through");
    }

    //****************
    // grabbing an item
    //****************

    // The dimmer redraws whatever raised it by grabbing that item. Whether the item has to render
    // through a layer to be grabbable decides whether a grid tile can drop its layer
    Component {
        id: grabTarget

        Rectangle {
            width: 120
            height: 80
            color: "red"

            property bool layered: false
            layer.enabled: layered

            Text {
                anchors.centerIn: parent
                text: "grab me"
            }
        }
    }

    function grabs(layered) {
        const item = createTemporaryObject(grabTarget, testCase, {
            layered: layered
        });
        verify(item !== null);

        let finished = false;
        let url = "";

        const started = item.grabToImage(function (result) {
            finished = true;
            url = result === null ? "" : String(result.url);
        });

        verify(started, "grabToImage refused to start");
        tryVerify(function () {
            return finished;
        }, 3000, "the grab never called back");

        return url;
    }

    function test_anItemWithoutALayerCanStillBeGrabbed() {
        const url = grabs(false);

        verify(url !== "", "grabbing an unlayered item produced nothing");
    }

    function test_aLayeredItemCanAlsoBeGrabbed() {
        const url = grabs(true);

        verify(url !== "", "grabbing a layered item produced nothing");
    }

    //****************
    // the slide itself
    //****************

    // The whole point: range and step must not change how fast the handle moves
    function test_everySliderCrossesInTheSameTime() {
        const traverse = 1400;
        const seconds = 0.35;

        // A quarter of the traverse time should cover a quarter of each track
        const coarse = Slide.delta(1, 0, 10, seconds, traverse) / (10 - 0);
        const fine = Slide.delta(1, 100, 240, seconds, traverse) / (240 - 100);
        const tiny = Slide.delta(1, 0, 1, seconds, traverse) / (1 - 0);

        fuzzyCompare(coarse, 0.25, 1e-9, "a quarter of the time did not cover a quarter of the track");
        fuzzyCompare(fine, coarse, 1e-9, "a 100..240 slider moved at a different rate than a 0..10 one");
        fuzzyCompare(tiny, coarse, 1e-9, "a 0..1 slider moved at a different rate than a 0..10 one");
    }

    function test_slidingLeftMovesTheOtherWay() {
        const left = Slide.delta(-1, 0, 10, 0.2, 1400);
        const right = Slide.delta(1, 0, 10, 0.2, 1400);

        compare(left, -right, "left and right did not mirror");
        verify(left < 0);
    }

    function test_aSliderCrossesItsWholeTrackInTheTraverseTime() {
        const span = 240 - 100;

        fuzzyCompare(Slide.delta(1, 100, 240, 1.4, 1400), span, 1e-9, "a full traverse did not cover the track");
    }

    //****************
    // steps
    //****************

    function test_valuesRoundToTheNearestStep() {
        compare(Slide.stepped(4.4, 0, 10, 1), 4);
        compare(Slide.stepped(4.6, 0, 10, 1), 5);
        compare(Slide.stepped(137.4, 100, 240, 5), 135);
        compare(Slide.stepped(138, 100, 240, 5), 140);
    }

    function test_stepsAreCountedFromTheStartOfTheRange() {
        // 100..240 by 5 lands on 105, never on 103
        compare(Slide.stepped(103, 100, 240, 5), 105);
        compare(Slide.stepped(0.37, 0, 1, 0.25), 0.25);
    }

    // A range the step does not divide evenly has no step at its end: 0..10 by 4 lands on 0, 4, 8
    function test_roundingStaysOnTheStepGrid() {
        compare(Slide.stepped(9.9, 0, 10, 4), 8, "rounding left the step grid");
        compare(Slide.stepped(0.1, 0, 10, 4), 0);
    }

    // The ends stay reachable even when no step lands on them
    function test_theEndsAreStillReachable() {
        compare(Slide.stepped(10, 0, 10, 4), 10, "the end of the range could not be reported");
        compare(Slide.stepped(0, 0, 10, 4), 0);
        compare(Slide.nudged(8, 0, 10, 4, 1), 10, "a tap could not reach the end");
    }

    function test_aSliderWithNoStepsKeepsItsValue() {
        compare(Slide.stepped(4.371, 0, 10, 0), 4.371);
    }

    function test_aTapMovesExactlyOneStep() {
        compare(Slide.nudged(4, 0, 10, 1, 1), 5);
        compare(Slide.nudged(4, 0, 10, 1, -1), 3);

        // From between steps it lands on the next one, not one step from where it stood
        compare(Slide.nudged(4.4, 0, 10, 1, 1), 5);
        compare(Slide.nudged(4.4, 0, 10, 1, -1), 3);
    }

    function test_aTapStopsAtTheEnds() {
        compare(Slide.nudged(10, 0, 10, 1, 1), 10);
        compare(Slide.nudged(0, 0, 10, 1, -1), 0);
    }

    //****************
    // bumping at the ends
    //****************

    function test_theEndsAreWhereItCanGoNoFurther() {
        verify(Slide.atEnd(0, 0, 10, -1), "the bottom of the range did not read as an end");
        verify(Slide.atEnd(10, 0, 10, 1), "the top of the range did not read as an end");
        verify(!Slide.atEnd(5, 0, 10, -1));
        verify(!Slide.atEnd(5, 0, 10, 1));
    }

    // An end is only an end the way it is being carried, so sitting at the bottom and moving up is
    // not one
    function test_anEndBelongsToItsDirection() {
        verify(!Slide.atEnd(0, 0, 10, 1), "the bottom read as an end while moving up");
        verify(!Slide.atEnd(10, 0, 10, -1), "the top read as an end while moving down");
    }

    function test_aHandleGoingNowhereHasNoEnd() {
        verify(!Slide.atEnd(10, 0, 10, 0), "a still handle read as being at an end");
        verify(!Slide.shouldBump(false, 10, 0, 10, 0), "a still handle bumped");
    }

    // The gate: held against an end this is asked every frame and only the first arrival answers
    function test_arrivingAtAnEndBumpsOnceAndNotAgain() {
        verify(Slide.shouldBump(false, 10, 0, 10, 1), "arriving at the end did not bump");
        verify(!Slide.shouldBump(true, 10, 0, 10, 1), "holding against the end bumped again");
    }

    function test_theMiddleOfTheRangeNeverBumps() {
        verify(!Slide.shouldBump(false, 5, 0, 10, 1));
        verify(!Slide.shouldBump(false, 5, 0, 10, -1));
    }

    // Leaving an end re-arms it, so coming back bumps again
    function test_leavingAnEndLetsItBumpOnReturn() {
        verify(!Slide.atEnd(9, 0, 10, 1), "the handle still read as being at the end after leaving it");
        verify(Slide.shouldBump(false, 10, 0, 10, 1), "returning to the end did not bump");
    }

    //****************
    // holding both arrows
    //****************

    function test_oneHeldArrowCarriesItsOwnWay() {
        compare(Slide.heldDirection(true, false, -1), -1);
        compare(Slide.heldDirection(false, true, 1), 1);
    }

    function test_nothingHeldGoesNowhere() {
        compare(Slide.heldDirection(false, false, -1), 0, "a released key kept its direction");
    }

    function test_theArrowPressedLastWinsWhileBothAreDown() {
        compare(Slide.heldDirection(true, true, 1), 1, "pressing right while left was held did not take over");
        compare(Slide.heldDirection(true, true, -1), -1, "pressing left while right was held did not take over");
    }

    // Hold left, add right, then let go of right: left is still down, so it carries on left rather
    // than stopping
    function test_releasingTheNewArrowHandsBackToTheOneStillHeld() {
        compare(Slide.heldDirection(true, true, 1), 1, "right did not take over");
        compare(Slide.heldDirection(true, false, 1), -1, "letting go of right stopped it instead of handing back to left");
    }

    // The mirror of it: let go of the older arrow and the newer one keeps going
    function test_releasingTheOlderArrowLeavesTheNewerOneGoing() {
        compare(Slide.heldDirection(false, true, 1), 1, "letting go of left stopped it instead of leaving right going");
    }

    function test_lettingGoOfBothStops() {
        compare(Slide.heldDirection(false, false, 1), 0);
    }

    //****************
    // re-seating
    //****************

    function test_theSlidersOwnReportDoesNotMoveTheHandle() {
        // The handle sits at 137.4, reports 135, and 135 comes back from the setting
        verify(!Slide.shouldReseat(135, 135, 5), "the handle would be dragged onto its own report");
    }

    function test_aValueFromElsewhereMovesTheHandle() {
        verify(Slide.shouldReseat(200, 135, 5), "a reset would not move the handle");
    }

    function test_halfAStepIsTheBoundary() {
        verify(!Slide.shouldReseat(137, 135, 5), "moved for less than half a step");
        verify(Slide.shouldReseat(138, 135, 5), "did not move for more than half a step");
    }
}
