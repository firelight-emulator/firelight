import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Flickable {
    id: root

    property bool isDragging: false

    Component {
        id: dragDelegate

        Rectangle {
            id: content
            required property var model
            required property var index

            color: ColorPalette.neutral800
            radius: 2

            z: dragArea.active ? 1 : 0

            property point dragEnd: Qt.point(0, 0)

            width: 200
            height: 200
            Drag.active: dragArea.active
            Drag.source: content
            Drag.hotSpot.x: width / 2
            Drag.hotSpot.y: height / 2
            Drag.keys: ["controller-reorder"]

            // GamepadStatus {
            //     id: status
            //     playerNumber: index + 1
            // }

            Image {
                id: controllerIcon
                source: model.image_url
                visible: model.model_name !== "Default"
                fillMode: Image.PreserveAspectFit
                width: 182
                sourceSize.width: 182
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: "No controller connected"
                color: ColorPalette.neutral500
                font.pixelSize: 14
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.family: Constants.regularFontFamily
                visible: model.model_name === "Default"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }

            FirelightButton {
                anchors.top: parent.bottom
                anchors.topMargin: 16
                visible: !root.isDragging && model.model_name !== "Default"
                focus: true

                label: "Edit profile"

                onClicked: function () {
                    // profileDialog.open()
                    screenStack.pushItem(profileEditor, {}, StackView.PushTransition)
                }
            }

            Text {
                id: playerNum
                text: (content.index + 1)
                color: "white"
                font.pixelSize: 14
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                anchors.top: parent.top
                anchors.topMargin: 8
                anchors.left: parent.left
                anchors.leftMargin: 8
                width: 16
                height: 16
            }

            // Text {
            //     text: model.model_name
            // }

            NumberAnimation {
                id: moveAnimation
                target: content
            }

            DragHandler {
                id: dragArea

                grabPermissions: PointerHandler.TakeOverForbidden

                yAxis.enabled: false
                xAxis.enabled: true

                onActiveChanged: {
                    root.isDragging = dragArea.active
                    if (dragArea.active) {
                        content.dragEnd = Qt.point(content.x, content.y)
                    }

                    if (!dragArea.active) {
                        let list = []
                        for (let i = 0; i < visualModel.items.count; i++) {
                            // console.log(visualModel.items.get(i).model.index)
                            list.push(visualModel.items.get(i).model.index)
                            console.log("i: " + i + " index: " + visualModel.items.get(i).model.index)
                            // visualModel.items.get(i).model.itemsIndex = i
                        }

                        // theAnimation.thingy = list
                        // theAnimation.start()

                        controller_manager.updateControllerOrder(list)
                        // visualModel.items.sort()
                    }
                }
            }
            // SequentialAnimation {
            //     id: theAnimation
            //
            //     property var thingy
            //
            //     ParallelAnimation {
            //         NumberAnimation {
            //             id: xAnim
            //             target: content
            //             property: "x"
            //             to: content.dragEnd.x
            //             duration: 200
            //             easing.type: Easing.InOutQuad
            //         }
            //         NumberAnimation {
            //             id: yAnim
            //             target: content
            //             property: "y"
            //             to: content.dragEnd.y
            //             duration: 200
            //             easing.type: Easing.InOutQuad
            //         }
            //     }
            //     ScriptAction {
            //         script: function () {
            //             controller_manager.updateControllerOrder(thingy)
            //         }
            //     }
            // }


            DropArea {
                width: parent.width / 2 - 2
                height: parent.height
                anchors.left: parent.left
                // anchors {
                //     fill: parent
                //     margins: 30
                // }

                onEntered: (drag) => {
                    drag.source.dragEnd = Qt.point(content.x, content.y)

                    const mine = content.DelegateModel.itemsIndex
                    const theirs = drag.source.DelegateModel.itemsIndex
                    if (mine < theirs) {
                        visualModel.items.move(theirs, mine)
                    }
                }
            }

            DropArea {
                width: parent.width / 2 - 2
                height: parent.height
                anchors.right: parent.right
                // anchors {
                //     fill: parent
                //     margins: 30
                // }

                onEntered: (drag) => {
                    drag.source.dragEnd = Qt.point(content.x, content.y)

                    const mine = content.DelegateModel.itemsIndex
                    const theirs = drag.source.DelegateModel.itemsIndex
                    if (mine > theirs) {
                        visualModel.items.move(theirs, mine)
                    }
                }
            }
        }


        // MouseArea {
        //     id: dragArea
        //
        //     property bool held: false
        //
        //     drag.target: held ? content : undefined
        //     drag.axis: Drag.YAxis
        //
        //     onPressAndHold: held = true
        //     onReleased: held = false
        //
        //     Rectangle {
        //         id: content
        //         anchors.fill: parent
        //         Drag.active: dragArea.held
        //         Drag.source: dragArea
        //         Drag.hotSpot.x: width / 2
        //         Drag.hotSpot.y: height / 2
        //     }
        //     DropArea {
        //         anchors {
        //             fill: parent
        //             margins: 10
        //         }
        //
        //         onEntered: (drag) => {
        //             visualModel.items.move(
        //                 drag.source.DelegateModel.itemsIndex,
        //                 dragArea.DelegateModel.itemsIndex)
        //         }
        //     }
        // }
    }

    DelegateModel {
        id: visualModel

        model: controller_model
        delegate: dragDelegate
    }

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

                focus: true
                keyNavigationEnabled: true
                Layout.preferredWidth: 836
                // Layout.preferredWidth: 500
                Layout.alignment: Qt.AlignHCenter
                Layout.fillHeight: true

                interactive: false
                orientation: ListView.Horizontal

                moveDisplaced: Transition {
                    NumberAnimation {
                        properties: "x,y"; duration: 100;
                        easing.type: Easing.InOutQuad
                    }
                }


                model: visualModel
                spacing: 12

            }
        }


        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.horizontalStretchFactor: 1
        }
    }

    Component {
        id: profileEditor
        ControllerProfilePage {
            controllerProfileId: 1
        }
    }

    FirelightDialog {
        id: profileDialog
        width: parent.width * 5 / 6
        height: parent.height * 5 / 6
        // title: "Creating new controller profile"
        centerButtons: false
        acceptText: "Save"
        contentItem: ControllerProfilePage {
            controllerProfileId: 1
        }
    }


}