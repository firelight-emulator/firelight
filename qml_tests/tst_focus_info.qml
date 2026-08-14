// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls
import QtTest
import Firelight 1.0

// Covers the parts of FLFocus that only exist once QML is involved: declaring
// metadata and actions inline, and reaching an object's metadata from JS
TestCase {
    id: testCase
    name: "FocusInfo"

    Item {
        id: declared

        FLFocus.showCursor: true
        FLFocus.spacing: -8
        FLFocus.holdEdges: FLFocus.Vertical
        FLFocus.actions: [
            FLAction {
                keys: [Qt.Key_Select, Qt.Key_Return]
                label: "Play"
            },
            FLAction {
                keys: [Qt.Key_Menu]
                label: "Options"
            }
        ]
    }

    // Never mentions FLFocus, so it must carry no metadata
    Item {
        id: undeclared
    }

    // Only ever touched from JS, to prove an attachment can be created that way
    Item {
        id: setFromJs
    }

    // The FLButtonBase -> FLIconButton relationship: metadata declared in a base
    // component must reach instances of anything derived from it, or every
    // button in the app silently loses its ring
    FocusDerived {
        id: derived
    }

    property int plainButtonClicks: 0

    // An ordinary button that declares nothing, standing in for the title bar's
    // buttons
    Button {
        id: plainButton
        onClicked: testCase.plainButtonClicks++
    }

    function test_declares_metadata_inline() {
        compare(declared.FLFocus.showCursor, true);
        compare(declared.FLFocus.spacing, -8);
        verify(declared.FLFocus.holdEdges & FLFocus.Up);
        verify(declared.FLFocus.holdEdges & FLFocus.Down);
        verify(!(declared.FLFocus.holdEdges & FLFocus.Left));
    }

    function test_declares_a_list_of_actions_inline() {
        compare(declared.FLFocus.getActionCount(), 2);
        compare(declared.FLFocus.getAction(0).label, "Play");
        compare(declared.FLFocus.getAction(1).label, "Options");
    }

    // The whole point: one item, two buttons, two outcomes
    function test_separate_keys_reach_separate_actions() {
        compare(declared.FLFocus.getActionFor(Qt.Key_Select).label, "Play");
        compare(declared.FLFocus.getActionFor(Qt.Key_Return).label, "Play");
        compare(declared.FLFocus.getActionFor(Qt.Key_Menu).label, "Options");
        compare(declared.FLFocus.getActionFor(Qt.Key_Escape), null);
    }

    function test_actions_outlive_the_binding() {
        // A second read must see the same objects, not a rebuilt list
        const first = declared.FLFocus.getAction(0);
        compare(declared.FLFocus.getAction(0), first);
        compare(declared.FLFocus.getActionFor(Qt.Key_Select), first);
    }

    function test_triggering_an_action_reaches_its_handler() {
        let fired = 0;
        const action = declared.FLFocus.getActionFor(Qt.Key_Menu);
        action.triggered.connect(function () {
            fired++;
        });

        action.triggered();

        compare(fired, 1);
    }

    // An item that declared nothing must be distinguishable, which is what lets
    // the migration run typed and duck-typed lookups side by side
    function test_find_returns_null_for_an_item_that_declared_nothing() {
        compare(declared.FLFocus.find(undeclared), null);
        compare(declared.FLFocus.find(null), null);
    }

    function test_find_returns_the_attached_instance() {
        compare(declared.FLFocus.find(declared), declared.FLFocus);
    }

    // The zero-config path: an ordinary button declares no actions, so the
    // window activates it by calling click(). The Controls documentation lists
    // only toggle() under Methods, so pin that click() really is reachable —
    // if it were not, activation would fail silently rather than error
    function test_a_plain_button_is_activated_by_calling_click() {
        compare(typeof plainButton.click, "function");

        plainButton.click();

        compare(testCase.plainButtonClicks, 1);
    }

    function test_a_plain_button_declares_no_actions() {
        compare(declared.FLFocus.find(plainButton), null);
    }

    function test_metadata_declared_in_a_base_reaches_derived_instances() {
        verify(declared.FLFocus.find(derived) !== null);
        compare(derived.FLFocus.showCursor, true);
        compare(derived.FLFocus.spacing, -4);
    }

    function test_metadata_can_be_created_from_js() {
        compare(declared.FLFocus.find(setFromJs), null);

        setFromJs.FLFocus.barrier = true;

        compare(setFromJs.FLFocus.barrier, true);
        verify(declared.FLFocus.find(setFromJs) !== null);
    }

    // The focus ring reads each corner by name, so the four have to be reachable that way and read
    // as unset until something declares one
    Item {
        id: cornered

        FLFocus.showCursor: true
        FLFocus.radius: 10
        FLFocus.bottomLeftRadius: 0
    }

    Rectangle {
        id: plainRect
        width: 40
        height: 40
        radius: 6
    }

    Rectangle {
        id: corneredRect
        width: 40
        height: 40
        radius: 6
        topLeftRadius: 20
    }

    function test_cornersAreUnsetUntilDeclared() {
        const info = FLFocus.find(cornered);
        verify(info !== null);

        verify(isNaN(info.topLeftRadius), "an undeclared corner did not read as unset");
        verify(isNaN(info.topRightRadius));
        verify(isNaN(info.bottomRightRadius));
        compare(info.bottomLeftRadius, 0, "a corner declared as square did not keep 0");
    }

    // Zero has to survive as a real value: a square corner is the whole reason per-corner exists,
    // and NaN is what "unset" means
    function test_aSquareCornerIsNotTheSameAsAnUnsetOne() {
        const info = FLFocus.find(cornered);

        verify(!isNaN(info.bottomLeftRadius), "a corner set to 0 read as unset");
        compare(info.radius, 10);
    }

    // The ring looks corners up by name rather than by property, so this is the access it relies on
    function test_cornersCanBeReadByName() {
        const info = FLFocus.find(cornered);

        compare(info["bottomLeftRadius"], info.bottomLeftRadius, "reading a corner by name gave something else");
        verify(isNaN(info["topLeftRadius"]), "reading an unset corner by name did not read as unset");
    }

    // What an item says about its own corners, which the ring falls back to
    function test_anItemsOwnCornersReadByName() {
        compare(corneredRect["topLeftRadius"], 20, "a declared corner did not read back");
        compare(plainRect["topLeftRadius"], plainRect.radius, "an undeclared corner did not follow radius");
        compare(cornered["topLeftRadius"], undefined, "a plain Item claimed to have corners");
    }
}
