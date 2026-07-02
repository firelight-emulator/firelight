import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models
import Firelight 1.0

// Reusable list of friendly emulation settings for a single tier. Callers set
// `level` (0 Game / 1 Platform / 2 Global) plus the scope (`platformId`,
// `contentHash`). Each row shows the effective value; overridden rows get a
// Reset affordance that clears this tier's override.
FocusScope {
    id: root

    property int level: 0
    property var platformId: -1
    property var contentHash: ""
    property bool advancedOpen: false

    property alias model: settingsModel

    EmulationSettingsModel {
        id: settingsModel
        level: root.level
        platformId: root.platformId
        contentHash: root.contentHash
    }

    // Raw libretro options for the "Advanced" section (populated once a game on
    // this console has run this session).
    CoreOptionsModel {
        id: coreOptionsModel
        level: root.level
        platformId: root.platformId
        contentHash: root.contentHash
    }

    ListView {
        id: listView
        height: parent.height
        width: Math.min(parent.width, 800)
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 12
        focus: true
        clip: true

        highlightMoveDuration: 80
        highlightRangeMode: InputMethodManager.usingMouse ? ListView.NoHighlightRange : ListView.ApplyRange
        preferredHighlightBegin: 64
        preferredHighlightEnd: height - 64
        ScrollBar.vertical: ScrollBar {}

        section.property: "section"
        section.delegate: SettingsSectionHeader {
            required property var section
            width: ListView.view.width
            sectionName: section
        }

        model: settingsModel

        // Advanced (raw core options) — only for a specific console, collapsed
        // by default.
        footer: ColumnLayout {
            width: ListView.view.width
            visible: root.platformId !== -1
            spacing: 8

            Rectangle {
                Layout.fillWidth: true
                Layout.topMargin: 16
                Layout.preferredHeight: 1
                color: "#333333"
            }

            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                hoverEnabled: true
                focusPolicy: Qt.NoFocus

                background: Rectangle {
                    color: ColorPalette.neutral300
                    opacity: parent.hovered ? 0.2 : 0.08
                    radius: 8
                }
                contentItem: Text {
                    text: root.advancedOpen ? qsTr("Hide advanced core options") : qsTr("Show advanced core options")
                    color: ColorPalette.neutral100
                    leftPadding: 8
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 16
                    font.family: Constants.regularFontFamily
                    font.weight: Font.DemiBold
                }
                onClicked: root.advancedOpen = !root.advancedOpen
            }

            Text {
                Layout.fillWidth: true
                leftPadding: 8
                visible: root.advancedOpen && coreOptionsModel.categories.length === 0
                text: qsTr("These are the core's raw options. They appear after a game on this console has run at least once.")
                color: ColorPalette.neutral300
                wrapMode: Text.WordWrap
                font.pixelSize: 14
                font.family: Constants.regularFontFamily
                font.weight: Font.Medium
            }

            // Inside a category: a back link to the category list.
            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                visible: root.advancedOpen && coreOptionsModel.categoryFilter !== ""
                hoverEnabled: true
                focusPolicy: Qt.NoFocus
                background: Rectangle {
                    color: ColorPalette.neutral300
                    opacity: parent.hovered ? 0.2 : 0.08
                    radius: 8
                }
                contentItem: Text {
                    text: "‹  " + qsTr("All categories")
                    leftPadding: 8
                    color: ColorPalette.neutral100
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 15
                    font.family: Constants.regularFontFamily
                    font.weight: Font.DemiBold
                }
                onClicked: coreOptionsModel.categoryFilter = ""
            }

            // Top level: one entry per category; click to drill in.
            Repeater {
                model: (root.advancedOpen && coreOptionsModel.categoryFilter === "") ? coreOptionsModel.categories : null

                Button {
                    required property var modelData
                    Layout.fillWidth: true
                    Layout.preferredHeight: 56
                    hoverEnabled: true
                    focusPolicy: Qt.NoFocus
                    background: Rectangle {
                        color: ColorPalette.neutral300
                        opacity: parent.hovered ? 0.2 : 0.08
                        radius: 8
                    }
                    contentItem: RowLayout {
                        Text {
                            Layout.fillWidth: true
                            leftPadding: 8
                            text: modelData.label
                            color: ColorPalette.neutral100
                            verticalAlignment: Text.AlignVCenter
                            font.pixelSize: 16
                            font.family: Constants.regularFontFamily
                            font.weight: Font.DemiBold
                        }
                        Text {
                            rightPadding: 8
                            text: "›"
                            color: ColorPalette.neutral300
                            font.pixelSize: 20
                        }
                    }
                    onClicked: coreOptionsModel.categoryFilter = modelData.key
                }
            }

            Repeater {
                model: root.advancedOpen ? coreOptionsModel : null

                ComboBoxSettingItem {
                    id: advancedItem
                    required property var model
                    required property var index
                    Layout.fillWidth: true
                    label: model.label
                    description: model.description
                    resettable: model.overridden

                    textRole: "label"
                    valueRole: "value"
                    comboBoxModel: model.options

                    currentIndex: {
                        for (let i = 0; i < model.options.length; i++) {
                            if (model.options[i].value === model.value) {
                                return i
                            }
                        }
                        return -1
                    }

                    onReset: coreOptionsModel.resetValue(index)
                    onClicked: function () {
                        advancedItem.popup.open()
                        advancedItem.popup.forceActiveFocus()
                    }
                    onCurrentValueChanged: {
                        if (model.value !== currentValue) {
                            model.value = currentValue
                        }
                    }
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 16
            }
        }

        delegate: DelegateChooser {
            role: "widget"
            DelegateChoice {
                roleValue: "toggle"
                delegate: toggleDelegate
            }
            DelegateChoice {
                roleValue: "slider"
                delegate: sliderDelegate
            }
            DelegateChoice {
                roleValue: "spinbox"
                delegate: sliderDelegate
            }
            DelegateChoice {
                roleValue: "gbc-palette"
                delegate: paletteDelegate
            }
            DelegateChoice {
                delegate: comboBoxDelegate
            }
        }
    }

    Component {
        id: paletteDelegate
        PaletteSettingItem {
            required property var model
            required property var index
            width: ListView.view.width
            visible: model.visible
            height: visible ? implicitHeight : 0
            enabled: model.enabled
            label: model.label
            description: model.description
            options: model.options
            value: model.value
            resettable: model.overridden

            onReset: root.model.resetValue(index)
            onActivated: function (v) {
                ListView.view.currentIndex = index
                model.value = v
            }
        }
    }

    Component {
        id: toggleDelegate
        ToggleSettingItem {
            required property var model
            required property var index
            width: ListView.view.width
            visible: model.visible
            height: visible ? implicitHeight : 0
            enabled: model.enabled
            label: model.label
            description: model.description
            checked: model.value
            resettable: model.overridden

            onReset: root.model.resetValue(index)
            onClicked: function () {
                ListView.view.currentIndex = index
                model.value = !checked
            }
        }
    }

    Component {
        id: sliderDelegate
        SliderSettingItem {
            required property var model
            required property var index
            width: ListView.view.width
            visible: model.visible
            height: visible ? implicitHeight : 0
            enabled: model.enabled
            label: model.label
            description: model.description
            resettable: model.overridden

            from: model.minimumValue
            to: model.maximumValue
            stepSize: model.stepValue
            value: Number(model.value)

            onReset: root.model.resetValue(index)
            onMoved: function (v) {
                ListView.view.currentIndex = index
                model.value = v
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
            visible: model.visible
            height: visible ? implicitHeight : 0
            enabled: model.enabled
            label: model.label
            description: model.description
            resettable: model.overridden

            textRole: "label"
            valueRole: "value"
            comboBoxModel: model.options

            currentIndex: {
                for (let i = 0; i < model.options.length; i++) {
                    if (model.options[i].value === model.value) {
                        return i
                    }
                }
                return -1
            }

            onReset: root.model.resetValue(index)
            onClicked: function () {
                comboSettingItem.popup.open()
                comboSettingItem.popup.forceActiveFocus()
            }
            onCurrentValueChanged: {
                if (model.value !== currentValue) {
                    model.value = currentValue
                }
            }
        }
    }
}
