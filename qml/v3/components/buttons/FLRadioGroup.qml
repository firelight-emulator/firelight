import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

//   FLRadioGroup {
//       model: [{ label: "Name", value: "displayName" }, { label: "Year", value: "year" }]
//       currentValue: view.sortRole
//       onActivated: value => view.sortRole = value
//   }
//
//   FLRadioGroup { model: [...]; onActivated: value => currentValue = value }
FocusScope {
    id: root

    property var model: []
    property var currentValue: undefined

    Layout.fillWidth: true

    property string textRole: "text"
    property string valueRole: "value"

    readonly property int currentIndex: {
        for (let i = 0; i < root.model.length; i++) {
            if (root.valueFor(root.model[i]) === root.currentValue) {
                return i;
            }
        }

        return -1;
    }

    readonly property Item currentItem: currentIndex >= 0 && currentIndex < repeater.count ? repeater.itemAt(currentIndex) : null

    property int surface: FLMenuItem.Surface.InMenu
    property bool subItem: false
    property bool isFirstInSection: false
    property bool isLastInSection: false
    property bool showDivider: false

    readonly property bool _onPage: root.surface === FLMenuItem.Surface.Page

    signal activated(var value)

    implicitWidth: column.implicitWidth
    implicitHeight: column.implicitHeight

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
    function select(index: int) {
        if (index < 0 || index >= root.model.length) {
            return;
        }

        const chosen = root.valueFor(root.model[index]);

        SoundEffects.radioSelect.play();
        root.activated(chosen);
    }
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
        spacing: 0

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

                surface: root.surface
                subItem: root.subItem
                isFirstInSection: root.isFirstInSection && option.index === 0
                isLastInSection: root.isLastInSection && option.index === repeater.count - 1

                showDivider: option.index < repeater.count - 1 ? root._onPage : root.showDivider

                onClicked: root.select(option.index)

                FLRadioIndicator {
                    selected: option.selected
                }
            }
        }
    }
}
