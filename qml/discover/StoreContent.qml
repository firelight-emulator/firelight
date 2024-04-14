import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models
import FirelightStyle 1.0

Item {
    id: root

    required property int modId
    required property string name
    required property string author
    required property string description
    required property string targetGameName
    required property bool targetInLibrary
    required property bool modInLibrary
    required property int gameReleaseId
    required property int romId

    Pane {
        id: content

        background: Item {
        }
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        Pane {
            background: Item {
            }
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.left: scrollableArea.right

            ColumnLayout {
                anchors.fill: parent
                spacing: 16

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                Text {
                    text: targetInLibrary ? "The required game is in your library" : "You need " + targetGameName + " in your library to play this mod"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    font.pointSize: 10
                    font.family: Constants.regularFontFamily
                    color: "#d5d5d5"
                }

                Button {
                    id: addToLibraryButton
                    Layout.preferredHeight: 60
                    Layout.preferredWidth: parent.width * 3 / 4
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    background: Rectangle {
                        color: enabled ? "white" : "grey"
                        radius: 6
                    }
                    enabled: !modInLibrary && targetInLibrary
                    contentItem: Text {
                        text: modInLibrary ? qsTr("In Library") : qsTr("Add to Library")
                        anchors.centerIn: parent
                        color: Constants.colorTestBackground
                        font.family: Constants.regularFontFamily
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pointSize: 11
                    }

                    HoverHandler {
                        acceptedDevices: PointerDevice.Mouse
                        cursorShape: Qt.PointingHandCursor
                    }

                    onClicked: {
                        addedPopup.open()
                        timer.start()
                        root.modInLibrary = true
                        library_model.addModToLibrary(root.modId)
                    }

                    // onClicked: {
                    //     if (!targetInLibrary) {
                    //         for (let i = 0; i < romIds.length; i++) {
                    //             library_model.addRomToLibrary(romIds[i])
                    //         }
                    //         targetInLibrary = true
                    //         addToLibraryButton.text = qsTr("In Library")
                    //     }
                    // }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
        }

        Flickable {
            id: scrollableArea
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            width: parent.width * 2 / 3
            // contentWidth: content.width * 2 / 3
            // contentHeight: 1000
            contentHeight: header.height + column.height
            boundsBehavior: Flickable.StopAtBounds
            flickableDirection: Flickable.VerticalFlick
            clip: true

            ScrollBar.vertical: ScrollBar {
            }

            ColumnLayout {
                id: header
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                spacing: 8

                Text {
                    text: name
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    Layout.fillWidth: true
                    font.pointSize: 20
                    font.family: Constants.semiboldFontFamily
                    color: "white"
                }

                Text {
                    text: "By " + author
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    Layout.fillWidth: true
                    font.pointSize: 10
                    font.family: Constants.regularFontFamily
                    color: "white"
                }
            }

            ColumnLayout {
                id: column
                anchors.top: header.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                Text {
                    text: "Screenshots will go here"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 180
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.WordWrap
                    font.pointSize: 11
                    font.family: Constants.regularFontFamily
                    color: "#d5d5d5"
                }

                Text {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    text: description
                    wrapMode: Text.WordWrap
                    font.pointSize: 12
                    font.family: Constants.regularFontFamily
                    color: "#d5d5d5"
                }
            }
        }
    }

    Popup {
        id: addedPopup

        parent: Overlay.overlay
        x: parent.width / 2 - addedPopup.width / 2
        y: parent.height - addedPopup.height - 24

        width: words.width + padding * 2
        height: 50
        closePolicy: Popup.NoAutoClose

        background: Rectangle {
            color: "white"
            radius: 4
        }

        Text {
            id: words
            anchors.centerIn: parent
            text: ""
            font.pointSize: 11
            font.family: Constants.regularFontFamily
            color: "#212020"
        }

        onAboutToShow: {
            words.text = "Added " + root.name + " to library"
        }

        enter: Transition {
            NumberAnimation {
                properties: "opacity"
                from: 0
                to: 1
                duration: 200
                easing.type: Easing.InOutQuad
            }
        }

        exit: Transition {
            NumberAnimation {
                properties: "opacity"
                from: 1
                to: 0
                duration: 200
                easing.type: Easing.InOutQuad
            }
        }
    }

    Timer {
        id: timer
        interval: 3000 // Adjust the duration in milliseconds (e.g., 3000 for 3 seconds)
        running: false
        repeat: false
        onTriggered: {
            addedPopup.close()
        }
    }
}