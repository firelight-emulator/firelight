// TODO: NEEDS REVIEW
import QtQuick
import QtTest
import Firelight 1.0

// Covers how a press finds the action answering it, and how the prompt bar folds the actions it
// can reach into one entry per label
TestCase {
    id: testCase
    name: "FocusActionLookup"

    // A container declaring an action over a child declaring its own: the shape a grid of tiles
    // inside a scrolling view makes
    Item {
        id: container

        FLFocus.actions: [
            FLAction {
                keys: [Qt.Key_PageDown]
                label: "Jump down"
            }
        ]

        Item {
            id: tile

            FLFocus.actions: [
                FLAction {
                    keys: [Qt.Key_Return]
                    label: "Play"
                }
            ]
        }
    }

    // The same key declared at both levels, to say which one a press runs
    Item {
        id: outerSameKey

        FLFocus.actions: [
            FLAction {
                keys: [Qt.Key_Return]
                label: "Outer"
            }
        ]

        Item {
            id: innerSameKey

            FLFocus.actions: [
                FLAction {
                    keys: [Qt.Key_Return]
                    label: "Inner"
                }
            ]
        }
    }

    // Navigation cannot leave a barrier, so neither can a lookup
    Item {
        id: beyondBarrier

        FLFocus.actions: [
            FLAction {
                keys: [Qt.Key_PageUp]
                label: "Unreachable"
            }
        ]

        Item {
            id: barrier

            FLFocus.barrier: true

            Item {
                id: insideBarrier
            }
        }
    }

    // A disabled action with nothing else answering its key
    Item {
        id: onlyDisabled

        FLFocus.actions: [
            FLAction {
                keys: [Qt.Key_PageUp]
                label: "Jump up"
                enabled: false
            }
        ]
    }

    // A disabled action nearer than an enabled one answering the same press
    Item {
        id: enabledAncestor

        FLFocus.actions: [
            FLAction {
                keys: [Qt.Key_PageUp]
                label: "Outer jump"
            }
        ]

        Item {
            id: disabledChild

            FLFocus.actions: [
                FLAction {
                    keys: [Qt.Key_PageUp]
                    label: "Inner jump"
                    enabled: false
                }
            ]
        }
    }

    // One label bound two ways, the second under a modifier
    Item {
        id: twoWaysIn

        FLFocus.actions: [
            FLAction {
                keys: [Qt.Key_Menu]
                label: "Menu"
            },
            FLAction {
                keys: [Qt.Key_Return]
                modifiers: Qt.ControlModifier
                label: "Menu"
            }
        ]
    }

    // One label bound two ways with only the second still answering
    Item {
        id: partlyDisabled

        FLFocus.actions: [
            FLAction {
                keys: [Qt.Key_Menu]
                label: "Menu"
                enabled: false
            },
            FLAction {
                keys: [Qt.Key_Home]
                label: "Menu"
            }
        ]
    }

    // Every way in is off
    Item {
        id: whollyDisabled

        FLFocus.actions: [
            FLAction {
                keys: [Qt.Key_Menu]
                label: "Menu"
                enabled: false
            }
        ]
    }

    function findFor(item, key, modifiers) {
        return testCase.FLFocus.findActionFor(item, key, modifiers === undefined ? Qt.NoModifier : modifiers);
    }

    function labelsOf(item) {
        return testCase.FLFocus.collectActions(item).map(action => action.label);
    }

    function groupNamed(item, label) {
        const groups = testCase.FLFocus.collectActionGroups(item).filter(group => group.label === label);
        return groups.length > 0 ? groups[0] : null;
    }

    // A press on a tile has to reach what the view around it declared, which is the same set the
    // prompt bar lists
    function test_a_press_reaches_an_action_declared_by_a_container() {
        const action = testCase.findFor(tile, Qt.Key_PageDown);

        verify(action !== null, "the container's action was not reachable from the tile");
        compare(action.label, "Jump down");
    }

    function test_a_press_still_reaches_the_items_own_action() {
        compare(testCase.findFor(tile, Qt.Key_Return).label, "Play");
    }

    function test_nothing_answers_a_key_nobody_declared() {
        compare(testCase.findFor(tile, Qt.Key_F12), null);
    }

    function test_the_nearest_declaration_of_a_key_wins() {
        compare(testCase.findFor(innerSameKey, Qt.Key_Return).label, "Inner");
    }

    function test_a_lookup_does_not_leave_a_barrier() {
        compare(testCase.findFor(insideBarrier, Qt.Key_PageUp), null);
    }

    function test_a_disabled_action_answers_nothing() {
        compare(testCase.findFor(onlyDisabled, Qt.Key_PageUp), null);
    }

    // Listed rather than hidden, so the prompt reads as the press being off rather than absent
    function test_a_disabled_action_is_listed_when_nothing_else_answers() {
        compare(testCase.labelsOf(onlyDisabled), ["Jump up"]);
    }

    // A press falls through a disabled action to an enabled one further out, so listing the
    // disabled one would name a prompt that is not what runs
    function test_a_disabled_action_gives_way_to_an_enabled_one_on_the_same_key() {
        compare(testCase.labelsOf(disabledChild), ["Outer jump"]);
        compare(testCase.findFor(disabledChild, Qt.Key_PageUp).label, "Outer jump");
    }

    function test_one_label_bound_twice_is_one_group() {
        const groups = testCase.FLFocus.collectActionGroups(twoWaysIn);

        compare(groups.length, 1);
        compare(groups[0].label, "Menu");
        compare(groups[0].bindings.length, 2);
    }

    function test_a_group_carries_each_ways_key_and_modifiers() {
        const bindings = testCase.groupNamed(twoWaysIn, "Menu").bindings;

        compare(bindings[0].key, Qt.Key_Menu);
        compare(bindings[0].modifiers, Qt.NoModifier);
        compare(bindings[1].key, Qt.Key_Return);
        compare(bindings[1].modifiers, Qt.ControlModifier);
    }

    // What the prompt draws when a key has no icon of its own
    function test_a_binding_carries_the_way_it_is_written() {
        const bindings = testCase.groupNamed(twoWaysIn, "Menu").bindings;

        verify(bindings[1].text.length > 0, "the modified binding was not written out");
    }

    function test_a_group_is_reachable_while_any_way_in_answers() {
        compare(testCase.groupNamed(partlyDisabled, "Menu").enabled, true);
    }

    function test_a_group_with_every_way_in_off_is_not_reachable() {
        compare(testCase.groupNamed(whollyDisabled, "Menu").enabled, false);
    }

    function test_an_unlabelled_action_is_not_a_group() {
        compare(testCase.FLFocus.collectActionGroups(barrier).length, 0);
    }
}
