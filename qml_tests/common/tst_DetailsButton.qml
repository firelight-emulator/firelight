import QtQuick 2.3
import QtTest 1.0
import QMLFirelightTest

Item {
    width: 400
    height: 400

    DetailsButton {
        id: button
        width: 48
        height: 48
    }

    TestCase {
        name: "DetailsButton"
        when: windowShown

        function test_background_color() {
            compare(button.background.color, "#00000000", "Background is transparent")
            mouseMove(button)
            verify(button.hovered, "Button is hovered")
            compare(button.background.color, "#ffffff", "Background color is correct 2")
            verify(button.background.opacity === 0.1, "Background opacity is correct")
        }

        function test_radius() {
            compare(button.background.radius, 24, "Radius is correct")
        }

        function test_symbol() {
            compare(button.contentItem.text, "\ue5d4", "Symbol is correct")
            compare(button.contentItem.color, "#ffffff", "Symbol color is correct")
            compare(button.contentItem.font.pointSize, 16, "Symbol size is correct")
        }
    }
}
