import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

Flickable {
    id: root

    required property int modId
    required property string name
    required property string author
    required property string description
    required property string targetGameName
    required property bool targetInLibrary
    required property bool modInLibrary

    contentHeight: theColumn.height
    boundsBehavior: Flickable.StopAtBounds

    ScrollBar.vertical: ScrollBar {}

    Image {
        id: headerBanner
        width: parent.width + 24
        height: header.height + headerSpacer.height + 24
        x: -12
        y: -12

        source: modId === 2 ? "file:system/_img/rrbanner.png" : "file:system/_img/goombanner.png"
        fillMode: Image.PreserveAspectCrop

        layer.enabled: true
        layer.effect: MultiEffect {
            autoPaddingEnabled: false
            source: headerBanner
            anchors.fill: headerBanner
            blurEnabled: true
            blurMultiplier: 1.0
            blurMax: 64
            blur: 1
        }
    }

    RowLayout {
        id: contentRow
        anchors.fill: parent
        spacing: 0

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.horizontalStretchFactor: 1
        }

        ColumnLayout {
            id: theColumn
            Layout.preferredWidth: parent.width * 3 / 4
            Layout.fillHeight: true

            RowLayout {
                id: header
                Layout.preferredHeight: 260
                Layout.minimumHeight: 260

                ColumnLayout {
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignLeft | Qt.AlignBottom

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }

                    Text {
                        text: root.name
                        color: "white"
                        Layout.fillWidth: true
                        font.pixelSize: AppStyle.fontSizeXLarge
                        font.family: Constants.regularFontFamily
                        font.weight: Font.DemiBold
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                    }

                    Text {
                        text: "By " + author
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        width: parent.width
                        font.pixelSize: AppStyle.fontSizeMedium
                        font.family: Constants.regularFontFamily
                        color: "white"
                        Layout.fillWidth: true
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                ColumnLayout {
                    Layout.fillHeight: true
                    Layout.maximumWidth: 180
                    spacing: 8
                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                    Text {
                        text: targetInLibrary ? "The required game is in your library" : "You need " + targetGameName + " in your library to play this mod"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        Layout.alignment: Qt.AlignRight | Qt.AlignBottom
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                        font.pixelSize: AppStyle.fontSizeSmall
                        font.family: Constants.regularFontFamily
                        color: "#d5d5d5"
                    }
                    Button {
                        id: addToLibraryButton
                        Layout.preferredHeight: 60
                        Layout.preferredWidth: 180
                        Layout.alignment: Qt.AlignRight | Qt.AlignBottom
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
                            font.pixelSize: AppStyle.fontSizeMedium
                        }

                        HoverHandler {
                            acceptedDevices: PointerDevice.Mouse
                            cursorShape: Qt.PointingHandCursor
                        }

                        onClicked: {
                            addedPopup.open();
                            timer.start();
                            root.modInLibrary = true;
                            library_model.addModToLibrary(root.modId);
                        }
                    }
                }
            }

            Item {
                id: headerSpacer
                Layout.fillWidth: true
                Layout.preferredHeight: 8
            }

            ListView {
                id: listView
                Layout.topMargin: 18
                Layout.fillWidth: true
                Layout.preferredHeight: 160
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                clip: true
                orientation: ListView.Horizontal
                model: ListModel {
                    ListElement {
                        source: "file:system/_img/goom2.png"
                        modId: 1
                    }
                    ListElement {
                        source: "file:system/_img/goom1.png"
                        modId: 1
                    }
                    ListElement {
                        source: "file:system/_img/goom3.png"
                        modId: 1
                    }
                    ListElement {
                        source: "file:system/_img/goom4.png"
                        modId: 1
                    }
                    ListElement {
                        source: "file:system/_img/rr1.png"
                        modId: 2
                    }
                    ListElement {
                        source: "file:system/_img/rr2.png"
                        modId: 2
                    }
                    ListElement {
                        source: "file:system/_img/rr3.png"
                        modId: 2
                    }
                    ListElement {
                        source: "file:system/_img/rr4.png"
                        modId: 2
                    }
                    ListElement {
                        source: "file:system/_img/rr5.png"
                        modId: 2
                    }
                    ListElement {
                        source: "file:system/_img/rr6.png"
                        modId: 2
                    }
                    ListElement {
                        source: "file:system/_img/rr7.png"
                        modId: 2
                    }
                    ListElement {
                        source: "file:system/_img/rr8.png"
                        modId: 2
                    }
                }
                spacing: 8
                delegate: Image {
                    width: model.modId === root.modId ? listView.height * (16 / 9) : -8
                    height: listView.height
                    visible: model.modId === root.modId

                    source: model.source
                    fillMode: Image.Stretch
                }
            }

            Row {
                Layout.fillWidth: true
                Layout.preferredHeight: 24

                Text {
                    text: "Click and drag to see all screenshots"
                    font.pixelSize: AppStyle.fontSizeSmall
                    font.family: Constants.regularFontFamily
                    font.weight: Font.Medium
                    color: "white"
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Text {
                text: description
                Layout.topMargin: 12
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                font.pixelSize: AppStyle.fontSizeMedium
                font.family: Constants.regularFontFamily
                color: "#d5d5d5"
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.horizontalStretchFactor: 1
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
            font.pixelSize: AppStyle.fontSizeMedium
            font.family: Constants.regularFontFamily
            color: "#212020"
        }

        onAboutToShow: {
            words.text = "Added " + root.name + " to library";
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
            addedPopup.close();
        }
    }
}
