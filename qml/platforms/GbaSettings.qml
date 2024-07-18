import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

FocusScope {
    Flickable {
        anchors.fill: parent
        contentHeight: column.height
        ColumnLayout {
            id: column
            width: parent.width

            // Image {
            //     Layout.fillWidth: true
            //     Layout.preferredHeight: parent.width
            //     source: "file:system/_img/snes.svg"
            //     sourceSize.height: height
            //     fillMode: Image.PreserveAspectFit
            // }


            Text {
                Layout.fillWidth: true
                text: qsTr("Default Game Boy Advance Settings")
                font.pointSize: 16
                font.family: Constants.regularFontFamily
                font.weight: Font.Bold
                Layout.bottomMargin: 8
                color: "white"
            }

            Text {
                Layout.fillWidth: true
                text: qsTr("These are the default settings for GBA games. These settings can be overridden on a per-game basis by selecting the game in your library and going to 'Settings'.")
                font.pointSize: 11
                font.family: Constants.regularFontFamily
                font.weight: Font.Normal
                wrapMode: Text.WordWrap
                Layout.bottomMargin: 8
                color: "white"
            }

            Rectangle {
                Layout.topMargin: 8
                Layout.bottomMargin: 8
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#333333"
            }

            Text {
                Layout.fillWidth: true
                text: qsTr("Advanced settings")
                font.pointSize: 11
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
                Layout.bottomMargin: 8
                color: "#a6a6a6"
            }

            ToggleOption {
                Layout.fillWidth: true
                Layout.minimumHeight: 42
                label: "Allow opposing D-Pad directions"
                description: "Enabling this will allow pressing / quickly alternating / holding both left and right (or up and down) directions at the same time. This may cause movement-based glitches."
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#333333"
            }

            ToggleOption {
                Layout.fillWidth: true
                Layout.minimumHeight: 42
                label: "Enable low pass audio filter"
                description: "Enables a low pass audio filter to reduce the 'harshness' of generated audio."
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#333333"
            }

            ToggleOption {
                Layout.fillWidth: true
                Layout.minimumHeight: 42
                label: "Color correction"
                description: "Adjusts output colors to match the display of real GBA hardware."
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#333333"
            }

            ToggleOption {
                Layout.fillWidth: true
                Layout.minimumHeight: 42
                label: "Game Boy Player Rumble (Restart)"
                description: "Enabling this will allow compatible games with the Game Boy Player boot logo to make the controller rumble. Due to how Nintendo decided this feature should work, it may cause glitches such as flickering or lag in some of these games."
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#333333"
            }

            ToggleOption {
                Layout.fillWidth: true
                Layout.minimumHeight: 42
                label: "Idle Loop Optimization"
                description: "Reduce system load by optimizing so-called 'idle-loops' - sections in the code where nothing happens, but the CPU runs at full speed (like a car revving in neutral). Improves performance, and should be enabled on low-end hardware."
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#333333"
            }

            ToggleOption {
                Layout.fillWidth: true
                Layout.minimumHeight: 42
                label: "Interframe Blending"
                description: "Simulates LCD ghosting effects. 'Simple' performs a 50:50 mix of the current and previous frames. 'Smart' attempts to detect screen flickering, and only performs a 50:50 mix on affected pixels. 'LCD Ghosting' mimics natural LCD response times by combining multiple buffered frames. 'Simple' or 'Smart' blending is required when playing games that aggressively exploit LCD ghosting for transparency effects (Wave Race, Chikyuu Kaihou Gun ZAS, F-Zero, the Boktai series...)."
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#333333"
            }

            ToggleOption {
                Layout.fillWidth: true
                Layout.minimumHeight: 42
                label: "Solar sensor level"
                description: "Sets ambient sunlight intensity. Can be used by games that included a solar sensor in their cartridges, e.g: the Boktai series."
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#333333"
            }

            ToggleOption {
                Layout.fillWidth: true
                Layout.minimumHeight: 42
                label: "Use BIOS"
                description: "Use official BIOS/bootloader for emulated hardware, if present in RetroArch's system directory."
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#333333"
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }

}