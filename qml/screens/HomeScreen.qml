import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtQuick.Layouts 1.0
import Firelight 1.0

FocusScope {
    id: root

    property bool showNowPlayingButton: false
    required property Component libraryPage
    required property Component modShopPage

    signal menuButtonClicked()

    Popup {
        id: scannerPopup
        width: 180
        height: 60
        modal: false
        focus: false
        visible: LibraryScanner.scanning
        x: (parent.width - width) / 2
        y: parent.height - height - 20
        background: Rectangle {
            color: ColorPalette.neutral800
        }
        contentItem: Text {
            text: "Updating library..."
            color: "white"
            font.pixelSize: 16
            font.family: Constants.regularFontFamily
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

    }

    Pane {
        id: headerBar
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.left: parent.left
        height: 72
        padding: 16

        KeyNavigation.down: contentStack

        background: Item {
        }

        contentItem: RowLayout {
            spacing: 24

            FirelightButton {
                id: hamburger

                tooltipLabel: "Menu"
                focus: true
                flat: true

                Layout.fillHeight: true
                Layout.preferredWidth: height
                Layout.leftMargin: 12

                iconCode: "\ue5d2"

                onClicked: {
                    root.menuButtonClicked()
                    // drawer2.open()
                }
            }

            Text {
                text: "Library"
                color: ColorPalette.neutral100
                font.pixelSize: 24
                font.weight: Font.Normal
                font.family: Constants.regularFontFamily
                Layout.fillHeight: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                Layout.leftMargin: 8
            }

            Text {
                text: "Mod Shop"
                color: ColorPalette.neutral100
                opacity: 0.5
                font.pixelSize: 24
                font.weight: Font.Normal
                font.family: Constants.regularFontFamily
                Layout.fillHeight: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                Layout.leftMargin: 8
            }


            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
            }

            // FirelightButton {
            //     id: searchButton
            //
            //     enabled: false
            //     tooltipLabel: "Search"
            //     flat: true
            //
            //     Layout.fillHeight: true
            //     Layout.preferredWidth: height
            //     KeyNavigation.left: hamburger
            //
            //     iconCode: "\ue8b6"
            // }
            //
            // FirelightButton {
            //     id: notificationButton
            //
            //     enabled: false
            //     tooltipLabel: "Notifications"
            //     flat: true
            //
            //     Layout.fillHeight: true
            //     Layout.preferredWidth: height
            //     KeyNavigation.left: searchButton
            //
            //     iconCode: "\ue7f4"
            // }

            Text {
                property bool drawColon: true
                Timer {
                    triggeredOnStart: true
                    interval: 1000
                    running: true
                    repeat: true
                    onTriggered: {
                        parent.text = new Date().toLocaleTimeString(Qt.locale("en_US"), Locale.ShortFormat)
                        if (!parent.drawColon) {
                            parent.text = parent.text.replace(":", " ")
                            parent.drawColon = true
                        } else {
                            parent.drawColon = false
                        }
                    }
                    // console.log("tick")
                }
                text: ""
                color: "white"
                font.pixelSize: 18
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
                Layout.rightMargin: 12
                font.weight: Font.Normal
                font.family: Constants.regularFontFamily
                Layout.preferredWidth: 85
                Layout.fillHeight: true
            }
        }
    }

    StackView {
        id: contentStack
        anchors.top: headerBar.bottom
        // anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.leftMargin: 40
        anchors.rightMargin: 40
        KeyNavigation.left: hamburger

        background: Item {
        }

        focus: true

        //
        objectName: "Home Content Stack View"
        // anchors.fill: parent

        property alias currentPageName: contentStack.topLevelName

        function goTo(page) {
            contentStack.replace(null, page)
            forceActiveFocus()
        }

        property string topLevelName: ""

        onCurrentItemChanged: {
            if (currentItem) {
                let top = contentStack.find(function (item, index) {
                    return item.topLevel === true
                })

                contentStack.topLevelName = top ? top.topLevelName : ""
            }
        }

        initialItem: root.libraryPage

        pushEnter: Transition {
        }
        pushExit: Transition {
        }
        popEnter: Transition {
        }
        popExit: Transition {
        }
        replaceEnter: Transition {
        }
        replaceExit: Transition {
        }
    }

}