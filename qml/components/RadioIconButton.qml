import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import Firelight 1.0

RoundButton {
    id: control

    property var currentIndex: 0
    property var hoveredIndex: 0
    property var model: []
    property string currentText: ""

    property bool showGlobalCursor: true
    property int globalCursorSpacing: 1

    flat: true
    
    signal selectionChanged(int index, string value, string text)

    text: "\u2713"
    
    onClicked: popup.open()

    Popup {
        id: popup
        
        x: control.width + 8
        y: 0
        width: 250
        
        padding: 8
        margins: 0
        
        focus: true
        modal: false
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
        
        onOpened: {
            // Focus the FocusScope to enable keyboard navigation
            control.hoveredIndex = control.currentIndex
            contentItem.forceActiveFocus()
            // Focus the currently selected radio button for global cursor
            if (repeater.count > 0 && control.currentIndex >= 0 && control.currentIndex < repeater.count) {
                repeater.itemAt(control.currentIndex).forceActiveFocus()
            }
        }
        
        background: Rectangle {
            color: ColorPalette.neutral900
            radius: 8
            border.color: ColorPalette.neutral700
            border.width: 1
        }
        
        contentItem: FocusScope {
            focus: true

            Keys.onBackPressed: {
                popup.close()
            }
            
            Keys.onUpPressed: {
                if (control.hoveredIndex > 0) {
                    control.hoveredIndex--
                    repeater.itemAt(control.hoveredIndex).forceActiveFocus()
                }
            }
            
            Keys.onDownPressed: {
                if (control.hoveredIndex < repeater.count - 1) {
                    control.hoveredIndex++
                    repeater.itemAt(control.hoveredIndex).forceActiveFocus()
                }
            }
            
            Keys.onSpacePressed: function (event) {
                // Select the hovered item and close
                if (control.hoveredIndex >= 0 && control.hoveredIndex < repeater.count) {
                    var hoveredItem = repeater.itemAt(control.hoveredIndex)
                    var modelData = hoveredItem.model
                    control.currentIndex = control.hoveredIndex
                    control.currentText = modelData.label || modelData.text || modelData
                    control.selectionChanged(control.hoveredIndex, modelData.value || modelData, modelData.label || modelData.text || modelData)
                    popup.close()
                }
            }
            
            Column {
                spacing: 4
                width: parent.width
                
                Repeater {
                    id: repeater
                    model: control.model
                    
                    delegate: RadioButton {
                        id: radioDelegate

                        required property var model
                        required property int index

                        property bool showGlobalCursor: true

                        width: popup.width - popup.padding * 2
                        height: 48

                        hoverEnabled: true

                        checked: control.currentIndex === index

                        onClicked: {
                            control.currentIndex = index
                            control.currentText = model.label || model.text || model
                            control.selectionChanged(index, model.value || model, model.label || model.text || model)
                            popup.close()
                        }

                        background: Rectangle {
                            color: radioDelegate.pressed ? ColorPalette.neutral700 : (radioDelegate.hovered ? ColorPalette.neutral800 : "transparent")
                            radius: 4
                        }

                        indicator: Rectangle {
                            implicitWidth: 20
                            implicitHeight: 20
                            x: parent.width - width - 8
                            y: parent.height / 2 - height / 2
                            radius: 10
                            border.color: radioDelegate.checked ? "#21be2b" : ColorPalette.neutral400
                            border.width: 2

                            Rectangle {
                                width: 10
                                height: 10
                                x: 5
                                y: 5
                                radius: 5
                                color: "#21be2b"
                                visible: radioDelegate.checked
                            }
                        }

                        contentItem: Text {
                            leftPadding: 8
                            rightPadding: radioDelegate.indicator.width + 16
                            text: model.label || model.text || model
                            font.family: Constants.regularFontFamily
                            font.pixelSize: 14
                            color: radioDelegate.checked ? ColorPalette.neutral100 : ColorPalette.neutral300
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }
        }
    }
}
