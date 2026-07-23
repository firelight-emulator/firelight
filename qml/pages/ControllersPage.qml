import QtQuick
import QtQuick.Controls
import QtQuick.Shapes 1.8
import QtQuick.Layouts
import Firelight 1.0

FocusScope {
    id: root

    property bool keyboardDragging: false
    property bool isDragging: false

    property GamepadListModel gamepads: GamepadListModel {}

    Component {
        id: dragDelegate

        FocusScope {
            id: content
            required property var model
            required property var index
            width: 200
            height: 200

            Button {
                id: myButton
                anchors.fill: parent
                KeyNavigation.down: root.keyboardDragging ? myButton : editProfileButton.visible ? editProfileButton : null

                z: -1
                focus: true
                property bool showGlobalCursor: true

                Keys.onPressed: function (event) {
                    if (event.key === Qt.Key_Select) {
                        if (root.isDragging) {
                            return;
                        }

                        if (root.keyboardDragging) {
                            root.keyboardDragging = false;
                            let list = {};
                            for (let i = 0; i < visualModel.items.count; i++) {
                                // console.log(visualModel.items.get(i).model.index)
                                list[i] = visualModel.items.get(i).model.index;
                                // visualModel.items.get(i).model.itemsIndex = i
                            }

                            // theAnimation.thingy = list
                            // theAnimation.start()

                            root.gamepads.changeGamepadOrder(list);
                            // visualModel.items.sort()
                        } else if (!root.keyboardDragging) {
                            root.keyboardDragging = true;
                        }
                        event.accept = true;
                    }
                    if (event.key === Qt.Key_Right) {
                        if (root.keyboardDragging && listThing.currentIndex < 3) {
                            visualModel.items.move(listThing.currentIndex, listThing.currentIndex + 1, 1);
                            event.accept = true;
                        }
                    }
                    if (event.key === Qt.Key_Left) {
                        if (root.keyboardDragging && listThing.currentIndex > 0) {
                            visualModel.items.move(listThing.currentIndex, listThing.currentIndex - 1, 1);
                            event.accept = true;
                        }
                    }
                }

                background: Rectangle {
                    opacity: 0.9
                    color: Theme.surface
                    radius: 4
                }
            }

            Shape {
                id: leftArrow
                visible: root.keyboardDragging && listThing.currentIndex > 0 && content.ListView.isCurrentItem
                width: 20
                height: 40
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.left
                anchors.rightMargin: 18

                ShapePath {
                    startX: leftArrow.width
                    startY: 0
                    fillColor: "white"
                    joinStyle: ShapePath.RoundJoin
                    capStyle: ShapePath.RoundCap
                    strokeWidth: 2
                    PathLine {
                        x: 0
                        y: leftArrow.height / 2
                    }
                    PathLine {
                        x: leftArrow.width
                        y: leftArrow.height
                    }
                    PathLine {
                        x: leftArrow.width
                        y: 0
                    }
                }
            }

            Shape {
                id: rightArrow
                visible: root.keyboardDragging && listThing.currentIndex < 3 && content.ListView.isCurrentItem
                width: 20
                height: 40
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.right
                anchors.leftMargin: 18

                ShapePath {
                    startX: 0
                    startY: 0
                    fillColor: "white"
                    joinStyle: ShapePath.RoundJoin
                    capStyle: ShapePath.RoundCap
                    strokeWidth: 2
                    PathLine {
                        x: 0
                        y: rightArrow.height
                    }
                    PathLine {
                        x: rightArrow.width
                        y: rightArrow.height / 2
                    }
                    PathLine {
                        x: 0
                        y: 0
                    }
                }
            }

            z: root.keyboardDragging && content.ListView.isCurrentItem || dragArea.active ? 1 : 0

            property point dragEnd: Qt.point(0, 0)

            Drag.active: dragArea.active
            Drag.source: content
            Drag.hotSpot.x: width / 2
            Drag.hotSpot.y: height / 2
            Drag.keys: ["controller-reorder"]

            Image {
                id: controllerIcon
                source: model.image_url
                visible: model.connected
                fillMode: Image.PreserveAspectFit
                width: 182
                sourceSize.width: 182
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: "No controller connected"
                color: Theme.textMuted
                font.pixelSize: AppStyle.fontSizeSmall
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.family: AppStyle.fontFamily
                visible: !model.connected
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }

            FirelightButton {
                id: editProfileButton
                anchors.top: parent.bottom
                anchors.topMargin: 16
                visible: !root.keyboardDragging && !root.isDragging && model.connected
                enabled: visible
                focus: true

                label: "Edit profile"

                onClicked: function () {
                    Router.navigate("/settings/controllers?profileId=" + encodeURIComponent(content.model.profile_id));
                }
            }

            Text {
                id: playerNum
                text: (content.index + 1)
                color: "white"
                font.pixelSize: AppStyle.fontSizeSmall
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                anchors.top: parent.top
                anchors.topMargin: 8
                anchors.left: parent.left
                anchors.leftMargin: 8
                width: 16
                height: 16
            }

            NumberAnimation {
                id: moveAnimation
                target: content
            }

            DragHandler {
                id: dragArea

                grabPermissions: PointerHandler.TakeOverForbidden

                yAxis.enabled: false
                xAxis.enabled: true
                xAxis.maximum: 660
                xAxis.minimum: -30

                onActiveChanged: {
                    root.isDragging = dragArea.active;
                    if (dragArea.active) {
                        content.dragEnd = Qt.point(content.x, content.y);
                        root.keyboardDragging = false;
                    }

                    if (!dragArea.active) {
                        let list = {};
                        for (let i = 0; i < visualModel.items.count; i++) {
                            // console.log(visualModel.items.get(i).model.index)
                            list[i] = visualModel.items.get(i).model.index;
                            // visualModel.items.get(i).model.itemsIndex = i
                        }

                        // theAnimation.thingy = list
                        // theAnimation.start()

                        root.gamepads.changeGamepadOrder(list);
                        // visualModel.items.sort()
                    }
                }
            }

            DropArea {
                width: parent.width / 2 - 2
                height: parent.height
                anchors.left: parent.left

                onEntered: drag => {
                    drag.source.dragEnd = Qt.point(content.x, content.y);

                    const mine = content.DelegateModel.itemsIndex;
                    const theirs = drag.source.DelegateModel.itemsIndex;
                    if (mine < theirs) {
                        visualModel.items.move(theirs, mine);
                    }
                }
            }

            DropArea {
                width: parent.width / 2 - 2
                height: parent.height
                anchors.right: parent.right

                onEntered: drag => {
                    drag.source.dragEnd = Qt.point(content.x, content.y);

                    const mine = content.DelegateModel.itemsIndex;
                    const theirs = drag.source.DelegateModel.itemsIndex;
                    if (mine > theirs) {
                        visualModel.items.move(theirs, mine);
                    }
                }
            }
        }
    }

    DelegateModel {
        id: visualModel

        model: root.gamepads
        delegate: dragDelegate
    }

    RowLayout {
        id: mainThing
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

            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 60
                Layout.maximumHeight: 60

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                FirelightButton {
                    id: manageProfilesButton
                    label: "Manage Profiles"
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

                    onClicked: function () {
                        Router.navigate("/controllers/manage");
                    }
                }
            }

            // Steam reads gamepads globally through its own driver, so its Guide
            // shortcut fires even for games it didn't launch. We can't pre-empt
            // it from in here — say so rather than let it look like our bug
            // Checked when the page opens; Steam may well have started since
            Rectangle {
                id: steamWarning
                property bool steamRunning: false

                Component.onCompleted: steamRunning = InputService.isSteamRunning()

                visible: steamRunning
                Layout.fillWidth: true
                Layout.bottomMargin: 8
                implicitHeight: steamWarningRow.implicitHeight + 20
                radius: 8
                color: Theme.surface
                border.width: 1
                border.color: Theme.border

                RowLayout {
                    id: steamWarningRow
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10

                    Icon {
                        name: "info"
                        size: 20
                        color: Theme.textMuted
                        Layout.alignment: Qt.AlignTop
                    }

                    Text {
                        Layout.fillWidth: true
                        text: "Steam is running, and can take the Guide button before Firelight sees it. " + "If that's a problem, turn off \"Guide button focuses Steam\" in Steam's " + "Settings → Controller. Firelight doesn't bind Guide to anything."
                        color: Theme.textMuted
                        font.pixelSize: AppStyle.fontSizeSmall
                        font.family: AppStyle.fontFamily
                        wrapMode: Text.WordWrap
                    }
                }
            }

            ListView {
                id: listThing

                focus: true
                keyNavigationEnabled: true
                Layout.fillWidth: true
                // Layout.preferredWidth: 500
                Layout.alignment: Qt.AlignHCenter
                Layout.fillHeight: true

                interactive: false
                orientation: ListView.Horizontal

                moveDisplaced: Transition {
                    NumberAnimation {
                        properties: "x,y"
                        duration: 100
                        easing.type: Easing.InOutQuad
                    }
                }

                move: Transition {
                    enabled: root.keyboardDragging
                    NumberAnimation {
                        properties: "x,y"
                        duration: 100
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
}
