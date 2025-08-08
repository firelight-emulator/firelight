import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

FocusScope {
    id: root

    // property alias controllerProfileId: gamepadProfile.profileId
    required property var playerNumber

    GamepadStatus {
        id: gamepadStatus
        playerNumber: root.playerNumber
    }

    FirelightDialog {
        id: confirmDialog

        property var buttonList: []
        property var currentIndex: 0

        text: "You're about to walk through assigning an input on your controller to each " + platformList.currentItem.model.display_name + " input.\n\n Continue?"
        showButtons: true

        onAccepted: function () {
            dialog.buttons = platformList.currentItem.model.buttons
            dialog.currentIndex = 0
            dialog.open()
        }
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

        // background: Rectangle {
        //     color: ColorPalette.neutral800
        // }
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

                 // onToggled: {
                 //     if (checked) {
                 //         ListView.view.currentIndex = index
                 //         col.forceActiveFocus()
                 //     }
                 // }
             }
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: ColorPalette.neutral600
            }
            ListView {
                 id: platformList
                 spacing: 0
                 Layout.fillHeight: true
                 Layout.fillWidth: true
                 // Layout.maximumWidth: 300
                 // Layout.preferredWidth: 300
                 // Layout.minimumWidth: 200
                 // Layout.fillHeight: true
                 // Layout.alignment: Qt.AlignRight
                 focus: true
                 clip: true

                 KeyNavigation.right: theStack

                 Keys.onBackPressed: {
                     root.StackView.view.popCurrentItem(StackView.PopTransition)
                 }

                 Keys.onEscapePressed: {
                     root.StackView.view.popCurrentItem(StackView.PopTransition)
                 }

                 currentIndex: 0

                 // onCurrentItemChanged: {
                 //     gamepadProfile.currentPlatformId = platformList.currentItem.model.platform_id
                 // }

                 model: platform_model
                 delegate: FirelightMenuItem {
                     required property var model
                     required property var index
                     // focus: true

                    ButtonGroup.group: menuButtonGroup

                     labelText: model.display_name
                     labelIcon: model.icon_name
                     property bool showGlobalCursor: true
                     // labelIcon: "\ue40a"
                     height: 54
                     width: ListView.view.width

                     // checked: ListView.isCurrentItem

                     onToggled: {
                         if (checked) {
                             ListView.view.currentIndex = index
                             theStack.replaceCurrentItem(mappingView, {platformId: model.platform_id, profileId: gamepadStatus.profileId, platformMetadataModel: model, gamepad: gamepadStatus}, StackView.Immediate)
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
        color: ColorPalette.neutral600
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

        ControllerInputMappingView {
        }
    }

    Component {
        id: shortcutsList
        ListView {
            model: ListModel {
                ListElement { name: "A"; key: "A" }
                ListElement { name: "B"; key: "B" }
                ListElement { name: "X"; key: "X" }
                ListElement { name: "Y"; key: "Y" }
                ListElement { name: "Start"; key: "Start" }
                ListElement { name: "Select"; key: "Select" }
                ListElement { name: "Left Shoulder"; key: "LeftShoulder" }
                ListElement { name: "Right Shoulder"; key: "RightShoulder" }
                ListElement { name: "Left Stick"; key: "LeftStick" }
                ListElement { name: "Right Stick"; key: "RightStick" }
            }
            delegate: Button {
                required property var model
                text: model.name
                width: parent.width
                height: 40
            }
        }
    }


}