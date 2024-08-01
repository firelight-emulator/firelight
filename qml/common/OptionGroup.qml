import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    id: root
    required property string label

    implicitHeight: theLabel.implicitHeight + thePane.implicitHeight
    implicitWidth: theLabel.implicitWidth + thePane.implicitWidth

    default property alias content: stuffCol.children

    Text {
        id: theLabel
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        text: root.label
        font.pixelSize: 15
        font.family: Constants.regularFontFamily
        font.weight: Font.DemiBold
        Layout.bottomMargin: 4
        color: ColorPalette.neutral400
    }

    Pane {
        id: thePane
        anchors.top: theLabel.bottom
        anchors.topMargin: 8
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        topPadding: stuffCol.spacing
        bottomPadding: topPadding * 3

        background: Rectangle {
            radius: 8
            color: ColorPalette.neutral800
        }

        contentItem: ColumnLayout {
            id: stuffCol

            // ToggleOption {
            //     Layout.fillWidth: true
            //     label: "Simulate LCD ghosting"
            //     // description: "Enables simulation of LCD ghosting effects by blending the current and previous frames."
            //
            //     Component.onCompleted: {
            //         checked = emulator_config_manager.getOptionValueForPlatform(1, "gambatte_mix_frames") === "accurate"
            //     }
            //
            //     onCheckedChanged: {
            //         if (checked) {
            //             emulator_config_manager.setOptionValueForPlatform(1, "gambatte_mix_frames", "accurate")
            //         } else {
            //             emulator_config_manager.setOptionValueForPlatform(1, "gambatte_mix_frames", "disabled")
            //         }
            //     }
            //
            // }
        }
    }
}

