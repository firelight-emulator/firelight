import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import QtMultimedia

FocusScope {
    id: root

    GridView {
        width: Math.floor(parent.width / cellWidth) * cellWidth
        height: parent.height
        anchors.centerIn: parent
        clip: true
        model: ListModel {
            ListElement {
                hasItem: true
                source: "https://media.retroachievements.org/Badge/100000.png"
            }
            ListElement {
                hasItem: true
                source: "https://media.retroachievements.org/Badge/100001.png"
            }
            ListElement {
                hasItem: true
                source: "https://media.retroachievements.org/Badge/100002.png"
            }
            ListElement {
                hasItem: true
                source: "https://media.retroachievements.org/Badge/100003.png"
            }
            ListElement {
                hasItem: true
                source: "https://media.retroachievements.org/Badge/100004.png"
            }
            ListElement {
                hasItem: true
                source: "https://media.retroachievements.org/Badge/100005.png"
            }
            ListElement {
                hasItem: true
                source: "https://media.retroachievements.org/Badge/100006.png"
            }
            ListElement {
                hasItem: true
                source: "https://media.retroachievements.org/Badge/100007.png"
            }
            ListElement {
                hasItem: true
                source: "https://media.retroachievements.org/Badge/100008.png"
            }
            ListElement {
                hasItem: true
                source: "https://media.retroachievements.org/Badge/100009.png"
            }
            ListElement {
                hasItem: true
                source: "https://media.retroachievements.org/Badge/100010.png"
            }
            ListElement {
                hasItem: true
                source: "https://media.retroachievements.org/Badge/100011.png"
            }
            ListElement {
                hasItem: true
                source: "https://media.retroachievements.org/Badge/100012.png"
            }
            ListElement {
                hasItem: true
                source: "https://media.retroachievements.org/Badge/100013.png"
            }
            ListElement {
                hasItem: true
                source: "https://media.retroachievements.org/Badge/100014.png"
            }
            ListElement {
                hasItem: true
                source: "https://media.retroachievements.org/Badge/100015.png"
            }
            ListElement {
                hasItem: true
                source: "https://media.retroachievements.org/Badge/100016.png"
            }
            ListElement {
                hasItem: true
                source: "https://media.retroachievements.org/Badge/100017.png"
            }
            ListElement {
                hasItem: true
                source: "https://media.retroachievements.org/Badge/100018.png"
            }
            ListElement {
                hasItem: true
                source: "https://media.retroachievements.org/Badge/100019.png"
            }
            ListElement {
                hasItem: true
                source: "https://media.retroachievements.org/Badge/100020.png"
            }
            ListElement {
                hasItem: false
                source: ""
            }
            ListElement {
                hasItem: false
                source: ""
            }
            ListElement {
                hasItem: false
                source: ""
            }
            ListElement {
                hasItem: false
                source: ""
            }
            ListElement {
                hasItem: false
                source: ""
            }
            ListElement {
                hasItem: false
                source: ""
            }
            ListElement {
                hasItem: false
                source: ""
            }
            ListElement {
                hasItem: false
                source: ""
            }
            ListElement {
                hasItem: false
                source: ""
            }
            ListElement {
                hasItem: false
                source: ""
            }
            ListElement {
                hasItem: false
                source: ""
            }
            ListElement {
                hasItem: false
                source: ""
            }
            ListElement {
                hasItem: false
                source: ""
            }
            ListElement {
                hasItem: false
                source: ""
            }
            ListElement {
                hasItem: false
                source: ""
            }
            ListElement {
                hasItem: false
                source: ""
            }
            ListElement {
                hasItem: false
                source: ""
            }
            ListElement {
                hasItem: false
                source: ""
            }
            ListElement {
                hasItem: false
                source: ""
            }
            ListElement {
                hasItem: false
                source: ""
            }
            ListElement {
                hasItem: false
                source: ""
            }
        }
        cellWidth: 140
        cellHeight: 140
        // interactive: false

        displaced: Transition {
            NumberAnimation {
                properties: "x,y"
                duration: 300
                easing.type: Easing.InOutQuad
            }
        }

        delegate: FocusScope {
            id: dele
            required property var model
            required property var index
            width: GridView.view.cellWidth
            height: GridView.view.cellHeight
            DropArea {
                id: dropArea
                property var index: dele.index
                anchors.fill: parent
                // onEntered: function(event) {
                //     let theirs = event.source.index
                //     let mine = dele.index
                //     if (theirs === mine) {
                //         return
                //     }
                //     dele.GridView.view.model.move(event.source.index, index, 1)
                //     console.log("entered")
                // }
                // onExited: {
                //     console.log("exited")
                // }
            }
            Rectangle {
                width: parent.width
                height: parent.height
                color: "lightgreen"
                radius: 8
                visible: dropArea.containsDrag
            }
            Button {
                id: deleButton
                visible: dele.model.hasItem
                property var index: dele.index
                z: state === "dragging" ? 1 : 0
                states: [
                    State {
                        name: "dragging"
                        when: dragger.active
                        AnchorChanges {
                            target: deleButton
                            anchors.horizontalCenter: undefined
                            anchors.verticalCenter: undefined
                        }
                    },
                    State {
                        name: "not-dragging"
                        when: !dragger.active
                        AnchorChanges {
                            target: deleButton
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                ]
                // transitions: [
                //     Transition {
                //         AnchorAnimation {
                //             duration: 300
                //             easing.type: Easing.OutElastic
                //             // easing.amplitude: 3
                //             easing.period: 0.5
                //         }
                //     }
                //
                // ]
                focus: true
                width: parent.width - 8
                height: parent.height - 8
                background: Rectangle {
                    color: dropArea.containsDrag ? "yellow" : "lightblue"
                    radius: 8
                }

                contentItem: Image {
                    source: dele.model.source
                }

                Drag.active: dragger.active
                Drag.hotSpot.x: width / 2
                Drag.hotSpot.y: height / 2

                DragHandler {
                    id: dragger
                    onActiveChanged: {
                        if (!active) {
                            let mine = dele.index
                            let theirs = deleButton.Drag.target.index

                            if (mine === theirs) {
                                return
                            }

                            let iHaveItem = dele.GridView.view.itemAtIndex(mine).model.hasItem
                            let theyHaveItem = dele.GridView.view.itemAtIndex(theirs).model.hasItem

                            let mySource = dele.GridView.view.itemAtIndex(mine).model.source
                            let theirSource = dele.GridView.view.itemAtIndex(theirs).model.source

                            dele.GridView.view.itemAtIndex(mine).model.hasItem = theyHaveItem
                            dele.GridView.view.itemAtIndex(theirs).model.hasItem = iHaveItem

                            dele.GridView.view.itemAtIndex(mine).model.source = theirSource
                            dele.GridView.view.itemAtIndex(theirs).model.source = mySource

                            console.log("my index is", dele.index, " their index is", deleButton.Drag.target.index)
                        }
                        console.log("drop target: ", deleButton.Drag.target)
                        console.log("active changed to", active)
                    }
                }
            }
        }
    }

}

