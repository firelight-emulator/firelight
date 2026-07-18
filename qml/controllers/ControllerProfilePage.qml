import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

Pane {
    id: root

    // property alias controllerProfileId: gamepadProfile.profileId
    required property var playerNumber
    required property var profileId

    padding: 16
    background: Item {}
    contentItem: FocusScope {

        GamepadStatus {
            id: gamepadStatus
            playerNumber: root.playerNumber
        }

        GamepadProfile {
            id: profile
            profileId: root.profileId
        }

        ButtonGroup {
            id: menuButtonGroup
            exclusive: true
        }

        Pane {
            id: menuPane
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 300

            background: Item {}

            contentItem: ColumnLayout {
                FirelightMenuItem {
                    labelText: "Shortcuts"
                    property bool showGlobalCursor: true
                    // labelIcon: "\ue40a"
                    Layout.preferredHeight: 50
                    Layout.fillWidth: true
                    focus: true
                    checked: true

                    ButtonGroup.group: menuButtonGroup

                    onToggled: {
                        if (checked) {
                            theStack.replaceCurrentItem(shortcutsList, {}, StackView.Immediate);
                        }
                    }
                }
                FirelightMenuItem {
                    labelText: "Analog Sticks"
                    property bool showGlobalCursor: true
                    Layout.preferredHeight: 50
                    Layout.fillWidth: true
                    visible: !profile.isKeyboardProfile

                    ButtonGroup.group: menuButtonGroup

                    onToggled: {
                        if (checked) {
                            theStack.replaceCurrentItem(analogPage, {
                                profileId: root.profileId
                            }, StackView.Immediate);
                        }
                    }
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: Theme.border
                }
                ListView {
                    id: platformList
                    spacing: 0
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    focus: true
                    clip: true

                    KeyNavigation.right: theStack

                    Keys.onBackPressed: {
                        Router.back();
                    }

                    Keys.onEscapePressed: {
                        Router.back();
                    }

                    currentIndex: 0

                    // onCurrentItemChanged: {
                    //     gamepadProfile.currentPlatformId = platformList.currentItem.model.platform_id
                    // }

                    model: PlatformModel
                    delegate: FirelightMenuItem {
                        required property var model
                        required property var index
                        // focus: true

                        ButtonGroup.group: menuButtonGroup

                        labelText: model.displayName
                        labelIcon: model.iconName
                        property bool showGlobalCursor: true
                        // labelIcon: "\ue40a"
                        height: 54
                        width: ListView.view.width

                        // checked: ListView.isCurrentItem

                        onToggled: {
                            if (checked) {
                                ListView.view.currentIndex = index;
                                theStack.replaceCurrentItem(mappingView, {
                                    platformId: model.platformId,
                                    profileId: gamepadStatus.profileId,
                                    platformMetadataModel: model,
                                    gamepad: gamepadStatus,
                                    isKeyboard: profile.isKeyboardProfile
                                }, StackView.Immediate);
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            id: verticalBar
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            // anchors.leftMargin: 8
            width: 1
            anchors.left: menuPane.right
            color: Theme.border
        }

        StackView {
            id: theStack

            anchors.left: verticalBar.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.leftMargin: 16

            initialItem: shortcutsList
        }

        Component {
            id: mappingView

            ControllerInputMappingView {}
        }

        Component {
            id: analogPage

            AnalogTuningPage {}
        }

        Component {
            id: shortcutsList
            ListView {
                id: shortcutsListView
                ShortcutInputPromptDialog {
                    id: shortcutDialog
                    shortcutName: ""
                    shortcut: ""
                    gamepad: gamepadStatus
                    isKeyboard: profile.isKeyboardProfile
                    modifierCandidates: shortcutsListView.model.modifierCandidates()

                    onMappingAdded: function (shortcut, modifiers, input) {
                        shortcutsListView.model.addBinding(shortcut, modifiers, input);
                    }
                }
                header: Text {
                    text: "Shortcuts"
                    color: Theme.textPrimary
                    font.pixelSize: AppStyle.fontSizeLarge
                    font.family: Constants.regularFontFamily
                    font.weight: Font.DemiBold
                    verticalAlignment: Text.AlignVCenter
                    padding: 16
                }
                model: profile.shortcutsModel
                delegate: Button {
                    id: myDelegate
                    required property var model
                    required property var index
                    property bool showGlobalCursor: true
                    height: 60
                    width: ListView.view.width
                    hoverEnabled: true
                    background: Rectangle {
                        color: ColorPalette.neutral300
                        radius: 8
                        border.color: ColorPalette.neutral500
                        opacity: parent.hovered || (!InputMethodManager.usingMouse && parent.activeFocus) ? 0.14 : 0

                        Behavior on opacity {
                            NumberAnimation {
                                duration: 64
                                easing.type: Easing.InOutQuad
                            }
                        }
                    }
                    ContextMenu.menu: RightClickMenu {
                        RightClickMenuItem {
                            text: "Assign"

                            onTriggered: {
                                shortcutDialog.shortcut = model.shortcutId;
                                shortcutDialog.shortcutName = model.name;
                                shortcutDialog.open();
                                // dialog.buttons = []
                                // dialog.buttons = [{
                                //     display_name: model.originalInputName,
                                //     retropad_button: model.originalInput
                                // }]
                                // dialog.open()
                            }
                        }

                        RightClickMenuItem {
                            text: "Clear mapping"
                            onTriggered: {
                                shortcutsListView.model.clearBindings(model.shortcutId);
                            }
                        }
                    }
                    onClicked: function () {
                        ListView.view.currentIndex = index;
                    }
                    onDoubleClicked: function () {
                        shortcutDialog.shortcut = model.shortcutId;
                        shortcutDialog.shortcutName = model.name;
                        shortcutDialog.open();
                    }
                    contentItem: RowLayout {
                        spacing: 12
                        Item {
                            Layout.fillHeight: true
                            Layout.preferredWidth: 16
                        }
                        Text {
                            Layout.preferredWidth: 240
                            Layout.maximumWidth: 240
                            Layout.alignment: Qt.AlignLeft
                            Layout.fillHeight: true
                            text: model.name
                            color: Theme.textPrimary
                            font.pixelSize: AppStyle.fontSizeMedium
                            font.family: Constants.regularFontFamily
                            font.weight: Font.DemiBold
                            verticalAlignment: Text.AlignVCenter
                        }

                        Text {
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignLeft
                            Layout.fillHeight: true
                            font.pixelSize: AppStyle.fontSizeMedium
                            color: model.hasBinding ? "white" : ColorPalette.neutral400
                            text: model.bindingsLabel
                            font.family: Constants.regularFontFamily
                            font.weight: model.hasBinding ? Font.DemiBold : Font.Medium
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }
    }
}
