import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// Bespoke delegate for the "shader-picker" widget: a strip of shader presets
// (with a leading "None" tile) from the ShaderStore model, plus a live panel of
// parameter sliders for the selected preset. The stored value is either "" or a
// JSON blob {"preset":id,"params":{key:val,...}}.
FocusScope {
    id: root

    property string label: ""
    property string description: ""
    property string value: ""
    property bool resettable: false

    signal reset()
    signal activated(string value)

    implicitHeight: col.implicitHeight

    // --- value (de)serialization -------------------------------------------------
    function parsed() {
        if (!root.value) {
            return { preset: "", params: {} }
        }
        try {
            const o = JSON.parse(root.value)
            if (o && typeof o === "object") {
                return { preset: o.preset || "", params: o.params || {} }
            }
        } catch (e) {
            // Bare preset id (no params).
        }
        return { preset: root.value, params: {} }
    }

    readonly property string selectedId: parsed().preset
    readonly property var selectedParams: parsed().params

    function selectPreset(id) {
        if (id === "") {
            root.activated("")
            return
        }
        // Seed params with the preset's declared defaults.
        const defs = ShaderStore.parametersFor(id)
        const params = {}
        for (let i = 0; i < defs.length; i++) {
            params[defs[i].key] = defs[i].default
        }
        root.activated(JSON.stringify({ preset: id, params: params }))
    }

    function setParam(key, val) {
        const p = parsed()
        if (p.preset === "") {
            return
        }
        p.params[key] = val
        root.activated(JSON.stringify({ preset: p.preset, params: p.params }))
    }

    component ShaderTile: Rectangle {
        id: tile
        property string tileName: ""
        property string tilePreview: ""
        property bool selected: false
        signal clicked()

        width: 170
        height: 130
        radius: 8
        color: ColorPalette.neutral800
        border.width: selected ? 3 : 1
        border.color: selected ? Theme.textPrimary : Theme.border

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 6

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 4
                color: "black"
                clip: true

                Image {
                    anchors.fill: parent
                    visible: tile.tilePreview !== ""
                    source: tile.tilePreview
                    fillMode: Image.PreserveAspectCrop
                    asynchronous: true
                    cache: true
                }
                Text {
                    anchors.centerIn: parent
                    visible: tile.tilePreview === ""
                    text: tile.tileName === qsTr("None") ? qsTr("Off") : tile.tileName
                    color: Theme.textMuted
                    font.pixelSize: 14
                    font.family: Constants.regularFontFamily
                }
            }
            Text {
                Layout.fillWidth: true
                text: tile.tileName
                color: Theme.textPrimary
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 13
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
            }
        }

        HoverHandler { cursorShape: Qt.PointingHandCursor }
        TapHandler { onTapped: tile.clicked() }
    }

    ColumnLayout {
        id: col
        width: parent.width
        spacing: 4

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 8
            Layout.rightMargin: 8

            Text {
                Layout.fillWidth: true
                text: root.label
                color: Theme.textPrimary
                font.pixelSize: 16
                font.family: Constants.regularFontFamily
                font.weight: Font.DemiBold
                verticalAlignment: Text.AlignVCenter
            }

            Button {
                visible: root.resettable
                padding: 6
                focusPolicy: Qt.NoFocus
                hoverEnabled: true
                onClicked: root.reset()
                HoverHandler { cursorShape: Qt.PointingHandCursor }
                background: Rectangle {
                    radius: 6
                    color: parent.hovered ? ColorPalette.neutral300 : "transparent"
                    opacity: parent.hovered ? 0.2 : 0
                }
                contentItem: Text {
                    text: qsTr("Reset")
                    color: Theme.textMuted
                    font.pixelSize: 14
                    font.family: Constants.regularFontFamily
                    font.weight: Font.Medium
                }
            }
        }

        Text {
            Layout.fillWidth: true
            Layout.leftMargin: 8
            Layout.rightMargin: 8
            visible: root.description !== ""
            text: root.description
            color: Theme.textMuted
            font.pixelSize: 15
            font.family: Constants.regularFontFamily
            font.weight: Font.Medium
            wrapMode: Text.WordWrap
        }

        Flickable {
            Layout.fillWidth: true
            Layout.preferredHeight: 140
            Layout.topMargin: 4
            leftMargin: 8
            rightMargin: 8
            clip: true
            contentWidth: strip.width
            contentHeight: height
            flickableDirection: Flickable.HorizontalFlick
            ScrollBar.horizontal: ScrollBar {}

            Row {
                id: strip
                height: parent.height
                spacing: 10

                ShaderTile {
                    anchors.verticalCenter: parent.verticalCenter
                    tileName: qsTr("None")
                    tilePreview: ""
                    selected: root.selectedId === ""
                    onClicked: root.selectPreset("")
                }

                Repeater {
                    model: ShaderStore
                    delegate: ShaderTile {
                        required property string shaderId
                        required property string name
                        required property string previewUrl
                        anchors.verticalCenter: parent.verticalCenter
                        tileName: name
                        tilePreview: previewUrl
                        selected: root.selectedId === shaderId
                        onClicked: root.selectPreset(shaderId)
                    }
                }
            }
        }

        // Live preview of the selected shader on a sample frame (updates as the
        // sliders below move). Hidden for "None".
        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 8
            Layout.preferredWidth: 360
            Layout.preferredHeight: 270
            visible: root.selectedId !== ""
            color: "black"
            radius: 8
            border.width: 1
            border.color: Theme.border
            clip: true

            ShaderPreviewItem {
                anchors.fill: parent
                anchors.margins: 1
                shaderId: root.selectedId
                params: JSON.stringify(root.selectedParams)
            }
        }

        // Parameter sliders for the selected preset.
        ColumnLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 8
            Layout.rightMargin: 8
            Layout.bottomMargin: 8
            spacing: 2
            visible: paramRepeater.count > 0

            Repeater {
                id: paramRepeater
                model: root.selectedId !== ""
                       ? ShaderStore.parametersFor(root.selectedId) : []

                RowLayout {
                    required property var modelData
                    Layout.fillWidth: true
                    spacing: 12

                    Text {
                        Layout.preferredWidth: 200
                        text: modelData.label
                        color: Theme.textPrimary
                        font.pixelSize: 15
                        font.family: Constants.regularFontFamily
                        elide: Text.ElideRight
                    }
                    Slider {
                        Layout.fillWidth: true
                        from: modelData.min
                        to: modelData.max
                        stepSize: modelData.step
                        value: {
                            const p = root.selectedParams
                            return (p && p[modelData.key] !== undefined)
                                   ? p[modelData.key] : modelData.default
                        }
                        onMoved: root.setParam(modelData.key, value)
                    }
                    Text {
                        Layout.preferredWidth: 48
                        horizontalAlignment: Text.AlignRight
                        text: {
                            const p = root.selectedParams
                            const v = (p && p[modelData.key] !== undefined)
                                      ? p[modelData.key] : modelData.default
                            return Number(v).toFixed(2)
                        }
                        color: Theme.textMuted
                        font.pixelSize: 14
                        font.family: Constants.regularFontFamily
                    }
                }
            }
        }
    }
}
