import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Firelight 1.0

FLTwoColumnPage {
    id: root
    objectName: "CollectionOrderPage"

    headerText: qsTr("Create Collection")
    menuOnRight: true

    property bool routeActive: true

    property string discardMessage: qsTr("Return without saving changes?")
    property string discardConfirmText: qsTr("OK")
    property string discardCancelText: qsTr("Continue reordering")

    // Which step of the workflow is showing
    property int stepIndex: 0
    readonly property int stepCount: 1
    readonly property bool onLastStep: root.stepIndex === root.stepCount - 1

    model: [
        {
            "type": "action",
            "key": "resume",
            "label": qsTr("Add Games")
        },
        {
            "type": "action",
            "key": "restart",
            "label": qsTr("Do Something Else")
        }
    ]

    footer: FLButton {
        id: saveButton
        Layout.alignment: Qt.AlignHCenter
        Layout.margins: AppStyle.spacingXl
        Layout.preferredWidth: root.menuWidth * 0.8
        text: qsTr("Save")
        onClicked: {
            if (!canInteract) {
                return;
            }

            root.save();
        }

        // canInteract: root._dirty
    }

    Component.onCompleted: {
        root.showPage(addGamesPage);
        root.focusContent();
        root.loaded();
    }

    Component {
        id: addGamesPage

        GameView {}
    }


}