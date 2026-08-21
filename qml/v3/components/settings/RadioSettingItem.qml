// TODO: NEEDS REVIEW
import QtQuick
import Firelight 1.0

// A catalog radio setting rendered as its options: one row each, carrying its own selection
// indicator and laid out among the rows around it rather than boxed inside one of them.
//
// `label` and `description` are accepted and unused. The only radio in the catalog shows in a menu,
// where the options are the whole row; a radio on a settings page needs a header row adding here
FLRadioGroup {
    id: root

    property var options: []
    property string label: ""
    property string description: ""
    property bool shown: true
    property bool resettable: false

    // TODO
    // The value as stored. Selecting an option writes currentValue, which would drop a binding the
    // caller had put on it, so the stored value arrives on its own property and drives that one
    property string storedValue: ""

    signal changed(string value)
    signal reset

    visible: shown

    model: root.options
    textRole: "label"
    valueRole: "value"

    Binding {
        target: root
        property: "currentValue"
        value: root.storedValue
        restoreMode: Binding.RestoreNone
    }

    onActivated: value => root.changed(value)
}
