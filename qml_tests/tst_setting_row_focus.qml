// TODO: NEEDS REVIEW
import QtQuick
import QtTest
import QtQuick.Controls

// A settings row owns the focus for the control inside it: the control is NoFocus so arrows reach
// the row, and the row is what the cursor lands on. That only works if the row can take focus at all
TestCase {
    id: testCase
    name: "SettingRowFocusTests"

    width: 400
    height: 300

    visible: true
    when: windowShown

    // What a row declaring no focusPolicy is
    Component {
        id: plainScope

        FocusScope {
            width: 400
            height: 60
        }
    }

    // A row shaped like the settings rows: a FocusScope holding a control that refuses focus, taking
    // focus from a tap of its own
    Component {
        id: rowWithControl

        FocusScope {
            id: row
            width: 400
            height: 60
            focusPolicy: Qt.StrongFocus

            property int taps: 0

            TapHandler {
                onTapped: {
                    row.taps++;
                    row.forceActiveFocus(Qt.MouseFocusReason);
                }
            }

            Slider {
                id: theSlider
                objectName: "slider"
                anchors.fill: parent
                focusPolicy: Qt.NoFocus
            }
        }
    }

    // The same row taking focus from the control being pressed rather than from a tap of its own
    Component {
        id: rowFocusedByPress

        FocusScope {
            id: row
            width: 400
            height: 60
            focusPolicy: Qt.StrongFocus

            Slider {
                id: theSlider
                objectName: "slider"
                anchors.fill: parent
                focusPolicy: Qt.NoFocus

                onPressedChanged: {
                    if (theSlider.pressed) {
                        row.forceActiveFocus(Qt.MouseFocusReason);
                    }
                }
            }
        }
    }

    function test_theRowsOwnTapNeverArrivesOverAControl() {
        const row = createTemporaryObject(rowWithControl, testCase);
        verify(row !== null);

        mouseClick(row, 200, 30);

        compare(row.taps, 0, "the row's TapHandler did fire, so it is not what stops the row focusing");
    }

    function test_pressingTheControlFocusesTheRow() {
        const row = createTemporaryObject(rowFocusedByPress, testCase);
        verify(row !== null);

        mouseClick(row, 200, 30);

        verify(row.activeFocus, "pressing the control did not focus the row");

        const slider = findChild(row, "slider");
        verify(!slider.activeFocus, "the control took the focus the row should hold");
    }

    function test_aFocusScopeRefusesFocusUnlessItSaysOtherwise() {
        const scope = createTemporaryObject(plainScope, testCase);
        verify(scope !== null);

        compare(scope.focusPolicy, Qt.NoFocus, "a bare FocusScope was expected to refuse focus");
    }

    // The handle is a public part of a Slider, so a row can point the cursor at it from outside
    function test_aSlidersHandleIsReachableFromOutside() {
        const row = createTemporaryObject(rowWithControl, testCase);
        verify(row !== null);

        const slider = findChild(row, "slider");
        verify(slider !== null);
        verify(slider.handle !== null, "a Slider did not expose its handle");
        verify(slider.handle.width > 0, "the handle has no size to draw a cursor around");
    }

    //****************
    // what a row loses by not being a focus scope
    //****************

    // The row a settings item used to be: activeFocus covers a focused descendant
    Component {
        id: scopeRow

        FocusScope {
            width: 200
            height: 40
            focusPolicy: Qt.StrongFocus

            property alias inner: scopeSlider

            Slider {
                id: scopeSlider
                anchors.fill: parent
                focusPolicy: Qt.StrongFocus
            }
        }
    }

    // The row it is now: an AbstractButton, which does not
    Component {
        id: delegateRow

        ItemDelegate {
            width: 200
            height: 40

            property alias inner: delegateSlider

            Slider {
                id: delegateSlider
                anchors.fill: parent
                focusPolicy: Qt.StrongFocus
            }
        }
    }

    function test_aFocusScopeCoversItsChildsFocus() {
        const row = createTemporaryObject(scopeRow, testCase);
        verify(row !== null);

        row.inner.forceActiveFocus();

        verify(row.inner.activeFocus, "the child never took focus");
        verify(row.activeFocus, "a FocusScope did not report activeFocus while its child held it");
    }

    // The reason a row that hands focus to its control needs a proxy term in its highlight: an
    // ItemDelegate goes dark the moment the control takes over
    function test_anItemDelegateDoesNotCoverItsChildsFocus() {
        const row = createTemporaryObject(delegateRow, testCase);
        verify(row !== null);

        row.inner.forceActiveFocus();

        verify(row.inner.activeFocus, "the child never took focus");
        verify(!row.activeFocus, "an ItemDelegate reported activeFocus for its child, so the proxy term is dead code");
    }

    // And the row still reports focus for itself, which is what `rowHoldsFocus` now means
    function test_anItemDelegateStillReportsItsOwnFocus() {
        const row = createTemporaryObject(delegateRow, testCase);
        verify(row !== null);

        row.forceActiveFocus();

        verify(row.activeFocus, "the row could not take focus itself");
        verify(!row.inner.activeFocus, "focus leaked into the control");
    }
}
