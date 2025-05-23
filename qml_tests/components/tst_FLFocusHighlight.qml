import QtQuick
import QtQuick.Controls
import QtTest
import QMLFirelightTest 1.0

Item {
    id: root

    width: 400
    height: 400

    Component {
        id: testItemComponent
        Rectangle {
            id: testItem
            width: 100
            height: 100
            radius: 10
            property bool showGlobalCursor: true
            property Item globalCursorProxy: null
        }
    }

    Component {
        id: testItemWithBackgroundComponent
        Button {
            id: testItemWithBackground
            width: 100
            height: 100
            background: Rectangle {
                radius: 5
            }
            property bool showGlobalCursor: true
            property Item globalCursorProxy: null
        }
    }

    Component {
        id: testItemWithProxyComponent
        Rectangle {
            id: testItemWithProxy
            width: 100
            height: 100
            radius: 10
            property bool showGlobalCursor: true
            property Item globalCursorProxy: Rectangle {
                id: proxyRect
                width: 100
                height: 100
                radius: 15
            }
        }
    }

    Component {
        id: activeFocusHighlightComponent
        FLFocusHighlight {}
    }

    TestCase {
        name: "FLFocusHighlightQMLTests"

        function test_initial_state() {
            var item = createTemporaryObject(testItemComponent, root)
            var highlight = createTemporaryObject(activeFocusHighlightComponent, root, {
                target: item,
                usingMouse: false
            })

            compare(highlight.visible, true, "Highlight should be visible when usingMouse is false")
            compare(highlight.anchors.fill, item, "Highlight should fill its target")
            compare(highlight.anchors.margins, -4, "Highlight margins should be -4")
            compare(highlight.border.width, 2, "Highlight border width should be 2")
            compare(highlight.radius, 14, "Highlight radius should be 14 (10 + abs(-4))")
        }

        function test_radius_with_background() {
            var itemWithBackground = createTemporaryObject(testItemWithBackgroundComponent, root)
            var highlight = createTemporaryObject(activeFocusHighlightComponent, root, {
                target: itemWithBackground,
                usingMouse: false
            })

            compare(highlight.radius, 9, "Highlight radius should be 9 (5 + abs(-4)) when parent has background with radius")
        }

        function test_radius_without_radius_or_background() {
            var item = createTemporaryObject(testItemComponent, root);
            var highlight = createTemporaryObject(activeFocusHighlightComponent, root, {
                target: item,
                usingMouse: false
            })

            item.radius = 0;

            compare(highlight.radius, 4, "Highlight radius should be 4 (abs(-4)) when parent has no radius or background radius")
        }

        function test_visibility_with_mouse() {
            var item = createTemporaryObject(testItemComponent, root)
            var highlight = createTemporaryObject(activeFocusHighlightComponent, root, {
                target: item,
                usingMouse: true
            })

            compare(highlight.visible, false, "Highlight should be invisible when usingMouse is true")
        }

        function test_target_change_with_global_cursor() {
            var itemWithProxy = createTemporaryObject(testItemWithProxyComponent, root)
            var highlight = createTemporaryObject(activeFocusHighlightComponent, root, {
                target: itemWithProxy,
                usingMouse: false
            })

            compare(highlight.parent, itemWithProxy.globalCursorProxy, "Highlight parent should be globalCursorProxy when showGlobalCursor is true")
            compare(highlight.radius, 19, "Highlight radius should be 19 (15 + abs(-4))")

            itemWithProxy.globalCursorProxy = null;
            highlight.target = null; // TODO: Support this use case
            highlight.target = itemWithProxy;

            compare(highlight.parent, itemWithProxy, "Highlight parent should be item when showGlobalCursor is false")
            compare(highlight.radius, 14, "Highlight radius should be 14 (10 + abs(-4))")
        }

        function test_target_change_with_null_parent() {
            var item = createTemporaryObject(testItemComponent, root)
            var highlight = createTemporaryObject(activeFocusHighlightComponent, null, {
                target: null,
                usingMouse: false
            })

            compare(highlight.parent, null, "Highlight parent should be null when parent is null")
            compare(highlight.radius, 4, "Highlight radius should be 4 (abs(-4)) when parent is null")
            compare(highlight.visible, false, "Highlight should be invisible when parent is null")
        }

        function test_target_change_radius() {
            var item = createTemporaryObject(testItemComponent, root);
            var highlight = createTemporaryObject(activeFocusHighlightComponent, root, {
                target: item,
                usingMouse: false
            });

            item.radius = 20;
            highlight.target = item;
            compare(highlight.radius, 24, "Radius should update when target radius changes");
        }

        function test_target_change_background_radius() {
            var item = createTemporaryObject(testItemWithBackgroundComponent, root);
            var highlight = createTemporaryObject(activeFocusHighlightComponent, root, {
                target: item,
                usingMouse: false
            });

            item.background.radius = 10;
            highlight.target = item;
            compare(highlight.radius, 14, "Radius should update when target background radius changes");
        }

        function test_border_properties() {
            var item = createTemporaryObject(testItemComponent, root)
            var highlight = createTemporaryObject(activeFocusHighlightComponent, root, {
                target: item,
                usingMouse: false,
                borderColor: "red",
                borderWidth: 4
            })

            compare(highlight.border.color, "#ff0000", "Border color should match custom value")
            compare(highlight.border.width, 4, "Border width should match custom value")
        }

        function test_bounce_amplitude_property() {
            var item = createTemporaryObject(testItemComponent, root)
            var highlight = createTemporaryObject(activeFocusHighlightComponent, root, {
                target: item,
                usingMouse: false,
                bounceAmplitude: 24
            })

            compare(highlight.bounceAmplitude, 24, "Bounce amplitude should match custom value")
        }

        // function test_show_global_cursor_change() {
        //     var item = createTemporaryObject(testItemComponent, root)
        //     var highlight = createTemporaryObject(activeFocusHighlightComponent, root, {
        //         target: item,
        //         usingMouse: false
        //     })
        //
        //     item.showGlobalCursor = false
        //     highlight.target = item
        //     compare(highlight.parent, null, "Parent should be null when showGlobalCursor is false")
        //
        //     item.showGlobalCursor = true
        //     highlight.target = item
        //     compare(highlight.parent, item, "Parent should be item when showGlobalCursor is true")
        // }

        function test_visibility_parent_change() {
            var item = createTemporaryObject(testItemComponent, root)
            var highlight = createTemporaryObject(activeFocusHighlightComponent, root, {
                target: item,
                usingMouse: false
            })

            compare(highlight.visible, true, "Highlight should be visible initially")

            highlight.parent = null
            compare(highlight.visible, false, "Highlight should be invisible when parent is null")

            highlight.parent = item
            compare(highlight.visible, true, "Highlight should be visible when parent is restored")
        }

        function test_target_null_to_valid() {
            var item = createTemporaryObject(testItemComponent, root)
            var highlight = createTemporaryObject(activeFocusHighlightComponent, null, {
                target: null,
                usingMouse: false
            })

            compare(highlight.visible, false, "Highlight should be invisible with null target")

            highlight.target = item
            compare(highlight.visible, true, "Highlight should become visible with valid target")
            compare(highlight.parent, item, "Parent should be set to target")
        }
    }
}