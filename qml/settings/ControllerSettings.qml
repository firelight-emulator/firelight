import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// The one home for keybinds. Every profile's shortcuts are edited here, picked
// from the dropdown at the top — the Controllers page's "Edit profile" button
// deep-links in with a profileId rather than mounting a second editor
FocusScope {
    id: root

    // Which profile is being edited. Set from the URL on arrival, then by the
    // picker
    property int profileId: -1

    ProfileListModel {
        id: profiles
    }

    GamepadProfile {
        id: profile
        profileId: root.profileId
    }

    GamepadStatus {
        id: gamepadStatus
        playerNumber: 1
    }

    // "Edit profile" deep-links here as /settings/controllers?profileId=N. The
    // route only carries the section, so the id rides in the query — resolve()
    // drops params for a subtree-owning route, but leaves the query alone
    // Without one, edit whatever is in player one: that's what someone opening
    // this page cold almost always means
    Component.onCompleted: {
        const requested = parseInt(Router.query.profileId);
        if (!isNaN(requested)) {
            root.profileId = requested;
        } else if (gamepadStatus.profileId > 0) {
            root.profileId = gamepadStatus.profileId;
        } else if (profiles.rowCount() > 0) {
            root.profileId = profiles.data(profiles.index(0, 0), Qt.UserRole + 1); // profileId
        }
    }

    ShortcutInputPromptDialog {
        id: shortcutDialog
        shortcutName: ""
        shortcut: ""
        gamepad: gamepadStatus
        isKeyboard: profile.isKeyboardProfile
        modifierCandidates: profile.shortcutsModel ? profile.shortcutsModel.modifierCandidates() : []

        onMappingAdded: function (shortcut, modifiers, input) {
            profile.shortcutsModel.addBinding(shortcut, modifiers, input);
        }
    }

    FirelightDialog {
        id: confirmPresetDialog

        property string presetId: ""
        property string presetLabel: ""

        text: "Replace every shortcut in this profile with the " + presetLabel + " defaults? Anything you've changed will be overwritten."

        onAccepted: profile.shortcutsModel.applyPreset(presetId)
    }

    PlatformInputPreferences {
        id: platformPrefs
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 12

        ToggleSettingItem {
            Layout.fillWidth: true
            label: "Prioritize controllers over keyboard"
            description: "Connecting a controller pushes the keyboard down a player slot, so the controller becomes player one."
            checked: InputService.prioritizeControllerOverKeyboard
            onCheckedChanged: InputService.prioritizeControllerOverKeyboard = checked
        }

        ToggleSettingItem {
            Layout.fillWidth: true
            label: "Only allow player one to navigate menus"
            description: "The keyboard can always navigate menus, whatever this is set to."
            checked: InputService.onlyPlayerOneCanNavigateMenus
            onCheckedChanged: InputService.onlyPlayerOneCanNavigateMenus = checked
        }

        SettingBinding {
            id: playerOneHotkeysBinding
            key: "only-player-one-hotkeys"
        }

        ToggleSettingItem {
            Layout.fillWidth: true
            label: playerOneHotkeysBinding.label
            description: playerOneHotkeysBinding.description
            checked: playerOneHotkeysBinding.value === "true"
            onCheckedChanged: playerOneHotkeysBinding.value = checked ? "true" : "false"
        }

        Text {
            text: "Preferred controller per platform"
            color: Theme.textPrimary
            font.pixelSize: AppStyle.fontSizeMedium
            font.family: Constants.regularFontFamily
            font.weight: Font.DemiBold
            Layout.fillWidth: true
            Layout.topMargin: 8
        }

        // Capped: one row per platform would otherwise push the keybinds off
        // the page, and this page's viewport doesn't scroll as a whole
        ScrollView {
            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(platformColumn.implicitHeight, 130)
            clip: true

            ColumnLayout {
                id: platformColumn
                width: parent.width
                spacing: 4

                Repeater {
                    model: PlatformModel
                    delegate: RowLayout {
                        required property int platformId
                        required property string displayName
                        Layout.fillWidth: true
                        spacing: 12

                        Text {
                            text: displayName
                            color: Theme.textPrimary
                            font.pixelSize: AppStyle.fontSizeSmall
                            font.family: Constants.regularFontFamily
                            Layout.fillWidth: true
                            verticalAlignment: Text.AlignVCenter
                        }
                        FLComboBox {
                            Layout.preferredWidth: 220
                            textRole: "label"
                            valueRole: "value"
                            model: platformPrefs.controllerTypeOptions()
                            Component.onCompleted: currentIndex = indexOfValue(platformPrefs.preferredType(platformId))
                            onActivated: platformPrefs.setPreferredType(platformId, currentValue)
                        }
                    }
                }
            }
        }

        Text {
            text: "Hotkeys"
            color: Theme.textPrimary
            font.pixelSize: AppStyle.fontSizeMedium
            font.family: Constants.regularFontFamily
            font.weight: Font.DemiBold
            Layout.fillWidth: true
            Layout.topMargin: 8
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
                text: "Profile"
                color: Theme.textPrimary
                font.pixelSize: AppStyle.fontSizeMedium
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
            }

            FLComboBox {
                id: profilePicker
                Layout.fillWidth: true
                model: profiles
                textRole: "name"
                valueRole: "profileId"

                // Follows root.profileId rather than driving it, so a deep link
                // and a pick end up in the same place
                currentIndex: indexOfValue(root.profileId)
                onActivated: root.profileId = currentValue
            }

            Text {
                text: "Preset"
                color: Theme.textPrimary
                font.pixelSize: AppStyle.fontSizeMedium
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
            }

            FLComboBox {
                id: presetPicker
                Layout.preferredWidth: 200
                textRole: "label"
                valueRole: "id"
                // Only the presets that fit this profile's device
                model: profile.shortcutsModel ? profile.shortcutsModel.presetOptions() : []

                Component.onCompleted: currentIndex = indexOfValue(profile.shortcutsModel ? profile.shortcutsModel.presetId : "")

                onActivated: {
                    confirmPresetDialog.presetId = currentValue;
                    confirmPresetDialog.presetLabel = currentText;
                    confirmPresetDialog.open();
                }
            }
        }

        Text {
            Layout.fillWidth: true
            text: profile.isKeyboardProfile ? "Press a key with Shift, Control or Alt to make a combo." : "Hold a button like Select and press another to make a combo. A combo only reaches the hotkey — a bare button still reaches the game."
            color: Theme.textMuted
            font.pixelSize: AppStyle.fontSizeSmall
            font.family: Constants.regularFontFamily
            wrapMode: Text.WordWrap
        }

        ListView {
            id: shortcutList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            focus: true
            model: profile.shortcutsModel
            spacing: 2

            ScrollBar.vertical: ScrollBar {}

            section.property: "category"
            section.criteria: ViewSection.FullString
            section.delegate: Text {
                required property string section
                width: ListView.view.width
                text: section
                color: Theme.textMuted
                font.pixelSize: AppStyle.fontSizeSmall
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
                topPadding: 12
                bottomPadding: 4
            }

            delegate: Button {
                id: shortcutRow
                required property var model
                required property int index
                property bool showGlobalCursor: true

                width: ListView.view.width
                height: 52
                hoverEnabled: true

                background: Rectangle {
                    color: ColorPalette.neutral300
                    radius: 8
                    opacity: shortcutRow.hovered || (!InputMethodManager.usingMouse && shortcutRow.activeFocus) ? 0.14 : 0
                }

                onClicked: {
                    ListView.view.currentIndex = index;
                }
                onDoubleClicked: assign()

                function assign() {
                    shortcutDialog.shortcut = model.shortcutId;
                    shortcutDialog.shortcutName = model.name;
                    shortcutDialog.open();
                }

                ContextMenu.menu: RightClickMenu {
                    RightClickMenuItem {
                        text: "Assign"
                        onTriggered: shortcutRow.assign()
                    }
                    RightClickMenuItem {
                        text: "Reset to default"
                        onTriggered: profile.shortcutsModel.resetToDefault(shortcutRow.model.shortcutId)
                    }
                    RightClickMenuItem {
                        // Unbinding is what turning a hotkey off means
                        text: "Clear (turns it off)"
                        onTriggered: profile.shortcutsModel.clearBindings(shortcutRow.model.shortcutId)
                    }
                }

                contentItem: RowLayout {
                    spacing: 12

                    Text {
                        Layout.preferredWidth: 200
                        Layout.fillHeight: true
                        text: shortcutRow.model.name
                        color: Theme.textPrimary
                        font.pixelSize: AppStyle.fontSizeMedium
                        font.family: Constants.regularFontFamily
                        font.weight: Font.DemiBold
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }

                    Text {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: shortcutRow.model.bindingsLabel
                        color: shortcutRow.model.hasBinding ? Theme.textPrimary : Theme.textMuted
                        font.pixelSize: AppStyle.fontSizeMedium
                        font.family: Constants.regularFontFamily
                        font.weight: shortcutRow.model.hasBinding ? Font.DemiBold : Font.Medium
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }

                    // Says this row no longer matches the preset, and is the
                    // only thing Reset acts on
                    Text {
                        visible: shortcutRow.model.isModified
                        text: "Changed"
                        color: Theme.textMuted
                        font.pixelSize: AppStyle.fontSizeSmall
                        font.family: Constants.regularFontFamily
                        verticalAlignment: Text.AlignVCenter
                    }

                    Button {
                        visible: shortcutRow.model.isModified
                        implicitWidth: 32
                        implicitHeight: 32
                        hoverEnabled: true
                        ToolTip.visible: hovered
                        ToolTip.text: "Reset to the preset's default"

                        background: Rectangle {
                            radius: 6
                            color: ColorPalette.neutral300
                            opacity: parent.hovered ? 0.2 : 0
                        }

                        contentItem: Icon {
                            name: "refresh"
                            size: 18
                            color: Theme.textMuted
                        }

                        onClicked: profile.shortcutsModel.resetToDefault(shortcutRow.model.shortcutId)
                    }
                }
            }
        }
    }
}
