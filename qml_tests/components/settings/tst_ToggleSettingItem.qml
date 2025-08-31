import QtQuick
import QtQuick.Controls
import QtTest
import QMLFirelightTest 1.0

Item {
    id: root

    width: 400
    height: 400

    Component {
        id: toggleSettingItemComponent
        ToggleSettingItem {
            label: "Test Toggle"
            description: "Test description"
        }
    }

    Component {
        id: simpleToggleSettingItemComponent
        ToggleSettingItem {
            label: "Simple Toggle"
        }
    }

    TestCase {
        name: "ToggleSettingItemQMLTests"

        function test_initial_state() {
            var toggleItem = createTemporaryObject(toggleSettingItemComponent, root)
            
            compare(toggleItem.label, "Test Toggle", "Label should be set correctly")
            compare(toggleItem.description, "Test description", "Description should be set correctly")
            compare(toggleItem.checked, false, "Toggle should be unchecked by default")
            verify(toggleItem.control !== null, "Control should exist")
            verify(toggleItem.control instanceof Switch, "Control should be a Switch")
        }

        function test_checked_property() {
            var toggleItem = createTemporaryObject(toggleSettingItemComponent, root)
            
            compare(toggleItem.checked, false, "Should start unchecked")
            
            toggleItem.checked = true
            compare(toggleItem.checked, true, "Should be checked after setting to true")
            compare(toggleItem.control.checked, true, "Internal switch should also be checked")
            
            toggleItem.checked = false
            compare(toggleItem.checked, false, "Should be unchecked after setting to false")
            compare(toggleItem.control.checked, false, "Internal switch should also be unchecked")
        }

        function test_toggle_interaction() {
            var toggleItem = createTemporaryObject(toggleSettingItemComponent, root)
            
            compare(toggleItem.checked, false, "Should start unchecked")
            
            mouseClick(toggleItem.control)
            compare(toggleItem.checked, true, "Should be checked after clicking")
            
            mouseClick(toggleItem.control)
            compare(toggleItem.checked, false, "Should be unchecked after clicking again")
        }

        function test_switch_properties() {
            var toggleItem = createTemporaryObject(toggleSettingItemComponent, root)
            var theSwitch = toggleItem.control
            
            verify(theSwitch.focusPolicy === Qt.NoFocus, "Switch should have NoFocus policy")
            compare(theSwitch.enabled, true, "Switch should be enabled by default")
            verify(theSwitch.indicator !== null, "Switch should have an indicator")
        }

        function test_switch_indicator_visual_properties() {
            var toggleItem = createTemporaryObject(toggleSettingItemComponent, root)
            var theSwitch = toggleItem.control
            var indicator = theSwitch.indicator
            
            compare(indicator.implicitWidth, 50, "Indicator width should be 50")
            compare(indicator.implicitHeight, 28, "Indicator height should be 28")
            compare(indicator.radius, indicator.height / 2, "Indicator should have rounded corners")
        }

        function test_switch_indicator_colors_unchecked() {
            var toggleItem = createTemporaryObject(toggleSettingItemComponent, root)
            var theSwitch = toggleItem.control
            var indicator = theSwitch.indicator
            
            toggleItem.checked = false
            compare(indicator.color, "#ffffff", "Unchecked indicator should be white")
            compare(indicator.border.color, "#cccccc", "Unchecked border should be light gray")
        }

        function test_switch_indicator_colors_checked() {
            var toggleItem = createTemporaryObject(toggleSettingItemComponent, root)
            var theSwitch = toggleItem.control
            var indicator = theSwitch.indicator
            
            toggleItem.checked = true
            compare(indicator.color, "#17a81a", "Checked indicator should be green")
            compare(indicator.border.color, "#17a81a", "Checked border should be green")
        }

        function test_switch_handle_position() {
            var toggleItem = createTemporaryObject(toggleSettingItemComponent, root)
            var theSwitch = toggleItem.control
            var handle = theSwitch.indicator.children[0]
            
            toggleItem.checked = false
            compare(handle.x, 0, "Handle should be at left when unchecked")
            
            toggleItem.checked = true
            compare(handle.x, handle.parent.width - handle.width, "Handle should be at right when checked")
        }

        function test_switch_handle_visual_properties() {
            var toggleItem = createTemporaryObject(toggleSettingItemComponent, root)
            var theSwitch = toggleItem.control
            var handle = theSwitch.indicator.children[0]
            
            compare(handle.width, 26, "Handle width should be 26")
            compare(handle.height, 26, "Handle height should be 26")
            compare(handle.radius, handle.height / 2, "Handle should be circular")
            compare(handle.color, "#ffffff", "Handle should be white")
        }

        function test_switch_handle_border_colors() {
            var toggleItem = createTemporaryObject(toggleSettingItemComponent, root)
            var theSwitch = toggleItem.control
            var handle = theSwitch.indicator.children[0]
            
            toggleItem.checked = false
            compare(handle.border.color, "#999999", "Unchecked handle border should be dark gray")
            
            toggleItem.checked = true
            compare(handle.border.color, "#21be2b", "Checked handle border should be bright green")
        }

        function test_description_visibility() {
            var toggleItemWithDesc = createTemporaryObject(toggleSettingItemComponent, root)
            var toggleItemWithoutDesc = createTemporaryObject(simpleToggleSettingItemComponent, root)
            
            verify(toggleItemWithDesc.description !== "", "First item should have description")
            verify(toggleItemWithoutDesc.description === "", "Second item should have no description")
        }

        function test_hover_cursor() {
            var toggleItem = createTemporaryObject(toggleSettingItemComponent, root)
            var theSwitch = toggleItem.control
            var hoverHandler = null
            
            for (var i = 0; i < theSwitch.children.length; i++) {
                if (theSwitch.children[i].toString().indexOf("HoverHandler") !== -1) {
                    hoverHandler = theSwitch.children[i]
                    break
                }
            }
            
            verify(hoverHandler !== null, "Switch should have a HoverHandler")
            compare(hoverHandler.cursorShape, Qt.PointingHandCursor, "Should show pointing hand cursor on hover")
        }

        function test_enabled_disabled() {
            var toggleItem = createTemporaryObject(toggleSettingItemComponent, root)
            var theSwitch = toggleItem.control
            
            compare(theSwitch.enabled, true, "Switch should be enabled by default")
            
            toggleItem.enabled = false
            compare(theSwitch.enabled, false, "Switch should be disabled when parent is disabled")
            
            toggleItem.enabled = true
            compare(theSwitch.enabled, true, "Switch should be enabled when parent is enabled")
        }
    }
}