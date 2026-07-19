import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// The emulation settings for a single tier. Callers set `level` plus the scope
// (`platformId`, `contentHash`). Each row shows the effective value; a row that
// overrides an inherited one gets a Reset
//
// The rows themselves are just the catalog's `emulation` page — SettingsGroup
// renders them, the same as every other settings page. What's special here is
// the footer: the core's own raw libretro options, which aren't catalog settings
// at all and get their own drill-down
FocusScope {
    id: root

    property int level: SettingsLevel.Game
    property var platformId: -1
    property var contentHash: ""
    // Driven by the global "Show advanced settings" preference — reveals both
    // advanced friendly settings and the raw core-options section
    property bool advancedOpen: GeneralSettings.showAdvancedSettings

    // Raw libretro options for the "Advanced" section (populated once a game on
    // this console has run this session)
    CoreOptionsModel {
        id: coreOptionsModel
        level: root.level
        platformId: root.platformId
        contentHash: root.contentHash
    }

    Flickable {
        id: flickable
        anchors.fill: parent
        contentWidth: width
        contentHeight: column.implicitHeight
        boundsBehavior: Flickable.StopAtBounds
        clip: true

        ScrollBar.vertical: FLScrollBar {}

        ColumnLayout {
            id: column
            width: Math.min(flickable.width, 800)
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 0

            SettingsPage {
                Layout.fillWidth: true
                page: "emulation"
                level: root.level
                platformId: root.platformId
                contentHash: root.contentHash
            }

            // Advanced (raw core options) — only for a specific console, and
            // only when advanced settings are on. Not catalog settings: these
            // are whatever the core declared at runtime
            ColumnLayout {
                Layout.fillWidth: true
                visible: root.platformId !== -1 && root.advancedOpen
                spacing: 8

                Rectangle {
                    Layout.fillWidth: true
                    Layout.topMargin: 16
                    Layout.preferredHeight: 1
                    color: Theme.border
                }

                Text {
                    Layout.fillWidth: true
                    text: qsTr("Advanced core options")
                    color: Theme.textPrimary
                    leftPadding: 8
                    topPadding: 8
                    font.pixelSize: AppStyle.fontSizeMedium
                    font.family: Constants.regularFontFamily
                    font.weight: Font.DemiBold
                }

                Text {
                    Layout.fillWidth: true
                    leftPadding: 8
                    visible: coreOptionsModel.categories.length === 0
                    text: qsTr("These are the core's raw options. They appear after a game on this console has run at least once.")
                    color: Theme.textMuted
                    wrapMode: Text.WordWrap
                    font.pixelSize: AppStyle.fontSizeSmall
                    font.family: Constants.regularFontFamily
                    font.weight: Font.Medium
                }

                // Inside a category: a back link to the category list
                Button {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    visible: coreOptionsModel.categoryFilter !== ""
                    hoverEnabled: true
                    focusPolicy: Qt.NoFocus
                    background: Rectangle {
                        color: ColorPalette.neutral300
                        opacity: parent.hovered ? 0.2 : 0.08
                        radius: 8
                    }
                    contentItem: Text {
                        text: "‹  " + qsTr("All categories")
                        leftPadding: 8
                        color: Theme.textPrimary
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: AppStyle.fontSizeMedium
                        font.family: Constants.regularFontFamily
                        font.weight: Font.DemiBold
                    }
                    onClicked: coreOptionsModel.categoryFilter = ""
                }

                // Top level: one entry per category; click to drill in
                Repeater {
                    model: coreOptionsModel.categoryFilter === "" ? coreOptionsModel.categories : null

                    Button {
                        required property var modelData
                        Layout.fillWidth: true
                        Layout.preferredHeight: 56
                        hoverEnabled: true
                        focusPolicy: Qt.NoFocus
                        background: Rectangle {
                            color: ColorPalette.neutral300
                            opacity: parent.hovered ? 0.2 : 0.08
                            radius: 8
                        }
                        contentItem: RowLayout {
                            Text {
                                Layout.fillWidth: true
                                leftPadding: 8
                                text: modelData.label
                                color: Theme.textPrimary
                                verticalAlignment: Text.AlignVCenter
                                font.pixelSize: AppStyle.fontSizeMedium
                                font.family: Constants.regularFontFamily
                                font.weight: Font.DemiBold
                            }
                            Text {
                                rightPadding: 8
                                text: "›"
                                color: Theme.textMuted
                                font.pixelSize: AppStyle.fontSizeLarge
                            }
                        }
                        onClicked: coreOptionsModel.categoryFilter = modelData.key
                    }
                }

                Repeater {
                    model: coreOptionsModel

                    ComboBoxSettingItem {
                        id: advancedItem
                        required property var model
                        required property var index
                        Layout.fillWidth: true
                        label: model.label
                        description: model.description
                        resettable: model.resettable

                        textRole: "label"
                        valueRole: "value"
                        comboBoxModel: model.options

                        currentIndex: {
                            for (let i = 0; i < model.options.length; i++) {
                                if (model.options[i].value === model.value) {
                                    return i;
                                }
                            }
                            return -1;
                        }

                        onReset: coreOptionsModel.resetValue(index)
                        onClicked: function () {
                            advancedItem.popup.open();
                            advancedItem.popup.forceActiveFocus();
                        }
                        onCurrentValueChanged: {
                            if (model.value !== currentValue) {
                                model.value = currentValue;
                            }
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 16
                }
            }
        }
    }
}
