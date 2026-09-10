// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts
import Firelight 1.0

// TODO
// Full-screen text entry: what has been typed sits above, the key grid below. Opened through
// the FLKeyboard singleton rather than directly
FLPanel {
    id: control

    objectName: "FLKeyboardOverlay"

    parent: Overlay.overlay
    modal: true
    focus: true
    closePolicy: Popup.NoAutoClose
    padding: 0

    blurOnDim: true

    property real bottomReservedHeight: 0

    // TODO
    // Popup understands anchors.centerIn and nothing else, so filling is done by hand
    x: 0
    y: 0
    width: control.parent ? control.parent.width : 0
    height: control.parent ? control.parent.height : 0

    // TODO
    // FLDimmer already holds the dim; without this the style lays its own scrim over it
    Overlay.modal: Item {}

    openSound: SoundEffects.openPopup

    property string placeholderText: ""
    property int maxLength: 0
    property bool sensitive: false
    property bool multiline: false
    property string acceptLabel: qsTr("OK")

    // TODO
    // How long a freshly typed character stays legible before it becomes a dot. Zero masks it
    // straight away
    property int revealMs: 0

    // TODO
    // Only meaningful while sensitive, and always starts off
    property bool revealed: false

    readonly property string text: field.text

    signal accepted(string text)
    signal rejected

    property bool _accepting: false

    readonly property real _maxContentWidth: 1300
    readonly property real _maxContentHeight: 340

    readonly property real _keyboardYAnimationOffset: 12

    enter: Transition {
        SequentialAnimation {
            PauseAnimation {
                duration: AppStyle.durationDimmer
            }
            ParallelAnimation {
                NumberAnimation {
                    target: panelSurfaceTranslation
                    property: "y"
                    from: control._keyboardYAnimationOffset
                    to: 0
                    duration: AppStyle.durationSlow
                    easing.type: AppStyle.easingStandard
                }
                NumberAnimation {
                    target: body
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: AppStyle.durationSlow
                    easing.type: AppStyle.easingStandard
                }
            }
        }
    }

    exit: Transition {
        NumberAnimation {
            target: panelSurfaceTranslation
            property: "y"
            from: 0
            to: control._keyboardYAnimationOffset
            duration: AppStyle.durationSlow
            easing.type: AppStyle.easingStandard
        }
        NumberAnimation {
            target: body
            property: "opacity"
            from: 1
            to: 0
            duration: AppStyle.durationSlow
            easing.type: AppStyle.easingStandard
        }

    }

    function reset(options) {
        const o = options || {};

        control.placeholderText = o.placeholderText !== undefined ? o.placeholderText : "";
        control.maxLength = o.maxLength !== undefined ? o.maxLength : 0;
        control.sensitive = o.sensitive === true;
        control.multiline = o.multiline === true;
        control.acceptLabel = o.acceptLabel !== undefined ? o.acceptLabel : qsTr("OK");
        control.revealMs = o.revealMs !== undefined ? o.revealMs : 0;
        control.revealed = false;
        control._accepting = false;

        field.text = o.text !== undefined ? o.text : "";
        field.cursorPosition = field.text.length;
    }

    function insert(characters: string) {
        const selected = field.selectedText.length;

        if (control.maxLength > 0 && field.text.length - selected + characters.length > control.maxLength) {
            SoundEffects.cursorBump.play();
            return;
        }

        // TODO
        // insert() drops text at a position rather than replacing what is selected, so the
        // selection is cleared first
        if (selected > 0) {
            const at = field.selectionStart;
            field.remove(field.selectionStart, field.selectionEnd);
            field.cursorPosition = at;
        }

        field.insert(field.cursorPosition, characters);
    }

    function backspace() {
        if (field.selectedText.length > 0) {
            field.remove(field.selectionStart, field.selectionEnd);
            return;
        }

        if (field.cursorPosition > 0) {
            field.remove(field.cursorPosition - 1, field.cursorPosition);
        }
    }

    function moveCaret(delta: int) {
        field.cursorPosition = Math.max(0, Math.min(field.text.length, field.cursorPosition + delta));
    }

    function commit() {
        control._accepting = true;
        control.accepted(field.text);
        control.close();
    }

    onClosed: {
        const accepting = control._accepting;

        control._accepting = false;

        // TODO
        // A password must not sit in the scene graph waiting for the next request
        field.text = "";
        control.revealed = false;

        if (!accepting) {
            control.rejected();
        }
    }

    onOpened: Qt.callLater(() => panel.focusKeyboard())

    background: Item {}

    contentItem: FocusScope {
        id: surface
        // TODO
        // The popup item above this is a focus scope of its own and keeps what it is given, so the
        // surface has to claim it for anything inside to be reached
        focus: true

        // TODO
        // Actions live one level in: FLPopupBehavior assigns the surface's action list wholesale
        // when it attaches, which would drop anything declared on the surface itself
        Item {
            id: body
            opacity: 0

            anchors.fill: parent

            FLFocus.actions: [
                FLAction {
                    label: qsTr("Cancel")
                    keys: [Qt.Key_Menu]
                    sound: SoundEffects.back
                    onTriggered: control.close()
                },
                FLAction {
                    label: qsTr("Move cursor")
                    keys: [Qt.Key_Minus]
                    sound: SoundEffects.menuNavigate
                    onTriggered: control.moveCaret(-1)
                },
                FLAction {
                    label: qsTr("Move cursor")
                    keys: [Qt.Key_Equal]
                    sound: SoundEffects.menuNavigate
                    onTriggered: control.moveCaret(1)
                }
            ]

            Keys.priority: Keys.BeforeItem

            // TODO
            // Physical typing reaches the grid rather than the field, so printable characters are
            // routed to the same entry point the on-screen keys use
            Keys.onPressed: event => {
                console.log("key pressed: " + event.key + " text: " + event.text + " modifiers: " + event.modifiers);
                if (event.accepted) {
                    return;
                }

                const chorded = event.modifiers & (Qt.ControlModifier | Qt.AltModifier | Qt.MetaModifier);

                if (chorded) {
                    return;
                }

                if (event.key === Qt.Key_Backspace) {
                    control.backspace();
                    event.accepted = true;
                    return;
                }

                if (event.key === Qt.Key_Delete) {
                    if (field.cursorPosition < field.text.length) {
                        field.remove(field.cursorPosition, field.cursorPosition + 1);
                    }

                    event.accepted = true;
                    return;
                }

                if (event.key === Qt.Key_Home || event.key === Qt.Key_End) {
                    field.cursorPosition = event.key === Qt.Key_Home ? 0 : field.text.length;
                    event.accepted = true;
                    return;
                }

                // TODO
                // A gamepad button arrives as a key event with no text at all, so nothing below
                // can fire for one. Control characters carry a text too and are not typed
                const typed = event.text;

                if (typed.length !== 1 || typed.charCodeAt(0) < 0x20 || typed.charCodeAt(0) === 0x7f) {
                    return;
                }

                control.insert(typed);
                event.accepted = true;
            }

            // TODO
            // The window installs a tap handler that pulls focus back to its content item, so the
            // keyboard must leave no gap for a click to fall through
            MouseArea {
                anchors.fill: parent
                anchors.topMargin: AppStyle.titleBarHeight
                acceptedButtons: Qt.AllButtons
            }

            Item {
                id: previewArea

                anchors.top: parent.top
                anchors.bottom: panelSurface.top
                anchors.topMargin: Math.min(AppStyle.spacingLg, (parent.height - 300) / 2)
                anchors.bottomMargin: Math.min(AppStyle.spacingLg, (parent.height - 300) / 2)
                anchors.right: parent.right
                anchors.left: parent.left

                Rectangle {
                    id: previewBox

                    anchors.centerIn: parent

                    height: Math.min(260, parent.height)
                    width: Math.min(control._maxContentWidth, parent.width - AppStyle.spacingLg * 2)
                    radius: AppStyle.radiusMd
                    color: "transparent"
                    border.width: 2
                    border.color: Theme.textPrimary

                    TextField {
                        id: field

                        objectName: "FLKeyboardOverlay|field"
                        anchors.fill: parent
                        anchors.margins: AppStyle.spacingLg

                        // TODO
                        // The grid holds the cursor, so this is pruned from directional navigation
                        // while still being clickable for selection and clipboard
                        focusPolicy: Qt.ClickFocus
                        FLFocus.mode: FLFocus.Skip

                        placeholderText: control.placeholderText
                        maximumLength: control.maxLength > 0 ? control.maxLength : 32767
                        echoMode: control.sensitive && !control.revealed ? TextInput.Password : TextInput.Normal
                        passwordMaskDelay: control.revealMs
                        wrapMode: control.multiline ? TextInput.Wrap : TextInput.NoWrap
                        selectByMouse: true

                        // TODO
                        // The caret spends most of its life without active focus, since the cursor
                        // is out on the grid, and both of these are otherwise reset when focus moves
                        persistentSelection: true
                        cursorVisible: true

                        verticalAlignment: TextInput.AlignTop

                        color: Theme.textPrimary
                        placeholderTextColor: Theme.textMuted
                        font.family: AppStyle.fontFamily
                        font.pixelSize: AppStyle.keyboardPreviewFontSize

                        background: Item {}
                    }
                }

                Text {
                    anchors.right: previewBox.right
                    anchors.top: previewBox.bottom
                    anchors.topMargin: AppStyle.spacingXs
                    visible: control.maxLength > 0
                    text: field.text.length + "/" + control.maxLength
                    color: Theme.textMuted
                    font.family: AppStyle.fontFamily
                    font.pixelSize: AppStyle.fontSizeMedium
                }
            }

            Pane {
                id: panelSurface

                transform: Translate {
                    id: panelSurfaceTranslation
                    y: control._keyboardYAnimationOffset
                }

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: parent.height * 0.6
                topPadding: AppStyle.spacingLg
                bottomPadding: control.bottomReservedHeight + AppStyle.spacingLg
                leftPadding: 0
                rightPadding: 0

                background: Rectangle {
                    topRightRadius: AppStyle.radiusLg * 2
                    topLeftRadius: AppStyle.radiusLg * 2

                    color: Theme.surface
                }

                FocusScope {
                    readonly property int _maxWidth: control._maxContentWidth
                    readonly property int _maxHeight: control._maxContentHeight

                    readonly property real _integerScale: {
                        const widthScale = Math.min(panelSurface.contentItem.width, _maxWidth) / _maxWidth;
                        const heightScale = Math.min(panelSurface.contentItem.height, _maxHeight) / _maxHeight;

                        return Math.min(widthScale, heightScale);
                    }

                    anchors.centerIn: parent
                    width: _maxWidth * _integerScale
                    height: _maxHeight * _integerScale

                    ColumnLayout {
                        anchors.fill: parent

                        spacing: AppStyle.spacingLg

                        FLKeyboardKey {
                            label: control.revealed ? qsTr("Hide text") : qsTr("Show text")
                            actionLabel: qsTr("Toggle")
                            active: control.revealed
                            visible: control.sensitive
                            Layout.fillWidth: true
                            Layout.preferredHeight: AppStyle.keyboardKeyHeight

                            onActivated: control.revealed = !control.revealed
                        }

                        // Rectangle {
                        //     color: Theme.surfaceElevated
                        //     radius: height / 2
                        //     Layout.fillWidth: true
                        //     Layout.preferredHeight: 48
                        //     Layout.alignment: Qt.AlignHCenter
                        // }

                        FLKeyboardPanel {
                            id: panel

                            objectName: "FLKeyboardOverlay|panel"
                            Layout.fillHeight: true
                            Layout.fillWidth: true

                            multiline: control.multiline
                            acceptLabel: control.acceptLabel

                            onCharacterEntered: character => control.insert(character)
                            onBackspacePressed: control.backspace()
                            onReturnPressed: control.insert("\n")
                            onAcceptPressed: control.commit()
                        }
                    }
                }


            }
        }
    }
}
