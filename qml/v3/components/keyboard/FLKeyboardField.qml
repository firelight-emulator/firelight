// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls
import Firelight 1.0

// TODO
// A text field a controller can fill in. Typing into it directly still works; a controller's
// select button hands it to the on-screen keyboard
FLTextField {
    id: control

    objectName: "FLKeyboardField|" + control.placeholderText

    // TODO
    // Masks the field and the keyboard alike
    property bool sensitive: false

    // TODO
    // What the keyboard's committing key reads
    property string acceptLabel: qsTr("OK")

    // TODO
    // Zero leaves the length unbounded
    property int limit: 0

    echoMode: control.sensitive ? TextInput.Password : TextInput.Normal
    maximumLength: control.limit > 0 ? control.limit : 32767

    FLFocus.showCursor: true

    function openKeyboard() {
        FLKeyboard.open({
            "text": control.text,
            "placeholderText": control.placeholderText,
            "maxLength": control.limit,
            "sensitive": control.sensitive,
            "acceptLabel": control.acceptLabel
        }, function (result) {
            if (result !== null) {
                control.text = result;
            }

            control.forceActiveFocus();
        });
    }

    // TODO
    // Runs ahead of the field's own handling, which would otherwise swallow the press and emit
    // accepted. A controller button carries no text where a typed Return carries one, so a
    // physical keyboard still submits the form rather than opening a keyboard on top of one
    Keys.priority: Keys.BeforeItem

    Keys.onPressed: event => {
        if (event.key !== Qt.Key_Enter && event.key !== Qt.Key_Return && event.key !== Qt.Key_Select) {
            return;
        }

        if (event.text.length === 0) {
            control.openKeyboard();
            event.accepted = true;
        }
    }
}
