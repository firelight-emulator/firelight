// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// TODO
// One cap on the on-screen keyboard. Draws either a character or a glyph, and reports
// activation rather than acting, so the panel owns what a press means
Item {
    id: control

    objectName: "FLKeyboardKey|" + (control.label !== "" ? control.label : control.glyphName)

    // TODO
    // What the cap shows when it is a character key
    property string label: ""

    // TODO
    // A MaterialSymbols name, shown instead of the label
    property string glyphName: ""

    property bool glyphFilled: true

    // TODO
    // What the guide bar calls this key. Constant for the life of the key, since the bar only
    // refreshes when focus moves
    property string actionLabel: qsTr("Enter")

    // TODO
    // A refusing key stays a navigation target. Disabling it outright would drop it from the
    // collector and open a hole the cursor falls through
    property bool canInteract: true

    // TODO
    // Accent-filled, for the key that commits
    property bool primary: false

    // TODO
    // Lit to show a mode the key controls is on
    property bool active: false

    property bool showIndicator: false
    property bool indicatorActive: false

    property bool allowAutoRepeat: false

    signal activated

    TapHandler {
        id: tapHandler

        onTapped: {
            if (control.canInteract) {
                control.activated();
            }
        }
    }

    focusPolicy: Qt.StrongFocus
    opacity: control.canInteract ? 1 : 0.4

    implicitHeight: 70
    implicitWidth: 128

    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.horizontalStretchFactor: 1
    Layout.verticalStretchFactor: 1

    FLFocus.showCursor: true
    FLFocus.focusSound: SoundEffects.menuNavigate
    FLFocus.spacing: 2
    FLFocus.fill: "black"

    FLFocus.actions: [
        FLAction {
            label: control.actionLabel
            keys: [Qt.Key_Enter, Qt.Key_Return, Qt.Key_Select]
            sound: control.canInteract ? SoundEffects.menuItemFocus : SoundEffects.cursorBump
            triggerOnAutoRepeat: control.allowAutoRepeat
            onTriggered: {
                if (!control.canInteract) {
                    return;
                }

                control.activated();
            }
        }
    ]

    readonly property bool cursorFocused: FocusCursor.isOn(control)

    readonly property color _fill: {
        if (control.primary) {
            return Theme.switch2Color;
        }

        if (control.active) {
            return Theme.surfaceHover;
        }

        return Theme.surfaceElevated;
    }

    readonly property color _fg: control.primary ? Theme.onAccent : Theme.textPrimary

    HoverHandler {
        id: hover
        cursorShape: Qt.PointingHandCursor
    }

    Rectangle {
        anchors.fill: parent
        color: control._fill
        border.width: control.active ? 1 : 0
        border.color: Theme.borderStrong

        Rectangle {
            anchors.fill: parent
            color: Theme.textPrimary
            opacity: {
                if (!control.canInteract) {
                    return 0;
                }

                return tapHandler.pressed ? 0.16 : (hover.hovered || control.cursorFocused) ? 0.08 : 0;
            }

            Behavior on opacity {
                NumberAnimation {
                    duration: AppStyle.durationVeryFast
                }
            }
        }
    }

    Rectangle {
        width: 12
        height: 12
        anchors.left: parent.left
        anchors.leftMargin: 6
        anchors.top: parent.top
        anchors.topMargin: 6
        radius: 6
        color: control.indicatorActive ? Theme.switch2Color : Theme.surfaceHover
        visible: control.showIndicator
    }

    Item {
        anchors.fill: parent

        Text {
            anchors.centerIn: parent
            visible: control.glyphName === ""
            text: control.label
            color: control._fg
            font.family: AppStyle.fontFamily
            font.pixelSize: AppStyle.fontSizeLarge
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        Icon {
            anchors.centerIn: parent
            visible: control.glyphName !== ""
            name: control.glyphName
            size: AppStyle.iconSizeLg
            color: control._fg
            filled: control.glyphFilled
        }
    }
}
