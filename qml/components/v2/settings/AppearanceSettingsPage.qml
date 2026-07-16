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

    ColumnLayout {
        id: col
        spacing: 0
        anchors.fill: parent

        SettingsSection {
            Layout.fillWidth: true
            title: "Theme"
            showTopPadding: false

            ColorSettingItem {
                Layout.fillWidth: true
                controlBelow: false
                label: "Accent color"
                value: AppearanceSettings.accentColor
                onPicked: function (hex) {
                    AppearanceSettings.accentColor = hex;
                }
            }

            SegmentedSettingItem {
                Layout.fillWidth: true
                label: "Background type"
                options: [
                    { "label": "Solid", "value": "solid" },
                    { "label": "Gradient", "value": "gradient" },
                    { "label": "Image", "value": "image" }
                ]
                currentValue: AppearanceSettings.backgroundMode
                controlBelow: false
                onChanged: AppearanceSettings.backgroundMode = value
            }

            SettingSubGroup {
                Layout.fillWidth: true
                ColorSettingItem {
                    shown: AppearanceSettings.backgroundMode !== "image"
                    label: AppearanceSettings.backgroundMode === "gradient" ? "First color" : "Color"
                    value: AppearanceSettings.backgroundColor
                    controlBelow: false
                    onPicked: function (hex) {
                        AppearanceSettings.backgroundColor = hex;
                    }
                }


                ColorSettingItem {
                    shown: AppearanceSettings.backgroundMode === "gradient"
                    label: "Second color"
                    controlBelow: false
                    value: AppearanceSettings.backgroundColor2
                    onPicked: function (hex) {
                        AppearanceSettings.backgroundColor2 = hex;
                    }
                }
            }

            SettingSubGroup {
                shown: AppearanceSettings.backgroundMode === "image"
                Layout.fillWidth: true

                FilePathSettingItem {
                    label: "Image file"
                    description: "Choose an image or gif (pronounced 'jif') to use as your background"
                    // The setting is stored as a file:// URL — Image.source and
                    // samplePalette both need one — but this control deals in
                    // plain paths, so convert at the boundary.
                    value: FilesystemUtils.removeFileURI(AppearanceSettings.backgroundFile)
                    controlBelow: false
                    extensions: ["png", "jpg", "jpeg", "gif", "webp", "bmp"]
                    onChosen: function (path) {
                        AppearanceSettings.backgroundFile =
                            path === "" ? "" : FilesystemUtils.prependFileURI(path);
                        resampleBackgroundImage();
                    }
                }

                SliderSettingItem {
                    Layout.fillWidth: true
                    label: "Image blur"
                    from: 0
                    to: 1
                    stepSize: 0.01
                    controlBelow: false
                    value: AppearanceSettings.backgroundBlur
                    onMoved: function (v) { AppearanceSettings.backgroundBlur = v }
                }
            }
        }

        SettingsSection {
            Layout.fillWidth: true
            title: "Stuff"

            SliderSettingItem {
                Layout.fillWidth: true
                label: "Background dim"
                shown: AppearanceSettings.backgroundMode === "image"
                from: 0
                to: 1
                stepSize: 0.01
                controlBelow: false
                value: AppearanceSettings.backgroundDim
                onMoved: function (v) { AppearanceSettings.backgroundDim = v }
            }

            SliderSettingItem {
                Layout.fillWidth: true
                label: "Surface tint"
                from: 0
                to: 1
                stepSize: 0.01
                controlBelow: false
                value: AppearanceSettings.themeIntensity
                onMoved: function (v) { AppearanceSettings.themeIntensity = v }
            }

            SliderSettingItem {
                Layout.fillWidth: true
                label: "Glass translucency"
                from: 0
                to: 1
                stepSize: 0.01
                controlBelow: false
                value: AppearanceSettings.glassOpacity
                onMoved: function (v) { AppearanceSettings.glassOpacity = v }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
