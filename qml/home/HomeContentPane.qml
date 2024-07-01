import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts 1.0
import QtQuick.Effects
import Firelight 1.0


StackView {
    objectName: "Home Content Stack View"
    id: stackview
    anchors.fill: parent

    property alias currentPageName: stackview.topLevelName

    function goTo(page) {
        stackview.replace(null, page)
    }

    property string topLevelName: ""

    onCurrentItemChanged: {
        if (currentItem) {
            let top = stackview.find(function (item, index) {
                return item.topLevel === true
            })

            stackview.topLevelName = top ? top.topLevelName : ""
        }
    }

    // Pane {
    //     width: 48
    //     height: 48
    //
    //     z: 2
    //
    //     background: Item {
    //     }
    //
    //     Button {
    //         id: melol
    //         anchors.left: parent.left
    //         anchors.verticalCenter: parent.verticalCenter
    //         // horizontalPadding: 12
    //         // verticalPadding: 8
    //
    //         enabled: stackview.depth > 1
    //
    //         hoverEnabled: false
    //
    //         HoverHandler {
    //             id: myHover
    //             cursorShape: melol.enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
    //         }
    //
    //         background: Rectangle {
    //             color: enabled ? myHover.hovered ? "#4e535b" : "#3e434b" : "#3e434b"
    //             radius: height / 2
    //         }
    //
    //         contentItem: Text {
    //             text: "\ue5c4"
    //             color: enabled ? "white" : "#7d848c"
    //             font.pointSize: 11
    //             font.family: Constants.symbolFontFamily
    //             horizontalAlignment: Text.AlignHCenter
    //             verticalAlignment: Text.AlignVCenter
    //         }
    //
    //         onClicked: {
    //             stackview.pop()
    //         }
    //     }
    // }

    // initialItem: libraryPage

    pushEnter: Transition {
    }
    pushExit: Transition {
    }
    popEnter: Transition {
    }
    popExit: Transition {
    }
    replaceEnter: Transition {
    }
    replaceExit: Transition {
    }
}