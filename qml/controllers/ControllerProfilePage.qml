import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0

FocusScope {
    id: root

    // property alias controllerProfileId: gamepadProfile.profileId
    required property var playerNumber
    required property var profileId

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

                 onToggled: {
                     if (checked) {
                         theStack.replaceCurrentItem(shortcutsList, {}, StackView.Immediate)
                     }
                 }
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
                             theStack.replaceCurrentItem(mappingView, {platformId: model.platform_id, profileId: gamepadStatus.profileId, platformMetadataModel: model, gamepad: gamepadStatus, isKeyboard: profile.isKeyboardProfile}, StackView.Immediate)
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
            id: shortcutsListView
            ShortcutInputPromptDialog {
                id: shortcutDialog
                shortcutName: ""
                shortcut: 0
                gamepad: gamepadStatus
                isKeyboard: profile.isKeyboardProfile

                onMappingAdded: function(shortcut, modifiers, input) {
                    shortcutsListView.model.setMapping(shortcut, modifiers, input);
                }
            }
            header: Text {
                text: "Shortcuts"
                color: "white"
                font.pixelSize: 20
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
                          shortcutDialog.shortcut = model.shortcut
                          shortcutDialog.shortcutName = model.name
                          shortcutDialog.open()
                        // dialog.buttons = []
                        // dialog.buttons = [{
                        //     display_name: model.originalInputName,
                        //     retropad_button: model.originalInput
                        // }]
                        // dialog.open()
                      }
                  }

                  // RightClickMenuItem {
                  //     text: "Reset to default"
                  //
                  //     // onTriggered: {
                  //     //   inputMappingsModel.resetToDefault(model.originalInput)
                  //     // }
                  // }

                  RightClickMenuItem {
                      text: "Clear mapping"
                      onTriggered: {
                            model.hasMapping = false
                      }
                  }
              }
              onClicked: function() {
                  ListView.view.currentIndex = index;
              }
              onDoubleClicked: function() {
                  shortcutDialog.shortcut = model.shortcut
                  shortcutDialog.shortcutName = model.name
                  shortcutDialog.open()
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
                      color: "white"
                      font.pixelSize: 15
                      font.family: Constants.regularFontFamily
                      font.weight: Font.DemiBold
                      verticalAlignment: Text.AlignVCenter
                  }

                  Row {
                      Layout.preferredWidth: 240
                      Layout.maximumWidth: 240
                      Layout.alignment: Qt.AlignLeft
                      Layout.fillHeight: true

                      Repeater {
                          model: myDelegate.model.modifierNames
                          delegate: Text {
                              height: parent.height
                              text: modelData + " + "
                              color: "white"
                              font.pixelSize: 15
                              font.family: Constants.regularFontFamily
                              font.weight: Font.DemiBold
                              verticalAlignment: Text.AlignVCenter
                          }
                      }

                      Text {
                          height: parent.height
                          // text: inputMapping.inputMappings[modelData.retropad_button] === undefined ? (gamepadStatus.inputLabels[modelData.retropad_button] + " (default)") : gamepadStatus.inputLabels[inputMapping.inputMappings[modelData.retropad_button]]
                          // color: inputMapping.inputMappings[modelData.retropad_button] === undefined ? ColorPalette.neutral400 : "white"
                          font.pixelSize: 15
                          color: model.hasConflict ? "yellow" : (!model.hasMapping ? ColorPalette.neutral400 : "white")
                          text: model.hasMapping ? model.inputName : "(Not mapped)"

                          font.family: Constants.regularFontFamily
                          font.weight: !model.hasMapping ? Font.Medium : Font.DemiBold
                          verticalAlignment: Text.AlignVCenter
                      }
                  }


                  Item {
                      Layout.preferredHeight: 32
                      Layout.preferredWidth: 32
                      Layout.alignment: Qt.AlignVCenter

                      FLIcon {
                          icon: "bar-chart"
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
    }


}