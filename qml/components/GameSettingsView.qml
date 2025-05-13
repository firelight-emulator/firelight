import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FocusScope {
    required property GameSettings gameSettings
    ColumnLayout {
        spacing: 8
        anchors.fill: parent
        // ComboBoxOption {
        //     Layout.fillWidth: true
        //     Layout.preferredHeight: 60
        //     label: "Window Mode?"
        // }

        ComboBoxOption2 {
            Layout.fillWidth: true
            label: "Picture mode"
            description: "Determines how the game image fills the screen."

            property bool initialized: false

            Component.onCompleted: {
                for (let i = 0; i < model.length; i++) {
                    if (model[i].value === gameSettings.pictureMode) {
                        currentIndex = i
                        break
                    }
                }
                initialized = true;
            }

            model: [
                {text: "Aspect Ratio Fill", value: "aspect-ratio-fill"},
                {text: "Integer Scale", value: "integer-scale"},
                {text: "Stretch", value: "stretch"}
            ]

            onCurrentValueChanged: {
                if (!initialized) {
                    return
                }
                gameSettings.pictureMode = currentValue
                // console.log("Picture mode changed to: " + currentValue)
            }
        }
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

        }
    }
}