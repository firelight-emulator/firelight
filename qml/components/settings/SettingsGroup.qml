import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models
import Firelight 1.0

// TODO
// Renders every setting declared in one catalog group as a section card, title
// and all. Adding a setting to the group is a catalog edit — nothing here
// changes. When a control needs to be bespoke, declare the setting `custom` and
// name your own delegate (PaletteSettingItem is the reference)
//
// Every widget the parser can emit needs a DelegateChoice below, or it silently
// falls through to a dropdown. That invariant is the whole reason this is the
// only place settings are rendered
//
// `level`/`platformId`/`contentHash` only matter for emulation settings; app
// settings in the group ignore them and always use the global tier
SettingsSection {
    id: root

    required property string group

    property int level: SettingsLevel.Global
    property var platformId: -1
    property var contentHash: ""

    title: settingsModel.groupLabel

    // The card would otherwise sit at its implicit width inside a ColumnLayout
    Layout.fillWidth: true

    property alias model: settingsModel

    SettingsModel {
        id: settingsModel
        group: root.group
        level: root.level
        platformId: root.platformId
        contentHash: root.contentHash
        showAdvanced: GeneralSettings.showAdvancedSettings
    }

    Repeater {
        model: settingsModel

        delegate: DelegateChooser {
            role: "widget"
            DelegateChoice { roleValue: "toggle"; delegate: toggleDelegate }
            DelegateChoice { roleValue: "slider"; delegate: sliderDelegate }
            DelegateChoice { roleValue: "spinbox"; delegate: sliderDelegate }
            DelegateChoice { roleValue: "stepper"; delegate: stepperDelegate }
            DelegateChoice { roleValue: "segmented"; delegate: segmentedDelegate }
            DelegateChoice { roleValue: "radio"; delegate: radioDelegate }
            DelegateChoice { roleValue: "gbc-palette"; delegate: paletteDelegate }
            DelegateChoice { roleValue: "text"; delegate: textDelegate }
            DelegateChoice { roleValue: "color"; delegate: colorDelegate }
            DelegateChoice { roleValue: "file-picker"; delegate: filePathDelegate }
            DelegateChoice { roleValue: "folder-picker"; delegate: filePathDelegate }
            DelegateChoice { roleValue: "multi-select"; delegate: multiSelectDelegate }
            DelegateChoice { roleValue: "key-binding"; delegate: keyBindingDelegate }
            DelegateChoice { delegate: comboBoxDelegate }
        }

        onItemAdded: function (index, item) {
            if (index === 0) {
                item.isFirstInSection = true
            }
            if (index === settingsModel.rowCount() - 1) {
                item.isLastInSection = true
            }
        }
    }

    Component {
        id: keyBindingDelegate
        KeyBindingSettingItem {
            required property var model
            required property var index
            shown: model.visible
            subItem: model.subItem
            enabled: model.enabled
            label: model.label
            description: model.description
            resettable: model.resettable
            binding: model.value

            onReset: settingsModel.resetValue(index)
        }
    }

    Component {
        id: toggleDelegate
        ToggleSettingItem {
            required property var model
            required property var index
            shown: model.visible
            subItem: model.subItem
            enabled: model.enabled
            label: model.label
            description: model.description
            checked: model.value
            resettable: model.resettable

            onReset: settingsModel.resetValue(index)
            onClicked: function () { model.value = !checked }
        }
    }

    Component {
        id: sliderDelegate
        SliderSettingItem {
            required property var model
            required property var index
            shown: model.visible
            subItem: model.subItem
            enabled: model.enabled
            label: model.label
            description: model.description
            resettable: model.resettable
            from: model.minimumValue
            to: model.maximumValue
            stepSize: model.stepValue
            value: parseFloat(model.value)

            onReset: settingsModel.resetValue(index)
            onMoved: function (v) { model.value = v.toString() }
        }
    }

    Component {
        id: stepperDelegate
        StepperSettingItem {
            required property var model
            required property var index
            shown: model.visible
            subItem: model.subItem
            enabled: model.enabled
            label: model.label
            description: model.description
            resettable: model.resettable
            from: model.minimumValue
            to: model.maximumValue
            stepSize: model.stepValue
            value: parseFloat(model.value)

            onReset: settingsModel.resetValue(index)
            onChanged: function (v) { model.value = v.toString() }
        }
    }

    Component {
        id: segmentedDelegate
        SegmentedSettingItem {
            required property var model
            required property var index
            shown: model.visible
            subItem: model.subItem
            enabled: model.enabled
            label: model.label
            description: model.description
            resettable: model.resettable
            options: model.options
            currentValue: model.value

            onReset: settingsModel.resetValue(index)
            onChanged: function (v) { model.value = v }
        }
    }

    Component {
        id: radioDelegate
        RadioSettingItem {
            required property var model
            required property var index
            shown: model.visible
            subItem: model.subItem
            enabled: model.enabled
            label: model.label
            description: model.description
            resettable: model.resettable
            options: model.options
            currentValue: model.value

            onReset: settingsModel.resetValue(index)
            onChanged: function (v) { model.value = v }
        }
    }

    Component {
        id: paletteDelegate
        PaletteSettingItem {
            required property var model
            required property var index
            shown: model.visible
            subItem: model.subItem
            enabled: model.enabled
            label: model.label
            description: model.description
            options: model.options
            value: model.value
            resettable: model.resettable

            onReset: settingsModel.resetValue(index)
            onActivated: function (v) { model.value = v }
        }
    }

    Component {
        id: textDelegate
        TextSettingItem {
            required property var model
            required property var index
            shown: model.visible
            subItem: model.subItem
            enabled: model.enabled
            label: model.label
            description: model.description
            resettable: model.resettable
            value: model.value
            placeholder: model.placeholder

            onReset: settingsModel.resetValue(index)
            onEdited: function (v) { model.value = v }
        }
    }

    Component {
        id: colorDelegate
        ColorSettingItem {
            required property var model
            required property var index
            shown: model.visible
            subItem: model.subItem
            enabled: model.enabled
            label: model.label
            description: model.description
            resettable: model.resettable
            value: model.value
            presets: model.options

            onReset: settingsModel.resetValue(index)
            onPicked: function (hex) { model.value = hex }
        }
    }

    Component {
        id: filePathDelegate
        FilePathSettingItem {
            required property var model
            required property var index
            shown: model.visible
            subItem: model.subItem
            enabled: model.enabled
            label: model.label
            description: model.description
            resettable: model.resettable
            value: model.value
            directoryMode: model.directoryMode
            extensions: model.fileExtensions

            onReset: settingsModel.resetValue(index)
            onChosen: function (path) { model.value = path }
        }
    }

    Component {
        id: multiSelectDelegate
        MultiSelectSettingItem {
            required property var model
            required property var index
            shown: model.visible
            subItem: model.subItem
            enabled: model.enabled
            label: model.label
            description: model.description
            resettable: model.resettable
            value: model.value
            options: model.options

            onReset: settingsModel.resetValue(index)
            onChanged: function (jsonValue) { model.value = jsonValue }
        }
    }

    Component {
        id: comboBoxDelegate
        ComboBoxSettingItem {
            id: comboItem
            required property var model
            required property var index
            shown: model.visible
            subItem: model.subItem
            enabled: model.enabled
            label: model.label
            description: model.description
            resettable: model.resettable

            textRole: "label"
            valueRole: "value"
            comboBoxModel: model.options

            currentIndex: {
                for (let i = 0; i < model.options.length; i++) {
                    if (model.options[i].value === model.value) {
                        return i;
                    }
                }
                return -1;
            }

            onReset: settingsModel.resetValue(index)
            onClicked: function () {
                comboItem.popup.open();
                comboItem.popup.forceActiveFocus();
            }
            onCurrentValueChanged: {
                if (model.value !== currentValue) {
                    model.value = currentValue;
                }
            }
        }
    }
}
