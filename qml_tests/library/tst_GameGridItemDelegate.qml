import QtQuick 2.3
import QtTest 1.0
import QMLFirelightTest

Item {
    id: root
    width: 400
    height: 400

    SignalSpy {
        id: spy
        target: button
        signalName: "openDetails"
    }

    SignalSpy {
        id: startGameSpy
        target: button
        signalName: "startGame"
    }

    SignalSpy {
        id: manageSaveDataSpy
        target: button
        signalName: "manageSaveData"
    }

    GameGridItemDelegate {
        id: button
        cellWidth: 100
        cellHeight: 100
        cellSpacing: 8
        anchors.centerIn: parent

        index: 0
        model: {
            return {
                "id": 5,
                "display_name": "Test",
                "platform_name": "Test description"
            }
        }
    }

    TestCase {
        name: "GameGridItemDelegate"
        when: windowShown

        function init() {
            spy.clear()
            startGameSpy.clear()
        }

        function test_size_is_correct() {
            compare(button.width, 100, "Width is correct")
            compare(button.height, 100, "Height is correct")
        }

        function test_background_color_changes_on_hover() {
            const background = findChild(button, "background")

            compare(background.color, "#25282c", "Background is correct when not hovering")
            mouseMove(button)
            compare(background.color, "#3a3e45", "Background is correct when hovering")
        }

        function test_left_click_emits_detailsOpened() {
            skip("Skipping while view details is disabled")
            mouseClick(button)
            compare(spy.count, 1, "openDetails signal was emitted")
            compare(spy.signalArguments[0][0], 5, "openDetails signal was emitted with correct id")
        }

        function test_right_click_opens_menu() {
            skip("Skipping while view details is disabled")
            const rightClick = findChild(button, "rightClickMenu")

            compare(rightClick.visible, false, "Right click is not visible")

            mouseClick(button, button.width / 2, button.height / 2, Qt.RightButton)
            compare(rightClick.visible, true, "Right click is visible")

            mouseClick(button, 10, 10, Qt.RightButton)
            compare(rightClick.visible, true, "Right click is still visible")
        }

        function test_right_click_menu_play_game() {
            const rightClick = findChild(button, "rightClickMenu")
            mouseClick(button, button.width / 2, button.height / 2, Qt.RightButton)

            const play = rightClick.itemAt(0)
            compare(play.enabled, true, "Play button is enabled")
            mouseClick(play)

            compare(startGameSpy.count, 1, "startGame signal was emitted")
            compare(startGameSpy.signalArguments[0][0], 5, "startGame signal was emitted with correct id")
        }

        function test_right_click_menu_view_details() {
            skip("Skipping while view details is disabled")
            const rightClick = findChild(button, "rightClickMenu")
            mouseClick(button, button.width / 2, button.height / 2, Qt.RightButton)

            const viewDetails = rightClick.itemAt(1)
            compare(viewDetails.enabled, true, "View details is enabled")
            mouseClick(viewDetails)

            compare(spy.count, 1, "openDetails signal was emitted")
            compare(spy.signalArguments[0][0], 5, "openDetails signal was emitted with correct id")
        }

        function test_right_click_menu_manage_save_data() {
            const rightClick = findChild(button, "rightClickMenu")
            mouseClick(button, button.width / 2, button.height / 2, Qt.RightButton)

            const manageSaveDataSpy = rightClick.itemAt(3)
            compare(manageSaveDataSpy.enabled, true, "View details is enabled")
            mouseClick(manageSaveDataSpy)

            compare(manageSaveDataSpy.count, 1, "manageSaveDataSpy signal was emitted")
            compare(manageSaveDataSpy.signalArguments[0][0], 5, "manageSaveDataSpy signal was emitted with correct id")
        }

        function test_esc_closes_right_click_menu() {
            const rightClick = findChild(button, "rightClickMenu")

            mouseClick(button, button.width / 2, button.height / 2, Qt.RightButton)
            compare(rightClick.visible, true, "Right click is visible")

            keyClick(Qt.Key_Escape)
            compare(rightClick.visible, false, "Right click is not visible")
        }

        function test_click_outside_closes_right_click_menu() {
            const rightClick = findChild(button, "rightClickMenu")

            mouseClick(button, button.width / 2, button.height / 2, Qt.RightButton)
            compare(rightClick.visible, true, "Right click is visible")

            mouseClick(button, button.width, button.height, Qt.LeftButton)
            compare(rightClick.visible, false, "Right click is not visible")
        }

        function test_right_click_menu_has_correct_items() {
            const rightClick = findChild(button, "rightClickMenu")

            compare(rightClick.count, 4, "Right click menu has 3 items")
            compare(rightClick.itemAt(0).text, "Play Test", "First item is 'Play'")
            compare(rightClick.itemAt(1).text, "View details", "Second item is 'View details'")
            compare(rightClick.itemAt(2).height - (rightClick.itemAt(2).verticalPadding * 2), 1, "Third item is menu separator") // It's stupid but it works.
            compare(rightClick.itemAt(3).text, "Manage save data", "Second item is 'Manage save data'")
        }
    }
}
