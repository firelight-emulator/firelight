import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts 1.0
import QtMultimedia
import QtQuick.Effects
import Firelight 1.0

FocusScope {
    id: root
    // color: "black"
    state: "hidden"

    required property var model

    Rectangle {
        color: "black"
        anchors.fill: parent
    }

    signal hidden()

    signal showing()

    property bool thing: false

    Keys.onSpacePressed: function (event) {
        // root.StackView.view.popCurrentItem(StackView.Immediate)
        exitAnimation.start()
    }

    StackView.onActivated: function () {
        // enterAnimation.play()
        enterAnimation.start()
        // coverImage.height = root.height - 150
    }

    ParallelAnimation {
        id: enterAnimation
        NumberAnimation {
            target: barthing
            property: "y"
            from: root.height
            to: root.height - 150
            duration: 220
            easing.type: Easing.InOutQuad
        }
        NumberAnimation {
            target: coverImage
            property: "height"
            from: root.height
            to: root.height - 150
            duration: 220
            easing.type: Easing.InOutQuad
        }
    }

    SequentialAnimation {
        id: exitAnimation
        ParallelAnimation {
            NumberAnimation {
                target: barthing
                property: "y"
                from: root.height - 150
                to: root.height
                duration: 220
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: coverImage
                property: "height"
                from: root.height - 150
                to: root.height
                duration: 220
                easing.type: Easing.InOutQuad
            }
        }
        ScriptAction {
            script: {
                root.StackView.view.popCurrentItem(StackView.Immediate)
            }
        }
    }

    // SequentialAnimation {
    //     id: enterAnimation
    //     ParallelAnimation {
    //         NumberAnimation {
    //             target: barthing
    //             property: "y"
    //             to: parent.height - 150
    //             duration: 220
    //             easing.type: Easing.InOutQuad
    //         }
    //         NumberAnimation {
    //             target: coverImage
    //             property: "height"
    //             from: parent.height
    //             to: parent.height - 150
    //             duration: 220
    //             easing.type: Easing.InOutQuad
    //         }
    //     }
    //
    // }

    // function show() {
    //     console.log("showing")
    //     state = "showing"
    // }
    //
    // function goaway() {
    //     console.log("hiding")
    //     state = "hidden"
    // }

    // onThingChanged: function () {
    //     console.log("thing changed", thing)
    // }
    //
    // onStateChanged: function () {
    //     console.log("state changed", state)
    //     theList.reset()
    // }

    // states: [
    //     State {
    //         name: "showing"
    //         when: coverImageBg.thing
    //
    //         PropertyChanges {
    //             target: coverImageBg
    //             opacity: 1
    //         }
    //         PropertyChanges {
    //             target: coverImage
    //             width: parent.width
    //             height: parent.height - 150
    //         }
    //         PropertyChanges {
    //             target: barthing
    //             y: parent.height - 150
    //         }
    //     },
    //     State {
    //         name: "hidden"
    //         when: !coverImageBg.thing
    //
    //         PropertyChanges {
    //             target: coverImageBg
    //             opacity: 0
    //         }
    //         PropertyChanges {
    //             target: coverImage
    //             width: parent.width
    //             height: parent.height
    //         }
    //         PropertyChanges {
    //             target: barthing
    //             y: parent.height
    //         }
    //     }
    // ]

    // transitions: [
    //     Transition {
    //         from: "hidden"
    //         to: "showing"
    //         SequentialAnimation {
    //             PropertyAction {
    //                 target: coverImageBg
    //                 property: "opacity"
    //                 value: 1
    //             }
    //             ParallelAnimation {
    //                 NumberAnimation {
    //                     target: coverImage
    //                     properties: "width, height"
    //                     duration: 220
    //                     easing.type: Easing.InOutQuad
    //                 }
    //                 NumberAnimation {
    //                     target: barthing
    //                     properties: "y"
    //                     duration: 220
    //                     easing.type: Easing.InOutQuad
    //                 }
    //             }
    //         }
    //     },
    //     Transition {
    //         from: "showing"
    //         to: "hidden"
    //         SequentialAnimation {
    //             ParallelAnimation {
    //                 NumberAnimation {
    //                     target: barthing
    //                     properties: "y"
    //                     duration: 220
    //                     easing.type: Easing.InOutQuad
    //                 }
    //                 NumberAnimation {
    //                     target: coverImage
    //                     properties: "width, height"
    //                     duration: 220
    //                     easing.type: Easing.InOutQuad
    //                 }
    //             }
    //             PropertyAction {
    //                 target: coverImageBg
    //                 property: "opacity"
    //                 value: 0
    //             }
    //             ScriptAction {
    //                 script: {
    //                     coverImageBg.hidden()
    //                 }
    //             }
    //         }
    //     }
    // ]

    Image {
        id: coverImage
        anchors.horizontalCenter: parent.horizontalCenter
        // height: parent.height
        width: parent.width
        height: parent.height - 150
        mipmap: true
        smooth: false
        cache: false
        source: theList.count > 0 ? theList.itemAtIndex(theList.currentIndex).model.modelData.image_url : ""
        fillMode: Image.PreserveAspectFit
    }


    Pane {
        id: barthing


        width: parent.width
        height: 150
        y: parent.height - 150
        background: Rectangle {
            color: "grey"
        }

        // Behavior on y {
        //     NumberAnimation {
        //         duration: 220
        //         easing.type: Easing.InOutQuad
        //     }
        // }

        contentItem: ListView {
            id: theList
            orientation: ListView.Horizontal
            layoutDirection: Qt.RightToLeft
            focus: true
            highlightMoveDuration: 80
            highlightMoveVelocity: -1
            keyNavigationEnabled: true
            highlightRangeMode: ListView.StrictlyEnforceRange
            preferredHighlightBegin: width / 2 - 75
            preferredHighlightEnd: width / 2 + 75
            currentIndex: 0
            highlight: Item {
            }

            Component.onCompleted: {
                console.log("count: " + theList.count)
                theList.currentIndex = 0
            }

            // SoundEffect {
            //     id: scrollSound
            //     loops: 0
            //     source: "file:system/sfx/glass_001.wav"
            // }

            onCurrentIndexChanged: {
                // if (currentIndex == 0) {
                //     coverImage.source = "image://gameImages/0"
                // } else {
                //     coverImage.source = theList.itemAtIndex(currentIndex).model.image_url
                // }
                // if (coverImageBg.state === "showing") {
                //     sfx_player.play()
                // }
            }

            // Keys.onEscapePressed: function (event) {
            //     console.log(":)")
            //     rewindList.visible = false
            // }
            spacing: 8
            // model: ListModel {
            //     id: theModel
            //     ListElement {
            //         imageSource: "file:system/_img/cursedmirror1.png"
            //     }
            //     ListElement {
            //         imageSource: "file:system/_img/cursedmirror2.png"
            //     }
            //     ListElement {
            //         imageSource: "file:system/_img/cursedmirror3.png"
            //     }
            //     ListElement {
            //         imageSource: "file:system/_img/cursedmirror4.png"
            //     }
            //     ListElement {
            //         imageSource: "file:system/_img/cursedmirror5.png"
            //     }
            // }
            // model: rewind_model
            model: root.model
            // delegate: Rectangle {
            //     color: "red"
            //     width: 100
            //     height: 100
            // }
            delegate: Image {
                required property var model
                required property var index
                Rectangle {
                    visible: parent.ListView.isCurrentItem
                    z: -1
                    width: parent.width + 8
                    height: parent.height + 8
                    x: -4
                    y: -4
                    color: "white"
                }

                Component.onCompleted: {
                    // print all keys in model
                    // for (const key in model.model) {
                    //     console.log("key: " + key)
                    //     console.log("value: " + model[key])
                    // }

                    console.log("modelData: " + JSON.stringify(model.modelData))
                }
                cache: false
                TapHandler {
                    onTapped: {
                        theList.currentIndex = index
                    }
                }


                mipmap: true
                smooth: false
                // source: rewindList.imageSource !== null ? rewindList.imageSource : ""
                source: model.modelData.image_url
                anchors.verticalCenter: ListView.view.contentItem.verticalCenter
                height: ListView.isCurrentItem ? ListView.view.height + 20 : ListView.view.height
                Behavior on height {
                    NumberAnimation {
                        duration: 100
                        easing.type: Easing.InOutQuad
                    }
                }
                Behavior on width {
                    NumberAnimation {
                        duration: 100
                        easing.type: Easing.InOutQuad
                    }
                }
                fillMode: Image.PreserveAspectFit
            }
        }
    }


}