import QtQuick 2.3
import QtTest 1.0
import QMLFirelightTest

Item {
    id: root
    width: 800
    height: 800

    LibraryPage2 {
        id: page
        anchors.fill: parent
    }

    TestCase {
        name: "LibraryPage"
        when: windowShown

        function test_width_data() {
            return [
                {rootWidth: 200, expectedNumCells: 1},
                {rootWidth: 300, expectedNumCells: 1},
                {rootWidth: 400, expectedNumCells: 2},
                {rootWidth: 500, expectedNumCells: 2},
                {rootWidth: 600, expectedNumCells: 3},
                {rootWidth: 700, expectedNumCells: 3},
                {rootWidth: 800, expectedNumCells: 4},
                {rootWidth: 900, expectedNumCells: 4},
                {rootWidth: 1000, expectedNumCells: 5},
                {rootWidth: 1100, expectedNumCells: 5},
                {rootWidth: 1200, expectedNumCells: 6},
                {rootWidth: 1300, expectedNumCells: 6},
                {rootWidth: 1400, expectedNumCells: 7},
                {rootWidth: 1500, expectedNumCells: 7},
                {rootWidth: 1600, expectedNumCells: 7}, // Make sure it caps at 7
                {rootWidth: 1700, expectedNumCells: 7},
                {rootWidth: 1800, expectedNumCells: 7},
                {rootWidth: 1900, expectedNumCells: 7}
            ]
        }

        function test_width(data) {
            root.width = data.rootWidth
            const grid = findChild(page, "GridView")
            compare(grid.width, grid.cellWidth * data.expectedNumCells, "Width is correct")
        }

        function test_default_values() {
            const grid = findChild(page, "GridView")
            compare(grid.preferredHighlightBegin, 0.33 * grid.height, "Preferred highlight begin is correct")
            compare(grid.preferredHighlightEnd, 0.66 * grid.height, "Preferred highlight end is correct")
            compare(grid.highlightRangeMode, GridView.ApplyRange, "Highlight range mode is correct")
            verify(grid.clip, "Clip is enabled")
            verify(grid.focus, "Focus is enabled")
            compare(grid.boundsBehavior, GridView.StopAtBounds, "Bounds behavior is correct")
            compare(grid.cellSpacing, 12, "Cell spacing is correct")
            compare(grid.cellWidth, 192, "Cell width is correct")
            compare(grid.cellHeight, 272, "Cell height is correct")
        }
    }
}
