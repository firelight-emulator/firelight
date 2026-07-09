import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform

FocusScope {
    implicitWidth: col.width
    implicitHeight: col.height

    // Sample the chosen background image so the theme can tint itself to match
    // it (top band -> Theme.tintTop, bottom band -> Theme.tintBottom). No-op
    // when no image is set or it can't be read.
    function resampleBackgroundImage() {
        var file = AppearanceSettings.backgroundFile;
        if (file === "") {
            return;
        }
        var palette = ImageUtils.samplePalette(file);
        if (palette.ok) {
            AppearanceSettings.imageColorTop = palette.top;
            AppearanceSettings.imageColorBottom = palette.bottom;
        }
    }

    // Sample once if an image is already set but hasn't been sampled yet.
    Component.onCompleted: {
        if (AppearanceSettings.backgroundMode === "image"
                && AppearanceSettings.backgroundFile !== ""
                && AppearanceSettings.imageColorTop === "") {
            resampleBackgroundImage();
        }
    }

    // A labelled 0..1 slider row bound to a theme knob.
    component ThemeSlider: RowLayout {
        id: sliderRoot
        property string labelText: ""
        property real val: 0
        signal moved(real v)

        Layout.fillWidth: true
        Layout.leftMargin: 8
        Layout.topMargin: 4
        spacing: 12

        Text {
            Layout.preferredWidth: 180
            text: sliderRoot.labelText
            color: Theme.textPrimary
            font.family: Constants.regularFontFamily
            font.pixelSize: 15
            font.weight: Font.Medium
            verticalAlignment: Text.AlignVCenter
        }
        Slider {
            Layout.fillWidth: true
            from: 0
            to: 1
            value: sliderRoot.val
            onMoved: sliderRoot.moved(value)
        }
    }

    ColumnLayout {
        id: col
        spacing: 4
        anchors.fill: parent

        SettingsSectionHeader {
            sectionName: "Background"
            showTopPadding: false
        }

        // Background type selector.
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 8
            Layout.topMargin: 4
            spacing: 8

            Repeater {
                model: [
                    { key: "solid", label: "Solid" },
                    { key: "gradient", label: "Gradient" },
                    { key: "image", label: "Image" }
                ]
                delegate: Button {
                    id: modeButton
                    required property var modelData
                    text: modelData.label
                    checkable: true
                    checked: AppearanceSettings.backgroundMode === modelData.key
                    onClicked: AppearanceSettings.backgroundMode = modelData.key

                    contentItem: Text {
                        text: modeButton.text
                        color: modeButton.checked ? "#ffffff" : ColorPalette.neutral100
                        font.family: Constants.regularFontFamily
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        implicitWidth: 96
                        implicitHeight: 34
                        radius: 6
                        color: ColorPalette.neutral300
                        opacity: modeButton.checked ? 0.28 : (modeButton.hovered ? 0.16 : 0.08)
                        Behavior on opacity {
                            NumberAnimation { duration: 100; easing.type: Easing.InOutQuad }
                        }
                    }
                }
            }
        }

        ColorSettingItem {
            Layout.fillWidth: true
            visible: AppearanceSettings.backgroundMode !== "image"
            label: AppearanceSettings.backgroundMode === "gradient" ? "First color" : "Color"
            value: AppearanceSettings.backgroundColor
            onPicked: function (hex) {
                AppearanceSettings.backgroundColor = hex;
            }
        }

        ColorSettingItem {
            Layout.fillWidth: true
            visible: AppearanceSettings.backgroundMode === "gradient"
            label: "Second color"
            value: AppearanceSettings.backgroundColor2
            onPicked: function (hex) {
                AppearanceSettings.backgroundColor2 = hex;
            }
        }

        FileOption {
            id: customBackgroundFile
            visible: AppearanceSettings.backgroundMode === "image"
            Layout.fillWidth: true
            label: "Background file"
            value: AppearanceSettings.backgroundFile

            background: Rectangle {
                color: ColorPalette.neutral300
                radius: 8
                border.color: ColorPalette.neutral500
                opacity: parent.hovered || (!InputMethodManager.usingMouse && customBackgroundFile.activeFocus) ? 0.2 : 0.1

                Behavior on opacity {
                    NumberAnimation {
                        duration: 100
                        easing.type: Easing.InOutQuad
                    }
                }
            }

            onValueChanged: function () {
                AppearanceSettings.backgroundFile = value
                resampleBackgroundImage()
            }
        }

        Text {
            font.pixelSize: 15
            visible: AppearanceSettings.backgroundMode === "image"
            font.family: Constants.regularFontFamily
            font.weight: Font.Medium
            Layout.fillWidth: true
            color: Theme.textPrimary
            text: "Choose an image or gif (pronounced jif) to use as your background."
            leftPadding: 12
            lineHeight: 1.2
            wrapMode: Text.WordWrap
            Layout.bottomMargin: 12
        }

        SettingsSectionHeader {
            sectionName: "Theme"
        }

        ThemeSlider {
            labelText: "Background blur"
            val: AppearanceSettings.backgroundBlur
            onMoved: function (v) { AppearanceSettings.backgroundBlur = v }
        }
        ThemeSlider {
            labelText: "Background dim"
            visible: AppearanceSettings.backgroundMode === "image"
            val: AppearanceSettings.backgroundDim
            onMoved: function (v) { AppearanceSettings.backgroundDim = v }
        }
        ThemeSlider {
            labelText: "Surface tint"
            val: AppearanceSettings.themeIntensity
            onMoved: function (v) { AppearanceSettings.themeIntensity = v }
        }
        ThemeSlider {
            labelText: "Glass translucency"
            val: AppearanceSettings.glassOpacity
            onMoved: function (v) { AppearanceSettings.glassOpacity = v }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
