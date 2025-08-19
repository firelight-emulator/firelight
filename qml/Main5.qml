import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQml.Models
import QtQuick.Layouts 1.0
import QtNetwork
import QtMultimedia
import QtQuick.Effects
import Firelight 1.0

ApplicationWindow {
    id: window
    objectName: "Application Window"

    width: GeneralSettings.mainWindowWidth
    height: GeneralSettings.mainWindowHeight
    x: GeneralSettings.mainWindowX
    y: GeneralSettings.mainWindowY

    onWidthChanged: {
        GeneralSettings.mainWindowWidth = width
    }

    onHeightChanged: {
        GeneralSettings.mainWindowHeight = height
    }

    onXChanged: {
        GeneralSettings.mainWindowX = x
    }

    onYChanged: {
        GeneralSettings.mainWindowY = y
    }
    visible: true
    visibility: GeneralSettings.fullscreen ? Window.FullScreen : Window.Windowed

    background: Rectangle {
        color: ColorPalette.neutral900
    }

    title: qsTr("Firelight")

    ListView {
        model: options
        anchors.fill: parent

        section.property: "category"
        section.criteria: ViewSection.FullString
        section.delegate: SettingsSectionHeader {
            required property string section
            sectionName: section
        }
    }

    DelegateModel {
        id: options
        model: EmulatorSettingsModel {
            platformId: 7
        }
        delegate: DelegateChooser {
            role: "type"
            DelegateChoice {
                roleValue: "toggle"
                delegate: BaseSettingItem {
                    required property var model
                    width: parent.width
                    label: model.name
                    controlComponent: CheckBox {
                        checked: model.value
                    }
                    description: model.description
                }
            }
            DelegateChoice {
                roleValue: "text"
                delegate:  BaseSettingItem {
                  required property var model
                    width: parent.width
                  label: model.name
                    controlComponent: TextEdit {
                        text: model.value
                    }
                  description: model.description
              }
            }
            DelegateChoice {
                roleValue: "dropdown"
                delegate:  BaseSettingItem {
                  required property var model
                    width: parent.width
                  label: model.name
                    controlComponent: ComboBox {
                        model: [ "Banana", "Apple", "Coconut" ]
                    }
                  description: model.description
              }
            }
        }
    }
}

