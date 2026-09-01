// TODO: NEEDS REVIEW
import QtQuick
import Firelight 1.0

FLButton {
    id: control

    // {key, modifiers, text}
    required property var bindings
    required property bool actionEnabled

    readonly property var _glyphs: control._buildGlyphs()

    focusPolicy: Qt.NoFocus
    variant: "subtle"


    enabled: control.actionEnabled
    canInteract: control.actionEnabled

    // TODO
    // The keys a binding draws as, modifiers first and the key itself last
    function _keysOf(binding) {
        let keys = [];

        if (binding.modifiers & Qt.ControlModifier) {
            keys.push(Qt.Key_Control);
        }

        if (binding.modifiers & Qt.ShiftModifier) {
            keys.push(Qt.Key_Shift);
        }

        if (binding.modifiers & Qt.AltModifier) {
            keys.push(Qt.Key_Alt);
        }

        if (binding.modifiers & Qt.MetaModifier) {
            keys.push(Qt.Key_Meta);
        }

        keys.push(binding.key);
        return keys;
    }

    // TODO
    // A binding whose every key has an icon draws as icons joined by a plus, and one missing any
    // draws as its written form instead. Bindings that would look identical are shown once
    function _buildGlyphs() {
        const icons = InputService.currentGamepadButtonIcons;
        let out = [];
        let seen = [];

        for (let i = 0; i < control.bindings.length && seen.length < 1; i++) {
            const binding = control.bindings[i];
            const sources = control._keysOf(binding).map(key => icons[key] || "");
            const drawable = sources.every(source => source !== "");
            const signature = drawable ? sources.join("+") : "text:" + binding.text;

            if (seen.indexOf(signature) !== -1) {
                continue;
            }

            seen.push(signature);

            if (seen.length > 1) {
                out.push({
                    "separator": "/"
                });
            }

            if (!drawable) {
                out.push({
                    "label": binding.text
                });
                continue;
            }

            for (let k = 0; k < sources.length; k++) {
                if (k > 0) {
                    out.push({
                        "separator": "+"
                    });
                }

                out.push({
                    "icon": sources[k]
                });
            }
        }

        return out;
    }

    leadingIcon: Component {
        Row {
            spacing: AppStyle.spacingXs

            Repeater {
                model: control._glyphs

                delegate: Row {
                    id: glyph

                    required property var modelData

                    spacing: AppStyle.spacingXs

                    Text {
                        visible: glyph.modelData.separator !== undefined
                        text: glyph.modelData.separator !== undefined ? glyph.modelData.separator : ""
                        height: AppStyle.iconSizeButton
                        verticalAlignment: Text.AlignVCenter
                        color: control._fg
                        font.pixelSize: AppStyle.fontSizeMedium
                        font.family: AppStyle.fontFamily
                    }

                    Text {
                        visible: glyph.modelData.label !== undefined
                        text: glyph.modelData.label !== undefined ? glyph.modelData.label : ""
                        height: AppStyle.iconSizeButton
                        verticalAlignment: Text.AlignVCenter
                        color: control._fg
                        font.pixelSize: AppStyle.fontSizeMedium
                        font.family: AppStyle.fontFamily
                    }

                    Image {
                        visible: glyph.modelData.icon !== undefined
                        source: glyph.modelData.icon !== undefined ? glyph.modelData.icon : ""
                        smooth: true
                        width: AppStyle.gamepadGlyphSize
                        height: AppStyle.gamepadGlyphSize
                        sourceSize.width: AppStyle.gamepadGlyphSize
                        sourceSize.height: AppStyle.gamepadGlyphSize
                        fillMode: Image.PreserveAspectFit
                    }
                }
            }
        }
    }

    onClicked: {
        if (control.bindings.length > 0) {
            EventEmitter.emitKeyEvent(control.bindings[0].key);
        }
    }
}
