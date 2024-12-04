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

            color: ColorPalette.neutral700

            property point dragEnd: Qt.point(0, 0)

            width: ListView.view.width
            height: 100
            Drag.active: dragArea.active
            Drag.source: content
            Drag.hotSpot.x: width / 2
            Drag.hotSpot.y: height / 2
            Drag.keys: ["controller-reorder"]

            // GamepadStatus {
            //     id: status
            //     playerNumber: index + 1
            // }

            Text {
                text: model.model_name
            }

            NumberAnimation {
                id: moveAnimation
                target: content
            }


            DragHandler {
                id: dragArea

                grabPermissions: PointerHandler.TakeOverForbidden

                yAxis.enabled: true
                xAxis.enabled: false

                onActiveChanged: {
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
                width: parent.width
                height: parent.height / 2 - 2
                anchors.top: parent.top
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
                width: parent.width
                height: parent.height / 2 - 2
                anchors.bottom: parent.bottom
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
                // Layout.preferredWidth: 836
                Layout.preferredWidth: 500
                Layout.alignment: Qt.AlignHCenter
                Layout.fillHeight: true

                interactive: false
                // orientation: ListView.Horizontal

                moveDisplaced: Transition {
                    NumberAnimation {
                        properties: "x,y"; duration: 100;
                        easing.type: Easing.InOutQuad
                    }
                }

                // populate: Transition {
                //     NumberAnimation {
                //         properties: "x,y"; duration: 300
                //     }
                // }


                model: visualModel

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
                // model: DelegateModel {
                //     id: theModel
                //     model: controller_model
                //     delegate: Image {
                //         required property var model
                //         required property int index
                //
                //         id: controllerIcon
                //         anchors.horizontalCenter: parent.horizontalCenter
                //         anchors.verticalCenter: parent.verticalCenter
                //         source: model.image_url
                //         fillMode: Image.PreserveAspectFit
                //         width: 182
                //         sourceSize.width: 182
                //
                //         property Item dragParent: null
                //
                //         DragHandler {
                //             id: dragHandler
                //
                //             grabPermissions: PointerHandler.TakeOverForbidden
                //
                //             // onActiveChanged: function () {
                //             //     if (!dragHandler.active) {
                //             //         // console.log("\n" + JSON.stringify(listThing.model))
                //             //         // for each item in theModel.items, print the index
                //             //         let list = []
                //             //         for (var i = 0; i < theModel.items.count; i++) {
                //             //             list.push(theModel.items.get(i).model.index)
                //             //         }
                //             //
                //             //         controller_manager.updateControllerOrder(list)
                //             //
                //             //     }
                //             // }
                //         }
                //
                //         Drag.onActiveChanged: function () {
                //             root.isDragging = Drag.active
                //             // if (!Drag.active) {
                //             //     console.log("Dropped the thing")
                //             //     if (dragParent !== null) {
                //             //         parent = dragParent
                //             //     }
                //             //
                //             //     dragParent = null
                //             // }
                //         }
                //
                //         Drag.active: dragHandler.active
                //         Drag.source: controllerIcon
                //         Drag.hotSpot.x: controllerIcon.width / 2
                //         Drag.hotSpot.y: controllerIcon.height / 2
                //         Drag.keys: ["controller-reorder"]
                //
                //         states: [
                //             State {
                //                 name: "NotDragging"
                //                 when: !dragHandler.active
                //             },
                //             State {
                //                 name: "Dragging"
                //                 when: dragHandler.active
                //
                //                 AnchorChanges {
                //                     target: controllerIcon
                //                     anchors {
                //                         horizontalCenter: undefined
                //                         verticalCenter: undefined
                //                     }
                //                 }
                //             }
                //         ]
                //
                //         transitions: Transition {
                //             from: "Dragging"
                //             to: "NotDragging"
                //             AnchorAnimation {
                //                 duration: 600;
                //                 easing.type: Easing.OutElastic
                //                 easing.amplitude: 0.5
                //             }
                //         }
                //
                //
                //         // background: Rectangle {
                //         //     color: "#25282C"
                //         //     radius: 4
                //         // }
                //         //
                //         // onEntered: function (drag) {
                //         //     if (drag.source === icon) {
                //         //         return
                //         //     }
                //         //
                //         //     theModel.items.move(drag.source.DelegateModel.itemsIndex, icon.DelegateModel.itemsIndex)
                //         // }
                //
                //         // Item {
                //         //     id: thingy
                //         //     visible: model.model_name === "Default"
                //         //
                //         //     height: 220
                //         //     width: 200
                //         //
                //         //     Rectangle {
                //         //         anchors.fill: parent
                //         //         border.color: "#515151"
                //         //         color: "transparent"
                //         //         radius: 6
                //         //
                //         //         Text {
                //         //             anchors.centerIn: parent
                //         //             text: "No controller connected"
                //         //             color: "#757575"
                //         //             font.pointSize: 11
                //         //             font.family: Constants.regularFontFamily
                //         //             horizontalAlignment: Text.AlignHCenter
                //         //             verticalAlignment: Text.AlignVCenter
                //         //         }
                //         //     }
                //         //
                //         //     anchors {
                //         //         horizontalCenter: parent.horizontalCenter
                //         //         verticalCenter: parent.verticalCenter
                //         //     }
                //         // }
                //
                //         // Item {
                //         //     id: icon
                //         //     // visible: model.model_name !== "Default"
                //         //
                //         //     height: contentColumn.height + 10
                //         //     width: 200
                //         //
                //         //     property Item dragParent: listThing
                //         //     property var myIndex: parent.index
                //         //
                //         //     HoverHandler {
                //         //         cursorShape: icon.state === "Dragging" ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                //         //         onGrabChanged: {
                //         //             console.log("grabChanged")
                //         //         }
                //         //     }
                //         //
                //         //     anchors {
                //         //         horizontalCenter: parent.horizontalCenter
                //         //         verticalCenter: parent.verticalCenter
                //         //     }
                //         //
                //         //     Rectangle {
                //         //         anchors.fill: parent
                //         //         color: "#25282C"
                //         //         radius: 6
                //         //     }
                //         //
                //         //     // DetailsButton {
                //         //     //     anchors.right: parent.right
                //         //     //     anchors.rightMargin: 8
                //         //     //     anchors.topMargin: 8
                //         //     //     anchors.top: parent.top
                //         //     // }
                //         //
                //         //     Column {
                //         //         id: contentColumn
                //         //         anchors.horizontalCenter: parent.horizontalCenter
                //         //         spacing: 8
                //         //         Image {
                //         //             source: model.image_url
                //         //             anchors.horizontalCenter: parent.horizontalCenter
                //         //             fillMode: Image.PreserveAspectFit
                //         //             width: 182
                //         //             // mipmap: true
                //         //             sourceSize.width: 182
                //         //
                //         //             DragHandler {
                //         //                 id: dragHandler
                //         //
                //         //                 grabPermissions: PointerHandler.TakeOverForbidden
                //         //
                //         //                 onActiveChanged: function () {
                //         //                     if (!dragHandler.active) {
                //         //                         // console.log("\n" + JSON.stringify(listThing.model))
                //         //                         // for each item in theModel.items, print the index
                //         //                         let list = []
                //         //                         for (var i = 0; i < theModel.items.count; i++) {
                //         //                             list.push(theModel.items.get(i).model.index)
                //         //                         }
                //         //
                //         //                         controller_manager.updateControllerOrder(list)
                //         //
                //         //                     }
                //         //                 }
                //         //             }
                //         //
                //         //             Drag.active: dragHandler.active
                //         //             Drag.source: icon
                //         //             Drag.hotSpot.x: icon.width / 2
                //         //             Drag.hotSpot.y: icon.height / 2
                //         //             Drag.keys: ["good"]
                //         //
                //         //             states: [
                //         //                 State {
                //         //                     name: "NotDragging"
                //         //                     when: !dragHandler.active
                //         //                 },
                //         //                 State {
                //         //                     name: "Dragging"
                //         //                     when: dragHandler.active
                //         //                     ParentChange {
                //         //                         target: icon
                //         //                         parent: icon.dragParent
                //         //                     }
                //         //
                //         //                     AnchorChanges {
                //         //                         target: icon
                //         //                         anchors {
                //         //                             horizontalCenter: undefined
                //         //                             verticalCenter: undefined
                //         //                         }
                //         //                     }
                //         //                 }
                //         //             ]
                //         //         }
                //         //         // Row {
                //         //         //     spacing: 4
                //         //         //     anchors.horizontalCenter: parent.horizontalCenter
                //         //         //     Repeater {
                //         //         //         model: myself.model.player_index + 1
                //         //         //         delegate: Rectangle {
                //         //         //             width: 8
                //         //         //             height: 8
                //         //         //             color: "#6bc80a"
                //         //         //             radius: 4
                //         //         //
                //         //         //         }
                //         //         //     }
                //         //         // }
                //         //         // Item {
                //         //         //     height: 6
                //         //         //     width: 1
                //         //         // }
                //         //         // MyComboBox {
                //         //         //     textRole: "text"
                //         //         //     valueRole: "value"
                //         //         //     width: parent.width
                //         //         //
                //         //         //     // onActivated: library_short_model.sortType = currentValue
                //         //         //     // Component.onCompleted: currentIndex = indexOfValue(library_short_model.sortType)
                //         //         //
                //         //         //     model: [
                //         //         //         {text: "Default profile", value: "display_name"},
                //         //         //         {text: "Newest first", value: "created_at"}
                //         //         //     ]
                //         //         // }
                //         //         // Button {
                //         //         //     width: parent.width
                //         //         //     padding: 8
                //         //         //     background: Rectangle {
                //         //         //         color: "#03438c"
                //         //         //         radius: 8
                //         //         //     }
                //         //         //     contentItem: Text {
                //         //         //         text: qsTr("Edit current profile")
                //         //         //         color: "white"
                //         //         //         font.pointSize: 11
                //         //         //         font.weight: Font.DemiBold
                //         //         //         font.family: Constants.regularFontFamily
                //         //         //         horizontalAlignment: Text.AlignHCenter
                //         //         //         verticalAlignment: Text.AlignVCenter
                //         //         //     }
                //         //         //
                //         //         //     onClicked: function () {
                //         //         //         root.StackView.view.push(profileEditor)
                //         //         //         // profileDialog.open()
                //         //         //     }
                //         //         // }
                //         //     }
                //         //     // Text {
                //         //     //     text: model.model_name
                //         //     //     color: "white"
                //         //     //     font.pointSize: 12
                //         //     //     font.family: Constants.regularFontFamily
                //         //     //     horizontalAlignment: Text.AlignHCenter
                //         //     //     verticalAlignment: Text.AlignVCenter
                //         //     // }
                //         //     // Text {
                //         //     //     text: model.wired ? "Wired" : "Wireless"
                //         //     //     color: "white"
                //         //     //     font.pointSize: 12
                //         //     //     font.family: Constants.regularFontFamily
                //         //     //     horizontalAlignment: Text.AlignHCenter
                //         //     //     verticalAlignment: Text.AlignVCenter
                //         //     // }
                //         // }
                //     }
                // }
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
        contentItem: ControllerProfilePage {
            controllerProfileId: 1
        }

        footer: DialogButtonBox {
            spacing: 2
            alignment: Qt.AlignRight
            onAccepted: {
                console.log("Accepted")
                profileDialog.contentItem.saveMapping()
            }
            Button {
                background: Rectangle {
                    implicitHeight: 44
                    implicitWidth: 100
                    color: "#03438c"
                    radius: 8
                }
                contentItem: Text {
                    text: qsTr("Save")
                    color: "white"
                    font.pointSize: 11
                    font.weight: Font.DemiBold
                    font.family: Constants.regularFontFamily
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                }
                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            }
            Button {
                background: Rectangle {
                    implicitHeight: 44
                    implicitWidth: 100

                    color: "transparent"
                    radius: 8
                }
                contentItem: Text {
                    text: qsTr("Cancel")
                    color: "white"
                    font.pointSize: 11
                    font.family: Constants.regularFontFamily
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                }
                DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
            }
            buttonLayout: DialogButtonBox.MacLayout
            background: Item {
            }
        }
    }


}