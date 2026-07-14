import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Window
import QtNetwork
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
            color: Theme.surface
        }
        contentItem: Text {
            text: "Updating library..."
            color: Theme.textPrimary
            font.pixelSize: AppStyle.fontSizeMedium
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

            Button {
                id: libraryButton
                KeyNavigation.right: modShopButton
                KeyNavigation.left: hamburger
                Layout.leftMargin: 8
                property bool showGlobalCursor: true
                Layout.fillHeight: true
                focus: true
                checkable: true
                autoExclusive: true
                checked: true
                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }
                background: Rectangle {
                    color: "transparent"
                    radius: 2
                }
                contentItem: Text {
                    text: "Library"
                    color: Theme.textPrimary
                    opacity: libraryButton.checked ? 1 : 0.5
                    font.pixelSize: AppStyle.fontSizeLarge
                    font.weight: Font.Normal
                    font.family: Constants.regularFontFamily
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onToggled: function () {
                    contentStack.replaceCurrentItem(root.libraryPage, {}, StackView.ReplaceTransition)
                }
            }

            Button {
                id: modShopButton
                Layout.leftMargin: 8
                Layout.fillHeight: true
                property bool showGlobalCursor: true
                checkable: true
                autoExclusive: true
                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }
                background: Rectangle {
                    color: "transparent"
                    radius: 2
                }
                contentItem: Text {
                    text: "Mod Shop"
                    color: Theme.textPrimary
                    opacity: modShopButton.checked ? 1 : 0.5
                    font.pixelSize: AppStyle.fontSizeLarge
                    font.weight: Font.Normal
                    font.family: Constants.regularFontFamily
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onToggled: function () {
                    contentStack.replaceCurrentItem(root.modShopPage, {}, StackView.ReplaceTransition)
                }
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

            Icon {
                name: "wifi"
                visible: NetworkInformation.reachability === NetworkInformation.Reachability.Online
                Layout.fillHeight: true
                topPadding: 1
                size: AppStyle.fontSizeLarge
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                color: AppStyle.buttonTextColorInactive
            }

            Icon {
                name: "wifi_off"
                visible: NetworkInformation.reachability === NetworkInformation.Reachability.Disconnected || NetworkInformation.reachability === NetworkInformation.Reachability.Unknown
                Layout.fillHeight: true
                topPadding: 1
                size: AppStyle.fontSizeLarge
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                color: AppStyle.buttonTextColorInactive
            }

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
                color: Theme.textPrimary
                font.pixelSize: AppStyle.fontSizeMedium
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
            NumberAnimation {
                property: "opacity"
                from: 0.0
                to: 1.0
                duration: 250
                easing.type: Easing.InOutQuad
            }
            // NumberAnimation {
            //     property: "x"
            //     from: 30 * (root.movingRight ? 1 : -1)
            //     to: 0
            //     duration: 200
            //     easing.type: Easing.InOutQuad
            // }
        }
        replaceExit: Transition {
            NumberAnimation {
                property: "opacity"
                from: 1.0
                to: 0.0
                duration: 100
                easing.type: Easing.InOutQuad
            }
            // NumberAnimation {
            //     property: "x"
            //     from: 0;
            //     to: 30 * (root.movingRight ? -1 : 1)
            //     duration: 200
            //     easing.type: Easing.InOutQuad
            // }
        }
    }

}