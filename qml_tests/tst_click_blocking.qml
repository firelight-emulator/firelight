// TODO: NEEDS REVIEW
import QtQuick
import QtTest
import QtQuick.Controls
import QtQuick.Layouts

// Whether a panel laid over something else stops a press reaching it. A surface that leaks presses
// looks identical to one that works until you click the thing behind it
TestCase {
    id: testCase
    name: "ClickBlockingTests"

    width: 400
    height: 300

    // A TestCase is invisible by default, and an invisible item is never hit-tested, so a mouse
    // event lands nowhere
    visible: true
    when: windowShown

    // A Pane whose background holds the blocker, which is where a Control's background sits: behind
    // its content, in front of whatever the panel covers
    Component {
        id: paneWithBlocker

        Item {
            width: 400
            height: 300

            property int underneathClicks: 0

            MouseArea {
                anchors.fill: parent
                onClicked: parent.underneathClicks++
            }

            Pane {
                anchors.centerIn: parent
                width: 200
                height: 100

                background: Rectangle {
                    color: "red"

                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.AllButtons
                    }
                }
            }
        }
    }

    // The same panel with nothing accepting presses, which is what a bare Pane is
    Component {
        id: paneWithoutBlocker

        Item {
            width: 400
            height: 300

            property int underneathClicks: 0

            MouseArea {
                anchors.fill: parent
                onClicked: parent.underneathClicks++
            }

            Pane {
                anchors.centerIn: parent
                width: 200
                height: 100

                background: Rectangle {
                    color: "red"
                }
            }
        }
    }

    // A TapHandler is how the settings rows answer a click. It takes a passive grab by design, so
    // it is not a blocker — this pins that, because it is the thing that looks like it should work
    Component {
        id: paneWithTapHandler

        Item {
            width: 400
            height: 300

            property int underneathClicks: 0
            property int tapped: 0

            MouseArea {
                anchors.fill: parent
                onClicked: parent.underneathClicks++
            }

            Pane {
                id: panel
                anchors.centerIn: parent
                width: 200
                height: 100

                background: Rectangle {
                    color: "red"
                }

                TapHandler {
                    onTapped: panel.parent.tapped++
                }
            }
        }
    }

    Component {
        id: bareMouseArea

        Item {
            width: 400
            height: 300

            property int clicks: 0

            MouseArea {
                anchors.fill: parent
                onClicked: parent.clicks++
            }
        }
    }

    // What the grid actually is underneath the panel: a Button whose TapHandler does the selecting.
    // A handler takes a passive grab, which is granted on a different pass from item acceptance, so
    // whether a panel above stops it is not answered by the MouseArea case
    Component {
        id: paneOverTapHandler

        Item {
            width: 400
            height: 300

            property int underneathTaps: 0

            Button {
                anchors.fill: parent

                TapHandler {
                    onSingleTapped: underneathTaps++
                }
            }

            Pane {
                anchors.centerIn: parent
                width: 200
                height: 100

                background: Rectangle {
                    color: "red"
                }
            }
        }
    }

    // A Pane sized only by its content, holding a layout that fills it — content sizes to the Pane
    // and the Pane sizes to the content
    Component {
        id: paneWithFilledContent

        Item {
            width: 400
            height: 300

            property alias panel: panel
            property alias row: row

            Pane {
                id: panel
                width: 300
                anchors.left: parent.left
                anchors.bottom: parent.bottom

                ColumnLayout {
                    anchors.fill: parent

                    Rectangle {
                        id: row
                        Layout.fillWidth: true
                        implicitHeight: 80
                        color: "blue"
                    }
                }
            }
        }
    }

    function test_aFilledLayoutStillSizesThePane() {
        const scene = createTemporaryObject(paneWithFilledContent, testCase);
        verify(scene !== null);

        // The row has to be inside what the panel will hit-test, or the panel draws over the grid
        // without covering it
        const rowBottom = scene.row.mapToItem(scene.panel, 0, scene.row.height).y;

        verify(scene.panel.height > 0, "the panel collapsed to nothing");
        verify(rowBottom <= scene.panel.height, "the row hangs " + (rowBottom - scene.panel.height) + "px past the panel");
    }

    function test_paneOverATapHandler() {
        const scene = createTemporaryObject(paneOverTapHandler, testCase);
        verify(scene !== null);

        mouseClick(scene, 200, 150);

        compare(scene.underneathTaps, 0, "the panel did not stop the TapHandler under it");
    }

    function test_sanityTheHarnessDeliversClicks() {
        const scene = createTemporaryObject(bareMouseArea, testCase);
        verify(scene !== null);
        compare(scene.width, 400);
        verify(scene.visible);

        mouseClick(scene, 200, 150);

        compare(scene.clicks, 1, "no click reached a bare MouseArea, so nothing below can be trusted");
    }

    function test_backgroundMouseAreaBlocks() {
        const scene = createTemporaryObject(paneWithBlocker, testCase);
        verify(scene !== null);

        mouseClick(scene, 200, 150);

        compare(scene.underneathClicks, 0, "the press went through the panel to what it covers");
    }

    function test_aBarePaneAlreadyBlocks() {
        const scene = createTemporaryObject(paneWithoutBlocker, testCase);
        verify(scene !== null);

        mouseClick(scene, 200, 150);

        compare(scene.underneathClicks, 0, "a Pane with nothing in it let the press through");
    }

    function test_aPaneStillBlocksWhileItsHandlerSeesTheTap() {
        const scene = createTemporaryObject(paneWithTapHandler, testCase);
        verify(scene !== null);

        mouseClick(scene, 200, 150);

        compare(scene.tapped, 1, "the TapHandler never saw the tap");
        compare(scene.underneathClicks, 0, "the press reached what the panel covers");
    }
}
