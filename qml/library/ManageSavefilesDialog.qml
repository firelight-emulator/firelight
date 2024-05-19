import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Effects
import Firelight 1.0


Dialog {
    id: control

    modal: true
    parent: Overlay.overlay
    anchors.centerIn: parent
    padding: 24

    background: Rectangle {
        color: Constants.colorTestSurfaceContainerLowest
        radius: 6
    }

    header: Text {
        padding: 12
        color: Constants.colorTestTextActive
        font.family: Constants.regularFontFamily
        text: qsTr("Manage Save Files")
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pointSize: 16
    }

    contentItem: ListView {
        id: listView
        implicitWidth: 400
        implicitHeight: 512
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        model: ListModel {
            ListElement {
                slot: 1
                display_name: "Slot 1"
                thing: "lol"
            }
            ListElement {
                slot: 2
                display_name: "Slot 2"
                thing: ""
            }
            ListElement {
                slot: 3
                display_name: "Slot 3"
                thing: ""
            }
            ListElement {
                slot: 4
                display_name: "Slot 4"
                thing: ""
            }
            ListElement {
                slot: 5
                display_name: "Slot 5"
                thing: ""
            }
        }

        ButtonGroup {
            id: group
        }

        delegate: Item {
            width: listView.width
            height: 100

            RowLayout {
                anchors.fill: parent
                spacing: 12
                Text {
                    text: model.display_name
                    color: Constants.colorTestTextActive
                    font.family: Constants.regularFontFamily
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pointSize: 11
                }

                Button {
                    id: button
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    hoverEnabled: true
                    checkable: true
                    padding: 8
                    ButtonGroup.group: group

                    background: Rectangle {
                        color: button.checked ? "white" : button.hovered ? "grey" : "transparent"
                        radius: 4
                        anchors.fill: parent
                    }

                    onClicked: {
                        // console.log("Clicked on " + model.slot)
                    }

                    Text {
                        visible: model.thing === ""
                        text: "nothin here boss"
                        anchors.fill: parent
                        color: Constants.colorTestTextActive
                        font.family: Constants.regularFontFamily
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pointSize: 11
                    }

                    RowLayout {
                        visible: model.thing !== ""
                        anchors.fill: parent
                        Rectangle {
                            Layout.preferredHeight: 80
                            Layout.preferredWidth: 100
                            Layout.leftMargin: 8
                            color: "red"
                            radius: 6
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        }

                        ColumnLayout {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            Layout.leftMargin: 8
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                            spacing: 8

                            Text {
                                text: "Save slot 1 stuff"
                                color: Constants.colorTestTextActive
                                font.family: Constants.regularFontFamily
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignBottom
                                font.pointSize: 11
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                            }

                            Text {
                                text: "2021-01-01 12:00"
                                color: Constants.colorTestTextActive
                                font.family: Constants.regularFontFamily
                                horizontalAlignment: Text.AlignLeft
                                verticalAlignment: Text.AlignTop
                                font.pointSize: 11
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                            }
                        }
                    }
                }


                // ColumnLayout {
                //     Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                //
                //     Text {
                //         text: "Date: 2021-01-01"
                //         color: Constants.colorTestTextActive
                //         font.family: Constants.regularFontFamily
                //         font.pointSize: 11
                //     }
                //
                //     Text {
                //         text: "Time: 12:00"
                //         color: Constants.colorTestTextActive
                //         font.family: Constants.regularFontFamily
                //         font.pointSize: 11
                //     }
                // }
            }
        }
    }

    footer: DialogButtonBox {
        visible: true
        alignment: Qt.AlignHCenter | Qt.AlignVCenter
        background: Rectangle {
            color: "transparent"
        }
        Button {
            background: Rectangle {
                color: "white"
                radius: height / 2
                implicitWidth: 160
                implicitHeight: 48
            }
            contentItem: Text {
                text: qsTr("Close")
                color: Constants.colorTestBackground
                font.family: Constants.regularFontFamily
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pointSize: 12
            }
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole

            HoverHandler {
                acceptedDevices: PointerDevice.Mouse
                cursorShape: Qt.PointingHandCursor
            }
        }
    }

    // onAccepted: {
    //     appRoot.state = "notPlayingGame"
    //     // closeGameAnimation.startWith(outPage, inPage)
    // }

    Overlay.modal: Rectangle {
        color: "black"
        anchors.fill: parent
        opacity: 0.4

        Behavior on opacity {
            NumberAnimation {
                duration: 200
            }
        }
    }

    enter: Transition {
        NumberAnimation {
            property: "opacity";
            from: 0.0;
            to: 1.0
            duration: 200
        }
        NumberAnimation {
            property: "scale";
            from: 0.9;
            to: 1.0
            duration: 200
        }
    }

    exit: Transition {
        NumberAnimation {
            property: "opacity";
            from: 1.0;
            to: 0.0;
            duration: 200
        }
        NumberAnimation {
            property: "scale";
            from: 1.0;
            to: 0.9
            duration: 200
        }
    }
}