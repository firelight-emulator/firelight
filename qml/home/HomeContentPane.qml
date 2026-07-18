import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts 1.0
import QtQuick.Effects
import Firelight 1.0

StackView {
    id: stackview
    objectName: "Home Content Stack View"
    anchors.fill: parent

    property alias currentPageName: stackview.topLevelName

    function goTo(page) {
        stackview.replace(null, page);
    }

    property string topLevelName: ""

    onCurrentItemChanged: {
        if (currentItem) {
            let top = stackview.find(function (item, index) {
                return item.topLevel === true;
            });

            stackview.topLevelName = top ? top.topLevelName : "";
        }
    }

    // initialItem: libraryPage

    pushEnter: Transition {}
    pushExit: Transition {}
    popEnter: Transition {}
    popExit: Transition {}
    replaceEnter: Transition {}
    replaceExit: Transition {}
}
