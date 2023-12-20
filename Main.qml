import QtQuick 6.5
import QtQuick.Controls 6.5
import QtQuick.Window 2.2
import Firelight 1.0

Window {
    id: window
    width: 1280
    height: 720
    minimumWidth: 640
    minimumHeight: 480
    visible: true
    title: qsTr("Firelight")

    // Rectangle {
    //     id: navBar
    //
    //     anchors.left: parent.left
    //     anchors.top: parent.top
    //     anchors.bottom: parent.bottom
    //     width: parent.width / 5
    //
    //     color: "#af1414"
    //
    //     Text {
    //         text: "under construction"
    //         font.family: "Helvetica"; font.pointSize: 13; font.bold: true
    //         color: "#FFFFFF"
    //     }
    // }
    //
    // Rectangle {
    //     id: contentRect
    //
    //     anchors.left: navBar.right
    //     anchors.top: parent.top
    //     width: (parent.width / 5) * 2
    //     anchors.bottom: parent.bottom
    //
    //     color: "#1f1f1f"
    //     ListView {
    //         id: listThing
    //         // clip: true
    //         focus: true
    //
    //
    //         anchors.fill: parent
    //
    //
    //         model: RomModel {
    //         }
    //         boundsBehavior: Flickable.StopAtBounds
    //         delegate: gameListItem
    //         preferredHighlightBegin: height / 3
    //         preferredHighlightEnd: 2 * (height / 3) + currentItem.height
    //     }
    //
    //     Component {
    //         id: gameListItem
    //
    //         Rectangle {
    //             id: wrapper
    //
    //             width: ListView.view.width
    //             height: 40
    //
    //             color: ListView.isCurrentItem ? "#f0f0f0" : (mouseArea.containsMouse ? Qt.lighter(1.1) : "transparent")
    //
    //             Text {
    //                 anchors.left: parent.left
    //                 anchors.leftMargin: 20
    //                 anchors.verticalCenter: parent.verticalCenter
    //
    //                 color: wrapper.ListView.isCurrentItem ? "#1f1f1f" : "#f1f1f1"
    //
    //                 font.pixelSize: 16
    //
    //                 text: "testing " + model.filename
    //             }
    //
    //             MouseArea {
    //                 id: mouseArea
    //                 hoverEnabled: true
    //
    //                 anchors.fill: parent
    //                 cursorShape: Qt.PointingHandCursor
    //                 onClicked: {
    //                     listThing.currentIndex = index
    //                 }
    //             }
    //         }
    //     }
    // }
    EmulatorView {
        id: emulator
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: parent.top
    }


    // FLGame {
    //     id: game
    //     // width: 400
    //     // height: 400
    //     anchors.left: parent.left
    //     anchors.top: parent.top
    //     anchors.right: parent.right
    //     height: parent.height / 2
    // }

    Component.onCompleted: {
        emulator.init()
        beforeRendering.connect(window.update)
    }
}
