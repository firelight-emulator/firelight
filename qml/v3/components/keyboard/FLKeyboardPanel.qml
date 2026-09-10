// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Layouts
import Firelight 1.0
import "keyboard_layout.js" as KeyLayout

// TODO
// The key grid. Owns which page is showing and the shift state, and reports what a press
// means rather than editing anything itself
FocusScope {
    id: root

    objectName: "FLKeyboardPanel"

    // TODO
    // Return inserts a newline when this is set, and is inert otherwise
    property bool multiline: false

    // TODO
    // What the committing key reads
    property string acceptLabel: qsTr("OK")

    // TODO
    // Whether the committing key is available
    property bool canAccept: true

    signal characterEntered(string character)
    signal backspacePressed
    signal returnPressed
    signal acceptPressed

    readonly property int shiftOff: KeyLayout.SHIFT_OFF
    readonly property int shiftArmed: KeyLayout.SHIFT_ARMED
    readonly property int shiftLocked: KeyLayout.SHIFT_LOCKED

    property int shiftState: root.shiftOff
    property string page: "abc"

    readonly property bool uppercase: root.shiftState !== root.shiftOff

    // TODO
    // The character rows plus the function row beneath them
    readonly property int gridRows: KeyLayout.CHARACTER_ROWS + 1
    readonly property int gridColumns: KeyLayout.CHARACTER_COLUMNS
    readonly property int functionRow: KeyLayout.CHARACTER_ROWS

    // TODO
    // Every seam in the keyboard, so a utility key can be sized in whole grid rows
    readonly property int keyGap: 6
    readonly property int sectionGap: 10

    // TODO
    // Positions only, and never rebuilt: a model that changed with the page would destroy the
    // focused key on every shift press and strand the cursor
    readonly property var characterCells: KeyLayout.characterCells()

    function characterAt(row: int, column: int): string {
        return KeyLayout.characterAt(root.page, root.uppercase, row, column);
    }

    // TODO
    // Off arms on the first press, arms to locked on a second press before any character,
    // and clears from locked
    function pressShift() {
        root.shiftState = KeyLayout.nextShift(root.shiftState);
    }

    function showPage(name: string) {
        root.page = name;
        root.shiftState = root.shiftOff;
    }

    function enterCharacter(character: string) {
        root.characterEntered(character);
        root.shiftState = KeyLayout.shiftAfterCharacter(root.shiftState);
    }

    function focusKeyboard() {
        characterRepeater.itemAt(0).forceActiveFocus();
    }

    implicitHeight: keyboard.implicitHeight

    FLRowLayout {
        id: keyboard
        anchors.fill: parent
        spacing: 10

        FLColumnLayout {
            id: characterColumn

            Layout.fillWidth: true
            Layout.fillHeight: true

            GridLayout {
                id: characterGrid
                Layout.fillHeight: true
                Layout.fillWidth: true

                rows: root.gridRows
                columns: root.gridColumns
                rowSpacing: root.keyGap
                columnSpacing: root.keyGap

                readonly property real rowHeight: (characterGrid.height - (root.gridRows - 1) * root.keyGap) / root.gridRows

                Repeater {
                    id: characterRepeater
                    model: root.characterCells

                    FLKeyboardKey {
                        required property var modelData

                        label: root.characterAt(modelData.row, modelData.column)
                        Layout.row: modelData.row
                        Layout.column: modelData.column

                        onActivated: root.enterCharacter(label)
                    }
                }

                FLKeyboardKey {
                    glyphName: "shift"
                    actionLabel: qsTr("Shift")
                    showIndicator: true
                    indicatorActive: root.shiftState === root.shiftLocked
                    glyphFilled: root.shiftState !== root.shiftOff

                    Layout.row: root.functionRow
                    Layout.column: 0
                    Layout.columnSpan: 2
                    Layout.topMargin: root.sectionGap - root.keyGap

                    onActivated: root.pressShift()
                }

                FLKeyboardKey {
                    glyphName: "abc"
                    actionLabel: qsTr("Letters")
                    active: root.page === "abc"

                    Layout.row: root.functionRow
                    Layout.column: 2
                    Layout.topMargin: root.sectionGap - root.keyGap

                    onActivated: root.showPage("abc")
                }

                FLKeyboardKey {
                    label: "#+="
                    actionLabel: qsTr("Symbols")
                    active: root.page === "sym"

                    Layout.row: root.functionRow
                    Layout.column: 3
                    Layout.topMargin: root.sectionGap - root.keyGap

                    onActivated: root.showPage("sym")
                }

                FLKeyboardKey {
                    glyphName: "space_bar"
                    actionLabel: qsTr("Space")

                    Layout.row: root.functionRow
                    Layout.column: 4
                    Layout.columnSpan: root.gridColumns - 4
                    Layout.topMargin: root.sectionGap - root.keyGap

                    onActivated: root.enterCharacter(" ")
                }
            }
        }

        FLColumnLayout {
            Layout.fillHeight: true
            Layout.preferredWidth: 90
            spacing: root.keyGap

            FLKeyboardKey {
                glyphName: "backspace"
                actionLabel: qsTr("Backspace")
                allowAutoRepeat: true
                Layout.fillWidth: true
                Layout.preferredHeight: characterGrid.rowHeight
                Layout.maximumHeight: characterGrid.rowHeight

                onActivated: root.backspacePressed()
            }

            FLKeyboardKey {
                glyphName: "keyboard_return"
                actionLabel: qsTr("New line")
                canInteract: root.multiline
                Layout.fillWidth: true
                Layout.preferredHeight: characterGrid.rowHeight * 2 + root.keyGap
                Layout.maximumHeight: characterGrid.rowHeight * 2 + root.keyGap

                onActivated: root.returnPressed()
            }

            FLKeyboardKey {
                id: acceptKey

                label: root.acceptLabel
                actionLabel: root.acceptLabel
                primary: true
                canInteract: root.canAccept
                Layout.fillWidth: true
                Layout.preferredHeight: characterGrid.rowHeight * 2 + root.keyGap
                Layout.maximumHeight: characterGrid.rowHeight * 2 + root.keyGap

                onActivated: root.acceptPressed()
            }
        }
    }

    // GridLayout {
    //     id: grid
    //
    //     objectName: "FLKeyboardPanel|grid"
    //     anchors.fill: parent
    //
    //     // TODO
    //     // 11 character columns plus a double-width column for backspace, return and accept
    //     columns: 13
    //     rows: 5
    //     columnSpacing: AppStyle.keyboardKeyGap
    //     rowSpacing: AppStyle.keyboardKeyGap
    //
    //     FLFocus.container: true
    //
    //     // TODO
    //     // A held direction stops at the top and bottom of the grid rather than shooting out
    //     FLFocus.holdEdges: FLFocus.Vertical
    //
    //     Repeater {
    //         model: root.characterCells
    //
    //         FLKeyboardKey {
    //             required property var modelData
    //
    //             label: root.characterAt(modelData.row, modelData.column)
    //             Layout.row: modelData.row
    //             Layout.column: modelData.column
    //             Layout.fillWidth: true
    //             Layout.preferredHeight: AppStyle.keyboardKeyHeight
    //
    //             onActivated: root.enterCharacter(label)
    //         }
    //     }
    //
    //     FLKeyboardKey {
    //         glyphName: "backspace"
    //         actionLabel: qsTr("Backspace")
    //         Layout.row: 0
    //         Layout.column: 11
    //         Layout.columnSpan: 2
    //         Layout.fillWidth: true
    //         Layout.preferredHeight: AppStyle.keyboardKeyHeight
    //
    //         onActivated: root.backspacePressed()
    //     }
    //
    //     FLKeyboardKey {
    //         glyphName: "keyboard_return"
    //         actionLabel: qsTr("New line")
    //         canInteract: root.multiline
    //         Layout.row: 1
    //         Layout.column: 11
    //         Layout.columnSpan: 2
    //         Layout.rowSpan: 2
    //         Layout.fillWidth: true
    //         Layout.fillHeight: true
    //
    //         onActivated: root.returnPressed()
    //     }
    //
    //     FLKeyboardKey {
    //         id: acceptKey
    //
    //         label: root.acceptLabel
    //         actionLabel: root.acceptLabel
    //         primary: true
    //         canInteract: root.canAccept
    //         Layout.row: 3
    //         Layout.column: 11
    //         Layout.columnSpan: 2
    //         Layout.rowSpan: 2
    //         Layout.fillWidth: true
    //         Layout.fillHeight: true
    //
    //         onActivated: root.acceptPressed()
    //     }
    //
    //
    // }
}
