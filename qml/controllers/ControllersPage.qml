import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


Flickable {
    id: root

    RowLayout {
        anchors.fill: parent
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.horizontalStretchFactor: 1
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.horizontalStretchFactor: 4
            Layout.minimumWidth: 700
            Layout.maximumWidth: 1200
            Layout.preferredWidth: parent.width * 3 / 4
            Layout.fillHeight: true

            // Text {
            //     text: "Controllers"
            //     color: "white"
            //     font.pointSize: 24
            //     font.family: Constants.regularFontFamily
            //     horizontalAlignment: Text.AlignLeft
            //     verticalAlignment: Text.AlignVCenter
            // }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 60
            }

            ListView {
                id: listThing
                Layout.preferredWidth: 836
                Layout.alignment: Qt.AlignHCenter
                Layout.fillHeight: true
                interactive: false
                orientation: Qt.Horizontal

                moveDisplaced: Transition {
                    NumberAnimation {
                        properties: "x,y"; duration: 100;
                        easing.type: Easing.InOutQuad
                    }
                }

                // populate: Transition {
                //     ParallelAnimation {
                //         NumberAnimation {
                //             property: "y"
                //             duration: 240
                //             easing.type: Easing.InOutQuad
                //             from: 20
                //             to: 0
                //         }
                //         NumberAnimation {
                //             property: "opacity"
                //             duration: 240
                //             easing.type: Easing.InOutQuad
                //             from: 0
                //             to: 1
                //         }
                //     }
                // }
                spacing: 12
                model: DelegateModel {
                    id: theModel
                    model: controller_model
                    delegate: DropArea {
                        id: myself
                        required property var model
                        required property var index

                        keys: model.model_name !== "Default" ? ["good"] : ["nope"]

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

                            theModel.items.move(drag.source.DelegateModel.itemsIndex, icon.DelegateModel.itemsIndex)
                        }

                        Item {
                            id: thingy
                            visible: model.model_name === "Default"

                            height: 220
                            width: 200

                            Rectangle {
                                anchors.fill: parent
                                border.color: "#515151"
                                color: "transparent"
                                radius: 6

                                Text {
                                    anchors.centerIn: parent
                                    text: "No controller connected"
                                    color: "#757575"
                                    font.pointSize: 11
                                    font.family: Constants.regularFontFamily
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }

                            anchors {
                                horizontalCenter: parent.horizontalCenter
                                verticalCenter: parent.verticalCenter
                            }
                        }

                        Item {
                            id: icon
                            visible: model.model_name !== "Default"

                            height: 220
                            width: 200

                            property Item dragParent: listThing
                            property var myIndex: parent.index

                            HoverHandler {
                                cursorShape: icon.state === "Dragging" ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                            }

                            DragHandler {
                                id: dragHandler

                                onActiveChanged: function () {
                                    if (!dragHandler.active) {
                                        console.log("No longer dragging")
                                        // console.log("\n" + JSON.stringify(listThing.model))
                                        // for each item in theModel.items, print the index
                                        let list = []
                                        for (var i = 0; i < theModel.items.count; i++) {
                                            list.push(theModel.items.get(i).model.index)
                                        }

                                        controller_manager.updateControllerOrder(list)

                                    }
                                }
                            }

                            Drag.active: dragHandler.active
                            Drag.source: icon
                            Drag.hotSpot.x: 36
                            Drag.hotSpot.y: 36
                            Drag.keys: ["good"]

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

                            Rectangle {
                                anchors.fill: parent
                                color: "#25282C"
                                radius: 6
                            }

                            DetailsButton {
                                anchors.right: parent.right
                                anchors.rightMargin: 8
                                anchors.topMargin: 8
                                anchors.top: parent.top

                                // onClicked: function () {
                                //     rightClickMenu.popup()
                                // }
                            }

                            Column {
                                anchors.horizontalCenter: parent.horizontalCenter
                                Image {
                                    source: model.image_url
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    fillMode: Image.PreserveAspectFit
                                    width: 182
                                    // mipmap: true
                                    sourceSize.width: 182
                                }
                                Row {
                                    spacing: 4
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    Repeater {
                                        model: myself.model.player_index + 1
                                        delegate: Rectangle {
                                            width: 8
                                            height: 8
                                            color: "#6bc80a"
                                            radius: 4

                                        }
                                    }
                                }
                            }
                            // Text {
                            //     text: model.model_name
                            //     color: "white"
                            //     font.pointSize: 12
                            //     font.family: Constants.regularFontFamily
                            //     horizontalAlignment: Text.AlignHCenter
                            //     verticalAlignment: Text.AlignVCenter
                            // }
                            // Text {
                            //     text: model.wired ? "Wired" : "Wireless"
                            //     color: "white"
                            //     font.pointSize: 12
                            //     font.family: Constants.regularFontFamily
                            //     horizontalAlignment: Text.AlignHCenter
                            //     verticalAlignment: Text.AlignVCenter
                            // }
                        }
                    }
                }
            }
        }
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.horizontalStretchFactor: 1
        }
    }


}