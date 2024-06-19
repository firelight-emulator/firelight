import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


Flickable {
    id: root

    ColumnLayout {
        anchors.fill: parent
        Text {
            text: "Controllers"
            color: "white"
            font.pointSize: 24
            font.family: Constants.regularFontFamily
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }

        ListView {
            id: listThing
            Layout.fillWidth: true
            Layout.fillHeight: true
            interactive: false
            orientation: Qt.Horizontal

            moveDisplaced: Transition {
                NumberAnimation {
                    properties: "x,y"; duration: 100;
                    easing.type: Easing.InOutQuad
                }
            }
            spacing: 8
            model: DelegateModel {
                id: theModel
                model: controller_manager
                delegate: DropArea {
                    id: myself
                    required property var model
                    required property var index

                    width: 200
                    height: 200
                    // background: Rectangle {
                    //     color: "#25282C"
                    //     radius: 4
                    // }
                    //
                    onEntered: function (drag) {
                        if (drag.source === icon) {
                            return
                        }

                        console.log("[before] source: " + drag.source.DelegateModel.itemsIndex + ", index: " + icon.DelegateModel.itemsIndex)

                        theModel.items.move(drag.source.DelegateModel.itemsIndex, icon.DelegateModel.itemsIndex)
                        // console.log("Drag source parent is: " + drag.source.DelegateModel.itemsIndex)
                        // console.log("My index is: " + index)
                        console.log("[after] source: " + drag.source.DelegateModel.itemsIndex + ", index: " + icon.DelegateModel.itemsIndex)


                    }

                    Column {
                        id: icon

                        height: 60
                        width: 60

                        property Item dragParent: listThing
                        property var myIndex: parent.index

                        HoverHandler {
                            cursorShape: icon.state === "Dragging" ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                        }

                        DragHandler {
                            id: dragHandler
                        }
                        //
                        Drag.active: dragHandler.active
                        Drag.source: icon
                        Drag.hotSpot.x: 36
                        Drag.hotSpot.y: 36

                        states: [
                            State {
                                name: "NotDragging"
                                when: !dragHandler.active
                            },
                            State {
                                name: "Dragging"
                                when: dragHandler.active
                                ParentChange {
                                    target: icon
                                    parent: icon.dragParent
                                }

                                AnchorChanges {
                                    target: icon
                                    anchors {
                                        horizontalCenter: undefined
                                        verticalCenter: undefined
                                    }
                                }
                            }
                        ]


                        anchors {
                            horizontalCenter: parent.horizontalCenter
                            verticalCenter: parent.verticalCenter
                        }
                        Image {
                            source: "file:system/_img/Xbox.svg"
                            width: 100
                            height: 100
                            anchors.horizontalCenter: parent.horizontalCenter
                            fillMode: Image.PreserveAspectFit
                        }
                        Text {
                            text: model.model_name
                            color: "white"
                            font.pointSize: 12
                            font.family: Constants.regularFontFamily
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        Text {
                            text: model.wired ? "Wired" : "Wireless"
                            color: "white"
                            font.pointSize: 12
                            font.family: Constants.regularFontFamily
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }
            // model: controller_manager
            // delegate: Pane {
            //     required property var model
            //     // required property var index
            //
            //     width: 200
            //     height: 200
            //     background: Rectangle {
            //         color: "#25282C"
            //         radius: 4
            //     }
            //     contentItem: Column {
            //         Image {
            //             source: "file:system/_img/Xbox.svg"
            //             width: 100
            //             height: 100
            //             anchors.horizontalCenter: parent.horizontalCenter
            //             fillMode: Image.PreserveAspectFit
            //         }
            //         Text {
            //             // text: model.model_name
            //             text: model.model_name
            //             // text: "okay"
            //             color: "white"
            //             font.pointSize: 12
            //             font.family: Constants.regularFontFamily
            //             horizontalAlignment: Text.AlignHCenter
            //             verticalAlignment: Text.AlignVCenter
            //         }
            //         Text {
            //             // text: model.model_name
            //             text: model.wired ? "Wired" : "Wireless"
            //             // text: "okay"
            //             color: "white"
            //             font.pointSize: 12
            //             font.family: Constants.regularFontFamily
            //             horizontalAlignment: Text.AlignHCenter
            //             verticalAlignment: Text.AlignVCenter
            //         }
            //     }
            // }
        }
    }


    // RowLayout {
    //     id: rowLayout
    //     anchors.top: header.bottom
    //     anchors.left: parent.left
    //     anchors.right: parent.right
    //     height: 2 * parent.height / 3
    //     spacing: 8
    //
    //     Item {
    //         Layout.fillWidth: true
    //         Layout.fillHeight: true
    //         Layout.horizontalStretchFactor: 1
    //     }
    //
    //     Pane {
    //         Layout.preferredWidth: parent.width / 5
    //         Layout.preferredHeight: parent.width / 5
    //         Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
    //
    //         background: Rectangle {
    //             color: "transparent"
    //             radius: 4
    //             opacity: 0.3
    //             border.color: "white"
    //             border.width: 1
    //         }
    //
    //         Text {
    //             anchors.fill: parent
    //             text: "No controller connected"
    //             color: "white"
    //             opacity: 0.5
    //             font.pointSize: 11
    //             font.family: Constants.regularFontFamily
    //             horizontalAlignment: Text.AlignHCenter
    //             verticalAlignment: Text.AlignVCenter
    //         }
    //     }
    //
    //     Pane {
    //         Layout.preferredWidth: parent.width / 5
    //         Layout.preferredHeight: parent.width / 5
    //         Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
    //
    //         background: Rectangle {
    //             color: "transparent"
    //             radius: 4
    //             opacity: 0.3
    //             border.color: "white"
    //             border.width: 1
    //         }
    //
    //         Text {
    //             anchors.fill: parent
    //             text: "No controller connected"
    //             color: "white"
    //             opacity: 0.5
    //             font.pointSize: 11
    //             font.family: Constants.regularFontFamily
    //             horizontalAlignment: Text.AlignHCenter
    //             verticalAlignment: Text.AlignVCenter
    //         }
    //     }
    //
    //     Pane {
    //         Layout.preferredWidth: parent.width / 5
    //         Layout.preferredHeight: parent.width / 5
    //         Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
    //
    //         background: Rectangle {
    //             color: "transparent"
    //             radius: 4
    //             opacity: 0.3
    //             border.color: "white"
    //             border.width: 1
    //         }
    //
    //         Text {
    //             anchors.fill: parent
    //             text: "No controller connected"
    //             color: "white"
    //             opacity: 0.5
    //             font.pointSize: 11
    //             font.family: Constants.regularFontFamily
    //             horizontalAlignment: Text.AlignHCenter
    //             verticalAlignment: Text.AlignVCenter
    //         }
    //     }
    //
    //     Pane {
    //         Layout.preferredWidth: parent.width / 5
    //         Layout.preferredHeight: parent.width / 5
    //         Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
    //
    //         background: Rectangle {
    //             color: "transparent"
    //             radius: 4
    //             opacity: 0.3
    //             border.color: "white"
    //             border.width: 1
    //         }
    //
    //         Text {
    //             anchors.fill: parent
    //             text: "No controller connected"
    //             color: "white"
    //             opacity: 0.5
    //             font.pointSize: 11
    //             font.family: Constants.regularFontFamily
    //             horizontalAlignment: Text.AlignHCenter
    //             verticalAlignment: Text.AlignVCenter
    //         }
    //     }
    // }
}