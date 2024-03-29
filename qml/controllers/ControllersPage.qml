import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FirelightStyle 1.0

Pane {
    id: root

    background: Rectangle {
        color: "transparent"
    }

    ListView {
        id: playerList
        width: 400
        height: 400
        anchors.centerIn: parent
        spacing: 6

        model: controller_manager
        delegate: RowLayout {
            implicitWidth: 300
            implicitHeight: 48

            Text {
                text: "P" + (model.player_index + 1)
                font.pointSize: 11
                font.family: Constants.semiboldFontFamily
                color: "#b3b3b3"
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.horizontalStretchFactor: 1
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
            }

            ContentPane {
                Text {
                    text: model.model_name
                    anchors.fill: parent
                    font.pointSize: 11
                    font.family: Constants.semiboldFontFamily
                    color: "white"
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter

                }
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.horizontalStretchFactor: 3
            }
        }


        // ContentPane {
        //     Text {
        //         text: controllerName
        //         anchors.fill: parent
        //         font.pointSize: 12
        //         font.family: Constants.strongFontFamily
        //         color: "white"
        //         horizontalAlignment: Text.AlignLeft
        //         verticalAlignment: Text.AlignVCenter
        //
        //     }
        //     width: parent.width
        //     height: 48
        // }
    }

    // ColumnLayout {
    //     id: players
    //     anchors.centerIn: parent
    //     spacing: 8
    //
    //     RowLayout {
    //         Layout.preferredWidth: 300
    //         Layout.preferredHeight: 48
    //
    //         Text {
    //             text: "P1"
    //             font.pointSize: 11
    //             font.family: Constants.strongFontFamily
    //             color: "#b3b3b3"
    //             Layout.fillWidth: true
    //             Layout.fillHeight: true
    //             Layout.horizontalStretchFactor: 1
    //             horizontalAlignment: Text.AlignLeft
    //             verticalAlignment: Text.AlignVCenter
    //         }
    //
    //         ContentPane {
    //             Text {
    //                 text: "Xbox One Controller for Windows"
    //                 anchors.fill: parent
    //                 font.pointSize: 12
    //                 font.family: Constants.strongFontFamily
    //                 color: "#b3b3b3"
    //                 horizontalAlignment: Text.AlignLeft
    //                 verticalAlignment: Text.AlignVCenter
    //
    //             }
    //             Layout.fillWidth: true
    //             Layout.fillHeight: true
    //             Layout.horizontalStretchFactor: 3
    //         }
    //     }

    // ContentPane {
    //     id: playerOne
    //     Layout.preferredWidth: 300
    //     Layout.preferredHeight: 48
    //
    //     Text {
    //         text: "Player 1"
    //         font.pointSize: 12
    //         font.family: Constants.strongFontFamily
    //         color: "#b3b3b3"
    //     }
    // }
    //
    // ContentPane {
    //     id: playerTwo
    //     Layout.preferredWidth: 300
    //     Layout.preferredHeight: 48
    //
    //     Text {
    //         text: "Player 2"
    //         font.pointSize: 12
    //         font.family: Constants.strongFontFamily
    //         color: "#b3b3b3"
    //     }
    // }
    //
    // ContentPane {
    //     id: playerThree
    //     Layout.preferredWidth: 300
    //     Layout.preferredHeight: 48
    //
    //     Text {
    //         text: "Player 3"
    //         font.pointSize: 12
    //         font.family: Constants.strongFontFamily
    //         color: "#b3b3b3"
    //     }
    // }
    //
    // ContentPane {
    //     id: playerFour
    //     Layout.preferredWidth: 300
    //     Layout.preferredHeight: 48
    //
    //     Text {
    //         text: "Player 4"
    //         font.pointSize: 12
    //         font.family: Constants.strongFontFamily
    //         color: "#b3b3b3"
    //     }
    // }

    // Thing {
    //     Layout.preferredWidth: parent.width
    //     Layout.minimumHeight: 48
    //     Layout.fillHeight: true
    //
    //     Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
    // }
    //
    // Thing {
    //     Layout.preferredWidth: parent.width
    //     Layout.minimumHeight: 48
    //     Layout.fillHeight: true
    //
    //     Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
    // }
    //
    // Thing {
    //     Layout.preferredWidth: parent.width
    //     Layout.minimumHeight: 48
    //     Layout.fillHeight: true
    //
    //     Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
    // }
    //
    // Thing {
    //     Layout.preferredWidth: parent.width
    //     Layout.minimumHeight: 48
    //     Layout.fillHeight: true
    //
    //     Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
    // }

    // }


    // Item {
    //     GridView {
    //         id: root
    //         width: parent.width
    //         height: parent.height
    //         cellWidth: parent.width
    //         cellHeight: parent.height / 4
    //
    //         displaced: Transition {
    //             NumberAnimation {
    //                 properties: "x,y"
    //                 easing.type: Easing.OutQuad
    //             }
    //         }
    //
    //         // model: controller_manager
    //         // delegate: Rectangle {
    //         //     width: 300
    //         //     height: 72
    //         //     color: "black"
    //         //     radius: 12
    //         //
    //         //     Text {
    //         //         anchors.centerIn: parent
    //         //         color: "white"
    //         //         text: playerIndex + ": " + controllerName
    //         //     }
    //         //
    //         //     DragHandler {
    //         //         id: dragHandler
    //         //     }
    //         // }
    //
    //         model: DelegateModel {
    //             id: visualModel
    //             model: controller_manager
    //
    //             component Thing: Rectangle {
    //                 id: icon
    //                 required property Item dragParent
    //                 required property string controllerName
    //                 required property int playerIndex
    //
    //                 property int visualIndex: 0
    //                 width: 300
    //                 height: 72
    //                 color: "black"
    //
    //                 anchors {
    //                     horizontalCenter: parent.horizontalCenter
    //                     verticalCenter: parent.verticalCenter
    //                 }
    //                 radius: 12
    //
    //                 Text {
    //                     anchors.centerIn: parent
    //                     color: "white"
    //                     text: controllerName
    //                 }
    //
    //                 DragHandler {
    //                     id: dragHandler
    //                 }
    //
    //                 HoverHandler {
    //                     cursorShape: icon.state === "Dragging" ? Qt.ClosedHandCursor : Qt.OpenHandCursor
    //                 }
    //
    //                 Drag.active: dragHandler.active
    //                 Drag.source: icon
    //                 Drag.hotSpot.x: 36
    //                 Drag.hotSpot.y: 36
    //
    //                 states: [
    //                     State {
    //                         name: "NotDragging"
    //                         when: !dragHandler.active
    //                     },
    //                     State {
    //                         name: "Dragging"
    //                         when: dragHandler.active
    //                         ParentChange {
    //                             target: icon
    //                             parent: icon.dragParent
    //                         }
    //
    //                         AnchorChanges {
    //                             target: icon
    //                             anchors {
    //                                 horizontalCenter: undefined
    //                                 verticalCenter: undefined
    //                             }
    //                         }
    //                     }
    //                 ]
    //
    //                 transitions: [
    //                     Transition {
    //                         from: "Dragging"
    //                         to: "NotDragging"
    //                         ScriptAction {
    //                             script: {
    //                                 var newOrder = {};
    //                                 for (var i = 0; i < visualModel.items.count; i++) {
    //                                     newOrder[i] = visualModel.items.get(i).model.playerIndex
    //                                     // console.log("playerIndex: " + visualModel.items.get(i).model.playerIndex)
    //                                 }
    //                                 // for (const key in visualModel.items) {
    //                                 //     if (myObject.hasOwnProperty(key)) {
    //                                 //         newOrder[visualModel.items[key].model.visualIndex] = visualModel.items[key].model.playerIndex;
    //                                 //     }
    //                                 // }
    //
    //                                 controller_manager.updateControllerOrder(newOrder)
    //                                 // for (var i = 0; i < visualModel.items.count; i++) {
    //                                 //     console.log("playerIndex: " + visualModel.items.get(i).model.playerIndex)
    //                                 // }
    //                             }
    //
    //                         }
    //                     }
    //                 ]
    //             }
    //
    //             delegate: DropArea {
    //                 id: delegateRoot
    //                 required property string controllerName
    //                 required property int playerIndex
    //
    //                 width: 300
    //                 height: 80
    //
    //                 onEntered: function (drag) {
    //                     visualModel.items.move(drag.source.visualIndex, icon2.visualIndex)
    //                     controller_manager.swap(drag.source.visualIndex, icon2.visualIndex)
    //                 }
    //
    //                 property int visualIndex: DelegateModel.itemsIndex
    //
    //                 Thing {
    //                     id: icon2
    //                     dragParent: root
    //                     visualIndex: delegateRoot.visualIndex
    //                     controllerName: delegateRoot.controllerName
    //                     playerIndex: delegateRoot.playerIndex
    //                 }
    //             }
    //         }
    //     }
    // }

    component Thing: Pane {
        id: con

        background: Rectangle {
            color: "white"
            opacity: conHover.hovered ? 0.2 : 0.1
            Behavior on opacity {
                NumberAnimation {
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
            }
            radius: 8
        }
        padding: 16

        Rectangle {
            id: conIcon
            width: con.height - (padding * 2)
            height: con.height - (padding * 2)
            anchors.verticalCenter: parent.verticalCenter
            radius: 8
            color: "white"
            opacity: 0.6
        }

        Text {
            anchors.left: conIcon.right
            anchors.leftMargin: 16
            anchors.top: parent.top
            text: "Player 1"
            font.pointSize: 12
            font.family: Constants.strongFontFamily
            color: "#b3b3b3"
        }

        HoverHandler {
            id: conHover
        }
    }
}