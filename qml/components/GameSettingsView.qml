import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models
import Firelight 1.0

FocusScope {
    id: root

    required property var level
    required property var contentHash
    required property var platformId
    required property var platformName

    property var model: EmulationSettingsModel {
            contentHash: root.contentHash
            platformId: root.platformId
            level: root.level
        }

    // property var level: GameSettings.Game

    ListView {
        height: parent.height
        width: Math.min(parent.width, 800)
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 12
        focus: true

        highlightMoveDuration: 80
        highlightMoveVelocity: -1
        highlightRangeMode: InputMethodManager.usingMouse ? ListView.NoHighlightRange : ListView.ApplyRange
        preferredHighlightBegin: 64
        preferredHighlightEnd: height - 64
        ScrollBar.vertical: ScrollBar { }

        section.property: "section"
        section.delegate: SettingsSectionHeader {
            required property var section
            sectionName: section
            showTopPadding: section !== "Video"
        }

        header: ColumnLayout {
            spacing: 8
            width: ListView.view.width
            TabBar {
                Layout.alignment: Qt.AlignCenter
                currentIndex: root.level
                onCurrentIndexChanged: {
                    console.log("Changing level to", currentIndex)
                    if (root.level !== currentIndex) {
                        root.level = currentIndex
                    }
                }

                TabButton {
                    width: 120
                    text: "Game"
                }
                TabButton {
                    width: 120
                    text: "Platform"
                }
            }

            Text {
                Layout.alignment: Qt.AlignCenter
                font.pixelSize: 15
                font.family: Constants.regularFontFamily
                font.weight: Font.Medium
                color: ColorPalette.neutral100
                text: root.level === 0 ? "Settings are applied only to the current game" : "Settings are applied to all " + root.platformName + " games"
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                Layout.preferredWidth: parent.width * 0.75
                lineHeight: 1.2

            }

        }

        model: root.model

        delegate: DelegateChooser {
            id: chooser
            role: "type"
            DelegateChoice {
                roleValue: "toggle"
                delegate: toggleDelegate
            }
            DelegateChoice {
                roleValue: "combobox"
                delegate: comboBoxDelegate
            }
        }
    }

    Component {
        id: toggleDelegate
        ToggleSettingItem {
            required property var model
            required property var index
            width: ListView.view.width
            description: model.description

            property bool initialized: false

             checked: model.value

            onClicked: function() {
                ListView.view.currentIndex = index
                model.value = !checked
            }

            // Component.onCompleted: {
            //      checked = model.value
            //      initialized = true
            // }

            onCheckedChanged: {
                // if (!initialized) {
                //     return
                // }
            }
        }
    }

    Component {
        id: comboBoxDelegate
        ComboBoxSettingItem {
            id: comboSettingItem
            required property var model
            required property var index
            focus: true
            width: ListView.view.width

            property bool initialized: false

            onClicked: function() {
                 comboSettingItem.popup.open()
                 comboSettingItem.popup.forceActiveFocus()
            }

            currentIndex: {
                for (let i = 0; i < model.options.length; i++) {
                    if (model.options[i].value === model.value) {
                        return i
                    }
                }
                return -1
            }

            onCurrentValueChanged: {
                // if (!initialized) {
                //     return
                // }
                if (model.value !== currentValue) {
                    model.value = currentValue
                }
            }

            label: model.label
            description: model.description

            textRole: "label"
            valueRole: "value"

             comboBoxModel: model.options
        }
    }
}