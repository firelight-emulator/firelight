import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FocusScope {
    id: root

    required property GamepadStatus gamepad

    required property var platformId
    required property var profileId
    required property bool isKeyboard
    property var controllerType: 1

    required property var platformMetadataModel
    property var inputMappingsModel: InputMappingsModel {
        profileId: root.profileId
        platformId: root.platformId
        controllerTypeId: root.controllerType
    }

    // Image for the currently-selected controller type. controllerImages is
    // aligned 1:1 with controllerTypeIds, so look up by the type's position
    // rather than assuming contiguous 1-based ids (e.g. NES = 1, 3)
    readonly property string controllerImageUrl: {
        const ids = platformMetadataModel ? platformMetadataModel.controllerTypeIds : undefined;
        const idx = ids ? ids.indexOf(root.controllerType) : -1;
        return (idx >= 0 && platformMetadataModel) ? platformMetadataModel.controllerImages[idx] : "";
    }

    FirelightDialog {
        id: confirmDialog
        text: {
            if (!root.isKeyboard) {
                return "You're about to walk through assigning an input on your controller to each " + platformMetadataModel.displayName + " input.\n\n Continue?";
            } else {
                return "You're about to walk through assigning a key on your keyboard to each " + platformMetadataModel.displayName + " input.\n\n Continue?";
            }
        }

        showButtons: true

        onAccepted: function () {
            dialog.buttons = [];
            for (var i = 0; i < buttonList.count; ++i) {
                dialog.buttons.push({
                    display_name: buttonList.model.index(i, 0).data(258),
                    retropad_button: buttonList.model.index(i, 0).data(257)
                });
            }
            // dialog.buttons = platformMetadataModel.buttons
            dialog.currentIndex = 0;
            dialog.open();
        }
    }

    FirelightDialog {
        id: resetAllDialog

        text: "Reset all mappings to default?"
        showButtons: true

        onAccepted: function () {
            inputMappingsModel.resetAllToDefault();
        }
    }

    InputPromptDialog {
        id: dialog
        // imageSourceUrl: platformList.currentItem.model.icon_url
        imageSourceUrl: root.controllerImageUrl
        platformName: platformMetadataModel.displayName
        gamepad: root.gamepad
        isKeyboard: root.isKeyboard

        onMappingAdded: function (original, mapped) {
            inputMappingsModel.setMapping(original, mapped);
        }
    }

    // Per-button editor for turbo/autofire, toggle, and alternate bindings
    Popup {
        id: bindingOptionsPopup

        property int targetInput: -1
        property string targetLabel: ""

        parent: Overlay.overlay
        anchors.centerIn: Overlay.overlay
        width: 640
        height: 520
        modal: true
        padding: 0

        background: Rectangle {
            color: Theme.surface
            radius: 8
            border.color: Theme.border
        }

        contentItem: BindingOptionsPage {
            profileId: root.profileId
            platformId: root.platformId
            controllerTypeId: root.controllerType
            targetInput: bindingOptionsPopup.targetInput
            targetLabel: bindingOptionsPopup.targetLabel
            capturePlayerNumber: root.gamepad.playerNumber
        }
    }

    ListView {
        id: buttonList
        anchors.fill: parent
        anchors.rightMargin: 8
        // clip: true
        focus: true

        // highlightFollowsCurrentItem: true
        keyNavigationEnabled: true
        highlightMoveDuration: 80
        highlightMoveVelocity: -1
        highlightRangeMode: InputMethodManager.usingMouse ? ListView.NoHighlightRange : ListView.ApplyRange
        preferredHighlightBegin: 64
        preferredHighlightEnd: height - 64

        ScrollBar.vertical: FLScrollBar {}

        spacing: 4

        header: ColumnLayout {
            id: theHeader
            width: ListView.view.width
            spacing: 16
            // height: 200

            // Device-class selector (e.g. Gamepad / Mouse / Light Gun): switches
            // which controller type's bindings this view edits. Hidden when the
            // platform only has one type. Uses the actual controller-type ids
            // (they aren't contiguous — NES is 1 and 3, no 2)
            Flow {
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                spacing: 8
                visible: root.platformMetadataModel.numControllerTypes > 1
                Repeater {
                    model: root.platformMetadataModel.controllerTypeNames
                    delegate: Rectangle {
                        id: classTab
                        required property int index
                        required property var modelData
                        property int typeId: root.platformMetadataModel.controllerTypeIds[index]
                        property bool current: root.controllerType === classTab.typeId
                        implicitWidth: classTabText.implicitWidth + 28
                        implicitHeight: 34
                        radius: 6
                        color: classTab.current ? ColorPalette.neutral100 : "transparent"
                        border.color: classTab.current ? ColorPalette.neutral100 : ColorPalette.neutral500
                        border.width: 1
                        Text {
                            id: classTabText
                            anchors.centerIn: parent
                            text: classTab.modelData
                            color: classTab.current ? "black" : "white"
                            font.pixelSize: AppStyle.fontSizeSmall
                            font.family: AppStyle.fontFamily
                            font.weight: Font.Medium
                        }
                        HoverHandler {
                            cursorShape: Qt.PointingHandCursor
                        }
                        TapHandler {
                            onTapped: root.controllerType = classTab.typeId
                        }
                    }
                }
            }

            Image {
                id: imagey
                Layout.maximumHeight: 360
                Layout.maximumWidth: 360
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                // source: root.platformMetadataModel.icon_url
                source: root.controllerImageUrl
                sourceSize.height: 360
                mipmap: true
                fillMode: Image.PreserveAspectFit
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 16
                focus: true
                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }

                FirelightButton {
                    id: assignAllButton
                    label: "Assign all"
                    Layout.alignment: Qt.AlignLeft | Qt.AlignBottom
                    focus: true

                    onClicked: function () {
                        confirmDialog.open();
                    }
                }

                FirelightButton {
                    id: clearButton
                    label: "Reset all to default"
                    Layout.alignment: Qt.AlignLeft | Qt.AlignBottom

                    onClicked: function () {
                        resetAllDialog.open();
                    }
                }
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 16
            }
        }

        model: inputMappingsModel
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
                        dialog.buttons = [];
                        dialog.buttons = [
                            {
                                display_name: model.originalInputName,
                                retropad_button: model.originalInput
                            }
                        ];
                        dialog.open();
                    }
                }

                RightClickMenuItem {
                    text: "Reset to default"

                    onTriggered: {
                        inputMappingsModel.resetToDefault(model.originalInput);
                    }
                }

                RightClickMenuItem {
                    text: "Clear mapping"
                    onTriggered: {
                        inputMappingsModel.clearMapping(model.originalInput);
                    }
                }

                RightClickMenuItem {
                    text: "Turbo & alternate bindings…"
                    visible: !root.isKeyboard
                    onTriggered: {
                        bindingOptionsPopup.targetInput = model.originalInput;
                        bindingOptionsPopup.targetLabel = model.originalInputName;
                        bindingOptionsPopup.open();
                    }
                }
            }
            onClicked: function () {
                ListView.view.currentIndex = index;
            }
            onDoubleClicked: function () {
                dialog.buttons = [];
                dialog.buttons = [
                    {
                        display_name: model.originalInputName,
                        retropad_button: model.originalInput
                    }
                ];
                dialog.open();
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
                    text: model.originalInputName
                    color: Theme.textPrimary
                    font.pixelSize: AppStyle.fontSizeMedium
                    font.family: AppStyle.fontFamily
                    font.weight: Font.DemiBold
                    verticalAlignment: Text.AlignVCenter
                }

                Row {
                    Layout.preferredWidth: 240
                    Layout.maximumWidth: 240
                    Layout.alignment: Qt.AlignLeft
                    Layout.fillHeight: true

                    Text {
                        height: parent.height
                        font.pixelSize: AppStyle.fontSizeMedium
                        color: model.hasConflict ? Theme.warning : (!model.hasMapping ? Theme.textMuted : Theme.textPrimary)
                        text: model.hasMapping ? model.mappedInputName : "(Not mapped)"

                        font.family: AppStyle.fontFamily
                        font.weight: !model.hasMapping ? Font.Medium : Font.DemiBold
                        verticalAlignment: Text.AlignVCenter
                    }

                    Text {
                        height: parent.height
                        visible: model.isDefault
                        font.pixelSize: AppStyle.fontSizeMedium
                        color: Theme.textMuted
                        text: " (Default)"
                        font.family: AppStyle.fontFamily
                        font.weight: Font.Medium
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                Item {
                    Layout.preferredHeight: 32
                    Layout.preferredWidth: 32
                    Layout.alignment: Qt.AlignVCenter

                    Icon {
                        name: "bar-chart"
                        color: "yellow"
                        anchors.fill: parent
                        size: height
                        visible: model.hasConflict
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 16
        visible: false
    }
}
