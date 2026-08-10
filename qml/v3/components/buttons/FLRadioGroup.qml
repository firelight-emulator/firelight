import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

// TODO
// A vertical group of radio options. `model` is a list of strings, or of
// objects with `label` and optional `value`. Up/Down move the cursor between
// options without changing the selection; A/Space/Return or a click selects the
// focused one and emits `activated`. Focus entering the group lands on the
// selected option
//
// `currentValue` is an input the owner drives, so selection stays wherever the
// real state lives and a bound value keeps tracking it:
//
//   FLRadioGroup {
//       model: [{ label: "Name", value: "displayName" }, { label: "Year", value: "year" }]
//       currentValue: view.sortRole
//       onActivated: value => view.sortRole = value
//   }
//
// With nothing external to bind to, the group can drive itself:
//
//   FLRadioGroup { model: [...]; onActivated: value => currentValue = value }
FocusScope {
    id: root

    property var model: []
    property var currentValue: undefined

    Layout.fillWidth: true

    // TODO
    // Which keys of a model object hold the row's text and its value, named
    // after ComboBox's equivalents. A model of plain strings ignores both
    property string textRole: "text"
    property string valueRole: "value"

    // TODO
    // Which option `currentValue` names, or -1 when it names none
    readonly property int currentIndex: {
        for (let i = 0; i < root.model.length; i++) {
            if (root.valueFor(root.model[i]) === root.currentValue) {
                return i;
            }
        }

        return -1;
    }

    // TODO
    // The selected option's row, or null while nothing is selected
    readonly property Item currentItem: currentIndex >= 0 && currentIndex < repeater.count ? repeater.itemAt(currentIndex) : null

    signal activated(var value)

    implicitWidth: column.implicitWidth
    implicitHeight: column.implicitHeight

    // TODO
    // Advertises the group as a focus target so containers that walk their
    // children (FLColumnLayout) navigate into it rather than skipping past
    focusPolicy: Qt.StrongFocus

    function labelFor(item): string {
        if (item === undefined || item === null) {
            return "";
        }

        if (typeof item === "object" && item[root.textRole] !== undefined) {
            return item[root.textRole];
        }

        return String(item);
    }

    // TODO
    // An option's value: its valueRole, else its text, else the item itself
    function valueFor(item): var {
        if (item === undefined || item === null) {
            return undefined;
        }

        if (typeof item === "object") {
            if (item[root.valueRole] !== undefined) {
                return item[root.valueRole];
            }

            return item[root.textRole];
        }

        return item;
    }

    // TODO
    // Commits an option by announcing it; the owner of `currentValue` decides
    // what to do. Re-selecting the current option still emits, so a caller can
    // treat that as a toggle
    function select(index: int) {
        if (index < 0 || index >= root.model.length) {
            return;
        }

        SoundEffects.radioSelect.play();
        root.currentValue = root.valueFor(root.model[index]);
        root.activated(root.currentValue);
    }

    // TODO
    // Moves the cursor without touching the selection
    function focusOption(index: int) {
        if (index < 0 || index >= repeater.count) {
            return;
        }

        repeater.itemAt(index).forceActiveFocus();
    }

    function focusCurrentIndex() {
        if (root.currentIndex >= 0 && root.currentIndex < repeater.count) {
            focusOption(root.currentIndex);
        } else if (repeater.count > 0) {
            focusOption(0);
        }
    }

    function enterFrom(step: int) {
        focusOption(step < 0 ? root.model.length - 1 : 0);
    }

    FLColumnLayout {
        id: column
        anchors.fill: parent
        spacing: AppStyle.spacingXs

        Repeater {
            id: repeater
            model: root.model

            delegate: FLMenuItem {
                id: option

                required property var modelData
                required property int index

                readonly property bool selected: root.currentIndex === index

                objectName: "FLRadioOption|" + root.labelFor(modelData)
                label: root.labelFor(modelData)
                checked: selected

                // TODO
                // Up/Down are the surrounding FLColumnLayout's; Space arrives as
                // AbstractButton's click, and the gamepad A button reaches this
                // through the window's activation fallback
                onClicked: root.select(option.index)

                FLRadioIndicator {
                    selected: option.selected
                }
            }
        }
    }
}
