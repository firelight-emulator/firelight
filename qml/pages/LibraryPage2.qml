import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import QtMultimedia

FocusScope {
    id: page

    property alias model: listView.model

    signal playButtonClicked(entryId: int)

    signal openDetails(entryId: int)

    signal launchAnimationFinished()

    function startLoadingGame(entryId) {
        startGameAnimation.gameId = entryId
        startGameAnimation.restart()
    }

    function playLaunchAnimation() {
        startGameAnimation.restart()
    }

    SequentialAnimation {
        id: startGameAnimation
        running: false

        PropertyAction {
            target: playClone
            property: "opacity"
            value: 0.3
        }

        ParallelAnimation {
            PropertyAction {
                target: playClone
                property: "visible"
                value: true
            }
            NumberAnimation {
                target: playClone
                property: "scale"
                from: 1.0
                to: 1.4
                duration: 500
                easing.type: Easing.OutQuad
            }
            ScriptAction {
                script: {
                    sfx_player.play("startgame")
                }
            }
        }
        PropertyAnimation {
            target: playClone
            property: "opacity"
            from: 0.3
            to: 0
            duration: 100
            easing.type: Easing.InQuint
        }
        PropertyAction {
            target: playClone
            property: "visible"
            value: false
        }
        // PauseAnimation {
        //     duration: 500
        // }

        ScriptAction {
            script: {
                page.launchAnimationFinished()
            }
        }
    }

    Pane {
        id: details

        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: listViewContainer.right

        padding: 72

        background: Item {
        }

        contentItem: ColumnLayout {
            spacing: 12
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
            Text {
                Layout.fillWidth: true
                text: listView.currentItem.model.displayName
                font.pixelSize: 24
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                font.weight: Font.DemiBold
                font.family: Constants.regularFontFamily
                color: "white"
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            FirelightButton {
                id: hamburger
                focus: true
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                Layout.preferredWidth: AppStyle.buttonStandardWidth
                Layout.preferredHeight: AppStyle.buttonStandardHeight

                label: "Play"

                onClicked: function () {
                    playButtonClicked(listView.currentItem.model.id)
                    console.log(startGameAnimation)
                    // GameLoader.loadEntry(listView.currentItem.model.id)
                    // page.startGame(listView.currentItem.model.id, listView.currentItem.model.contentHash)
                    // startGameAnimation.gameId = listView.currentItem.model.id
                    // // startGameAnimation.gameHash = listView.currentItem.model.contentHash
                    // startGameAnimation.restart()
                }

                FirelightButton {
                    id: playClone
                    opacity: 0.3
                    visible: false
                    anchors.fill: parent

                    label: "Play"
                }
            }
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

        }
    }

    Pane {
        id: listViewContainer
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 400
        clip: true
        focus: true
        background: Item {
        }
        KeyNavigation.right: details
        contentItem: ListView {
            id: listView
            objectName: "GridView"
            focus: true
            // anchors.fill: parent
            // width: parent.width * 0.8
            // anchors.horizontalCenter: parent.horizontalCenter
            // height: parent.height
            //

            // SplitView.preferredWidth: parent.width / 2

            ScrollBar.vertical: ScrollBar {
            }

            preferredHighlightBegin: 0.33 * listView.height
            preferredHighlightEnd: 0.66 * listView.height
            highlightRangeMode: InputMethodManager.usingMouse ? GridView.NoHighlightRange : GridView.ApplyRange

            contentY: 0

            highlightMoveDuration: 10
            highlightMoveVelocity: -1

            boundsBehavior: Flickable.StopAtBounds

            delegate: FocusScope {
                id: scope
                required property var index
                required property var model
                width: ListView.view.width
                visible: !model.hidden
                height: 50
                focus: true
                Button {
                    id: thingg
                    anchors.fill: parent
                    focus: true

                    property var platformTextColor: ColorPalette.neutral400

                    property bool showGlobalCursor: true

                    TapHandler {
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        onTapped: function (event, button) {
                            if (button === Qt.RightButton) {
                                rightClickMenu.popup()
                                event.accepted = true
                            } else if (button === Qt.LeftButton) {
                                // Router.navigateTo("library/" + myDelegate.model.id + "/details")
                                // myDelegate.openDetails(myDelegate.model.id)
                                // page.startGame(myDelegate.model.id, myDelegate.model.contentHash)
                                listView.currentIndex = scope.index
                            }
                        }
                    }

                    onClicked: {
                        listView.currentIndex = scope.index
                        if (!InputMethodManager.usingMouse) {
                            details.forceActiveFocus()
                        }
                    }

                    RightClickMenu {
                        id: rightClickMenu
                        objectName: "rightClickMenu"

                        RightClickMenuItem {
                            text: "Play " + scope.model.displayName
                            onTriggered: {
                                startGameAnimation.gameId = scope.model.id
                                // startGameAnimation.gameHash = listView.currentItem.model.contentHash
                                startGameAnimation.start()
                                // GameLoader.loadEntry(scope.model.id)
                                // page.startGame(scope.model.id, scope.model.contentHash)
                            }
                        }

                        RightClickMenuItem {
                            enabled: false
                            text: "View details"
                            onTriggered: {
                                page.openDetails(scope.model.id)
                            }
                        }

                        MenuSeparator {
                            contentItem: Rectangle {
                                implicitWidth: rightClickMenu.width
                                implicitHeight: 1
                                color: "#606060"
                            }
                        }

                        RightClickMenuItem {
                            enabled: false
                            text: "Manage save data"
                            onTriggered: {
                                page.manageSaveData(scope.model.id)
                            }
                        }
                    }

                    background: Rectangle {
                        color: "white"
                        opacity: parent.pressed ? 0.1 : parent.hovered ? 0.2 : 0
                    }

                    hoverEnabled: true

                    contentItem: RowLayout {
                        spacing: 12
                        Text {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            // Layout.preferredWidth: 500
                            // Layout.minimumWidth: 300
                            text: model.displayName
                            color: "white"
                            elide: Text.ElideRight
                            maximumLineCount: 1
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignLeft
                            font.pixelSize: 16
                            font.weight: Font.Normal
                            font.family: Constants.regularFontFamily
                        }
                    }
                }
            }
        }
    }
}

