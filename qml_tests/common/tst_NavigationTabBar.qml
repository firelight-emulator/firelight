import QtQuick 2.3
import QtTest 1.0
import QMLFirelightTest

Item {
    width: 400
    height: 400

    NavigationTabBar {
        id: control
        width: parent.width
        height: 100

        tabs: ["Hi", "There"]
        tabWidth: 100
    }

    TestCase {
        name: "NavigationTabBar"
        when: windowShown

        function init() {
            control.currentIndex = 0
        }

        function test_clicking() {
            verify(control.contentChildren.length === 2)

            const buttonOne = control.contentChildren[0]
            const buttonTwo = control.contentChildren[1]

            compare(control.currentIndex, 0, "currentIndex is correct")
            mouseClick(buttonTwo)
            compare(control.currentIndex, 1, "currentIndex is correct")
            mouseClick(buttonOne)
            compare(control.currentIndex, 0, "currentIndex is correct")

            // verify(control.contentChildren[0].text === "Hi")
        }

        function test_highlight() {
            verify(control.contentChildren.length === 2)
            const highlight = findChild(control, "background")

            const buttonOne = control.contentChildren[0]
            const buttonTwo = control.contentChildren[1]

            compare(control.currentIndex, 0, "currentIndex is correct")
            compare(highlight.width, 100, "highlight width is correct")
            compare(highlight.height, 4, "highlight height is correct")
            compare(highlight.radius, 2, "highlight radius is correct")
            compare(highlight.color, "#ffffff", "highlight color is correct")
            compare(highlight.y, 100, "highlight y is correct")
            compare(highlight.x, 0, "highlight x is correct")

            mouseClick(buttonTwo)
            wait(200)
            compare(control.currentIndex, 1, "currentIndex is correct")
            compare(highlight.x, 100, "highlight x is correct")
        }

        function test_tabWidth() {
            verify(control.contentChildren.length === 2)

            const buttonOne = control.contentChildren[0]
            const buttonTwo = control.contentChildren[1]

            compare(buttonOne.width, 100, "buttonOne width is correct")
            compare(buttonTwo.width, 100, "buttonTwo width is correct")
        }

        function test_tab_labels() {
            verify(control.contentChildren.length === 2)

            const buttonOne = control.contentChildren[0]
            const buttonTwo = control.contentChildren[1]

            compare(control.currentIndex, 0, "currentIndex is correct")
            const textOne = findChild(buttonOne, "text")
            const textTwo = findChild(buttonTwo, "text")

            compare(textOne.text, "Hi", "buttonOne text is correct")
            compare(textOne.color, "#ffffff", "text color is correct")
            compare(textOne.font.pixelSize, 15, "text pointSize is correct")
            compare(textOne.font.weight, Font.Bold, "text weight is correct")
            verify(buttonOne.checked, "buttonOne is checked")

            compare(textTwo.text, "There", "buttonTwo text is correct")
            compare(textTwo.color, "#ffffff", "text color is correct")
            compare(textTwo.font.pixelSize, 15, "text pointSize is correct")
            compare(textTwo.font.weight, Font.Normal, "text weight is correct")
            verify(!buttonTwo.checked, "buttonTwo is not checked")

            mouseClick(buttonTwo)

            compare(textTwo.text, "There", "buttonTwo text is correct")
            compare(textTwo.color, "#ffffff", "text color is correct")
            compare(textTwo.font.pixelSize, 15, "text pointSize is correct")
            compare(textTwo.font.weight, Font.Bold, "text weight is correct")
            verify(buttonTwo.checked, "buttonTwo is checked")

            compare(textOne.text, "Hi", "buttonOne text is correct")
            compare(textOne.color, "#ffffff", "text color is correct")
            compare(textOne.font.pixelSize, 15, "text pointSize is correct")
            compare(textOne.font.weight, Font.Normal, "text weight is correct")
            verify(!buttonOne.checked, "buttonOne is not checked")


        }

    }
}