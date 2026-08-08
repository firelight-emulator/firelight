import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FLMenuItem {
    id: control

    required property var model
    property string textRole: "text"
    property string valueRole: "value"

    property var currentValues: []
    checked: currentValues.length > 0

    // TODO
    // How long the cursor rests on the row before the submenu opens, so sweeping
    // past on the way to another row does not open it
    property int hoverDelay: 500

    property bool registered: false

    // TODO
    // Qt dismisses an open menu when an unrelated one opens, and only treats
    // this one as belonging to `menu` once it has been added to it. The menu's
    // own contentItem is replaced, so this changes nothing on screen
    function registerIn(menu) {
        if (control.registered) {
            return;
        }

        control.registered = true;
        menu.addMenu(submenu);

        // TODO
        // addMenu leaves a MenuItem of its own beside the rows in the menu's
        // content item. Nothing draws it, so the cursor is kept off it
        const generated = menu.itemAt(menu.count - 1);

        if (generated !== null) {
            generated.focusPolicy = Qt.NoFocus;
        }
    }

    function openSubmenu() {
        if (!submenu.opened) {
            submenu.popup(control, -80, -80);
        }
    }

    function closeSubmenu() {
        hoverTimer.stop();

        if (submenu.opened) {
            SoundEffects.back.play();
            submenu.close();
        }
    }

    hoverEnabled: true

    onHoveredChanged: hovered ? hoverTimer.restart() : hoverTimer.stop()
    onClicked: control.openSubmenu()

    Keys.onPressed: function(event) {
        if (event.key !== Qt.Key_Right) {
            event.accepted = false;
            return;
        }

        event.accepted = true;
        control.openSubmenu();
    }

    Text {
        id: numSelectedText
        text: control.currentValues.length > 0 ? control.currentValues.length + " selected" : "None selected"
        color: Theme.textMuted
        font.family: AppStyle.fontFamily
        font.pixelSize: AppStyle.fontSizeSmall
        horizontalAlignment: Text.AlignRight
    }

    Timer {
        id: hoverTimer
        interval: control.hoverDelay
        onTriggered: control.openSubmenu()
    }

    FLMenu {
        id: submenu
        //
        // x: control.width + AppStyle.spacingXs
        // y: -200

        onOpened: {
            submenu.focusFirstChild();
        }

        FLButton {
            id: clearButton
            text: "Clear selections"
            Layout.fillWidth: true
            Layout.leftMargin: AppStyle.spacingSm
            Layout.rightMargin: AppStyle.spacingSm
            Layout.topMargin: AppStyle.spacingSm
            Layout.bottomMargin: AppStyle.spacingSm
            canInteract: control.currentValues.length > 0

            FLFocus.focusSound: SoundEffects.menuNavigate
            FLFocus.actions: [
                FLAction {
                    keys: [Qt.Key_Select, Qt.Key_Return, Qt.Key_Enter, Qt.Key_Space]
                    label: qsTr("Select")
                    sound: clearButton.canInteract ? SoundEffects.openPopup : SoundEffects.cursorBump
                    onTriggered: {
                        if (!clearButton.canInteract) {
                            return
                        }

                        clearButton.click()
                    }
                }
            ]

            Keys.onPressed: function(event) {
                if (event.key !== Qt.Key_Left) {
                    event.accepted = false;
                    return;
                }

                event.accepted = true;
                control.closeSubmenu();
            }

            onClicked: function(event) {
                control.currentValues = []
            }

            // variant: "subtle"
        }

        // FLMenuSeparator {}

        Repeater {
            model: control.model

            delegate: FLToggleMenuItem {
                id: option

                required property var modelData
                required property int index

                Keys.onPressed: function(event) {
                    if (event.key !== Qt.Key_Left) {
                        event.accepted = false;
                        return;
                    }

                    event.accepted = true;
                    control.closeSubmenu();
                }

                label: modelData[control.textRole]

                checked: control.currentValues.indexOf(modelData[control.valueRole]) !== -1

                onSelected: function(selected) {
                    if (selected) {
                        control.currentValues.push(modelData[control.valueRole])
                        control.currentValuesChanged()
                    } else {
                        control.currentValues.splice(control.currentValues.indexOf(modelData[control.valueRole]), 1)
                        control.currentValuesChanged()
                    }
                }
            }
        }
    }
}
