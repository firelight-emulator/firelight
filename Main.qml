import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Window
import QtQuick.Layouts
import Firelight 1.0

Window {
    id: window
    width: 1280
    height: 720
    minimumWidth: 640
    minimumHeight: 480
    visible: true
    title: qsTr("Firelight")
    // color: "#1c1b1f"
    // color: "transparent"
    Material.theme: Material.Dark
    Material.accent: Material.color("#ff704f")

    Loader {
        id: emulatorLoader
        anchors.centerIn: parent
    }

    EmulatorView {
        id: emulatorView
        anchors.centerIn: parent
        width: 640
        height: 480

        Component.onCompleted: {
            // emulator.setWidth(width)
            // emulator.setHeight(height)
            this.initialize()
        }
    }

    // Component {
    //     id: emulator
    //
    // }

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
    // EmulatorView {
    //     id: emulator
    //     anchors.left: parent.left
    //     anchors.right: parent.right
    //     anchors.bottom: parent.bottom
    //     anchors.top: parent.top
    // }
    //
    // Rectangle {
    //     id: blackScreen
    //     anchors.fill: parent
    //     opacity: 0
    //     color: "black"
    // }
    //
    // SequentialAnimation {
    //     id: startGame
    //     NumberAnimation {
    //         target: blackScreen
    //         properties: "opacity"
    //         from: 0
    //         to: 1.0
    //         duration: 1000
    //         easing {
    //             type: Easing.OutQuad
    //         }
    //     }
    //     ScriptAction {
    //         script: content.replace(gamePage)
    //     }
    //     NumberAnimation {
    //         target: blackScreen
    //         properties: "opacity"
    //         from: 1.0
    //         to: 0
    //         duration: 400
    //         easing {
    //             type: Easing.OutQuad
    //         }
    //     }
    // }

    // TabBar {
    //     id: bar
    //     anchors.horizontalCenter: parent.horizontalCenter
    //     height: 50
    //
    //     currentIndex: 2
    //
    //     TabButton {
    //         width: 125
    //         height: bar.height
    //         enabled: false
    //         text: qsTr("Home")
    //     }
    //     TabButton {
    //         width: 125
    //         height: bar.height
    //         enabled: false
    //         text: qsTr("Explore")
    //     }
    //     TabButton {
    //         width: 125
    //         height: bar.height
    //         text: qsTr("Library")
    //         onClicked: content.replace(libraryPage)
    //     }
    //     TabButton {
    //         width: 125
    //         height: bar.height
    //         enabled: false
    //         text: qsTr("Controllers")
    //     }
    //     TabButton {
    //         width: 125
    //         height: bar.height
    //         text: qsTr("Settings")
    //         enabled: false
    //     }
    // }
    //
    // Component {
    //     id: libraryPage
    //     Item {
    //         id: wrapper
    //         ListView {
    //             id: libraryList
    //             focus: true
    //             clip: true
    //
    //             anchors.fill: parent
    //             model: libraryEntryModelShort
    //             boundsBehavior: Flickable.StopAtBounds
    //             delegate: gameListItem
    //             // preferredHighlightBegin: height / 3
    //             // preferredHighlightEnd: 2 * (height / 3) + currentItem.height
    //         }
    //
    //         Component {
    //             id: gameListItem
    //
    //             Rectangle {
    //                 id: wrapper
    //
    //                 width: ListView.view.width
    //                 height: 50
    //
    //                 color: mouseArea.containsMouse ? "#353438" : "transparent"
    //
    //                 MouseArea {
    //                     id: mouseArea
    //                     hoverEnabled: true
    //                     anchors.fill: parent
    //
    //                     cursorShape: Qt.PointingHandCursor
    //                     onClicked: {
    //                         console.log(model.entryId)
    //                         // startGame.start()
    //                     }
    //                 }
    //
    //                 Item {
    //                     id: picture
    //                     width: 100
    //                     height: parent.height
    //                 }
    //                 Text {
    //                     id: labels
    //                     x: picture.width + 20
    //                     anchors.verticalCenter: parent.verticalCenter
    //                     font.family: "Helvetica"
    //                     font.pointSize: 14
    //                     text: model.displayName
    //                     color: "white"
    //                 }
    //             }
    //         }
    //     }
    //
    // }

    // StackView {
    //     id: content
    //
    //     anchors.left: parent.left
    //     anchors.right: parent.right
    //     anchors.top: bar.bottom
    //     anchors.bottom: parent.bottom
    //
    //     anchors.topMargin: 20
    //
    //     initialItem: gamePage
    //
    //     popEnter: Transition {
    //     }
    //     popExit: Transition {
    //     }
    //     pushEnter: Transition {
    //     }
    //     pushExit: Transition {
    //     }


    // StackLayout {
    //     id: pageStack
    // }


    // FLGame {
    //     id: game
    //     // width: 400
    //     // height: 400
    //     anchors.left: parent.left
    //     anchors.top: parent.top
    //     anchors.right: parent.right
    //     height: parent.height / 2
    // }

    // Component.onCompleted: {
    //     emulator.init()
    //     beforeRendering.connect(window.update)
    // }
    // }
    //
    // Component {
    //     id: gamePage
    //
    //     Item {
    //         id: wrapper
    //         // EmulatorView {
    //         //     id: emulator
    //         //     anchors.left: parent.left
    //         //     anchors.right: parent.right
    //         //     anchors.bottom: parent.bottom
    //         //     anchors.top: parent.top
    //         // }
    //
    //         Component.onCompleted: {
    //             emulator.initialize()
    //         }
    //     }
    // }
}
