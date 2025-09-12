import QtQuick
import QtTest
import QtQuick.Controls
import QMLFirelightTest 1.0

TestCase {
    id: testCase
    name: "IconButtonTests"
    
    width: 400
    height: 300
    
    // Component {
    //     id: iconButtonComponent
    //     IconButton {
    //         // icon.source: "qrc:/icons/test-icon.svg"
    //     }
    // }
    //
    // Component {
    //     id: signalSpy
    //     SignalSpy {}
    // }
    
    // function test_defaultProperties() {
    //     let button = createTemporaryObject(iconButtonComponent, testCase)
    //     verify(button !== null, "Button should be created")
    //
    //     compare(button.height, 50, "Default height should be 50")
    //     compare(button.width, 50, "Default width should be 50")
    //     compare(button.icon.width, 32, "Default icon width should be 32")
    //     compare(button.icon.height, 32, "Default icon height should be 32")
    //     compare(button.icon.color, "white", "Default icon color should be white")
    //     verify(button.flat, "Button should be flat by default")
    //     verify(button.hoverEnabled, "Hover should be enabled by default")
    //     verify(button.showGlobalCursor, "showGlobalCursor should be true by default")
    //     compare(button.globalCursorSpacing, 1, "globalCursorSpacing should be 1 by default")
    // }
    //
    // function test_clickable() {
    //     let button = createTemporaryObject(iconButtonComponent, testCase)
    //     let clickSpy = createTemporaryObject(signalSpy, testCase, {
    //         target: button,
    //         signalName: "clicked"
    //     })
    //
    //     verify(button !== null, "Button should be created")
    //     verify(clickSpy.valid, "Click spy should be valid")
    //
    //     mouseClick(button)
    //     compare(clickSpy.count, 1, "Button should emit clicked signal when clicked")
    // }
    //
    // function test_hoverBehavior() {
    //     let button = createTemporaryObject(iconButtonComponent, testCase)
    //     verify(button !== null, "Button should be created")
    //
    //     // Initially not hovered
    //     verify(!button.hovered, "Button should not be hovered initially")
    //
    //     // Mouse enter
    //     mouseMove(button, button.width / 2, button.height / 2)
    //     verify(button.hovered, "Button should be hovered when mouse is over it")
    //
    //     // Mouse leave
    //     mouseMove(button, -1, -1)
    //     verify(!button.hovered, "Button should not be hovered when mouse leaves")
    // }
    //
    // function test_backgroundColorChanges() {
    //     let button = createTemporaryObject(iconButtonComponent, testCase)
    //     verify(button !== null, "Button should be created")
    //
    //     let background = button.background
    //     verify(background !== null, "Background should exist")
    //
    //     // Test hover state
    //     mouseMove(button, button.width / 2, button.height / 2)
    //     verify(button.hovered, "Button should be hovered")
    //     // Background color should change when hovered (ColorPalette.neutral100)
    //
    //     // Test pressed state
    //     mousePress(button, button.width / 2, button.height / 2)
    //     verify(button.pressed, "Button should be pressed")
    //     compare(background.opacity, 0.14, "Background opacity should be 0.14 when pressed")
    //
    //     mouseRelease(button, button.width / 2, button.height / 2)
    //     verify(!button.pressed, "Button should not be pressed after release")
    //     compare(background.opacity, 0.23, "Background opacity should be 0.23 when not pressed")
    // }
    //
    // function test_tooltipVisibility() {
    //     let button = createTemporaryObject(iconButtonComponent, testCase)
    //     verify(button !== null, "Button should be created")
    //
    //     let tooltip = null
    //     // Find the ToolTip child
    //     for (let i = 0; i < button.children.length; i++) {
    //         if (button.children[i].toString().indexOf("ToolTip") !== -1) {
    //             tooltip = button.children[i]
    //             break
    //         }
    //     }
    //
    //     verify(tooltip !== null, "ToolTip should exist")
    //
    //     // Initially not visible (not hovered, not focused)
    //     verify(!tooltip.visible, "ToolTip should not be visible initially")
    //
    //     // Test hover visibility
    //     mouseMove(button, button.width / 2, button.height / 2)
    //     verify(tooltip.visible, "ToolTip should be visible when hovered")
    //
    //     // Test focus visibility
    //     mouseMove(button, -1, -1)
    //     button.forceActiveFocus()
    //     verify(tooltip.visible, "ToolTip should be visible when focused")
    // }
    //
    // function test_customProperties() {
    //     let button = createTemporaryObject(iconButtonComponent, testCase, {
    //         showGlobalCursor: false,
    //         globalCursorSpacing: 5
    //     })
    //
    //     verify(button !== null, "Button should be created")
    //     verify(!button.showGlobalCursor, "showGlobalCursor should be false")
    //     compare(button.globalCursorSpacing, 5, "globalCursorSpacing should be 5")
    // }
    //
    // function test_keyboardInteraction() {
    //     let button = createTemporaryObject(iconButtonComponent, testCase)
    //     let clickSpy = createTemporaryObject(signalSpy, testCase, {
    //         target: button,
    //         signalName: "clicked"
    //     })
    //
    //     verify(button !== null, "Button should be created")
    //     button.forceActiveFocus()
    //     verify(button.activeFocus, "Button should have active focus")
    //
    //     // Test space key activation
    //     keyPress(Qt.Key_Space)
    //     keyRelease(Qt.Key_Space)
    //     compare(clickSpy.count, 1, "Button should be clicked with space key")
    //
    //     // Test enter key activation
    //     keyPress(Qt.Key_Return)
    //     keyRelease(Qt.Key_Return)
    //     compare(clickSpy.count, 2, "Button should be clicked with enter key")
    // }
    //
    // function test_tooltipContent() {
    //     let button = createTemporaryObject(iconButtonComponent, testCase)
    //     verify(button !== null, "Button should be created")
    //
    //     let tooltip = null
    //     for (let i = 0; i < button.children.length; i++) {
    //         if (button.children[i].toString().indexOf("ToolTip") !== -1) {
    //             tooltip = button.children[i]
    //             break
    //         }
    //     }
    //
    //     verify(tooltip !== null, "ToolTip should exist")
    //     verify(tooltip.contentItem !== null, "ToolTip contentItem should exist")
    //
    //     let tooltipText = tooltip.contentItem
    //     compare(tooltipText.text, "Sort", "ToolTip text should be 'Sort'")
    //     compare(tooltipText.color, "orange", "ToolTip text color should be orange")
    // }
}