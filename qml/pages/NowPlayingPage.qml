import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


FocusScope {
    id: root

    required property int entryId
    property int saveSlot: 1
    required property string contentHash
    required property bool undoEnabled

    signal resumeGamePressed()

    signal restartGamePressed()

    signal closeGamePressed()

    signal rewindPressed()

    signal writeSuspendPoint(index: int)

    signal loadSuspendPoint(index: int)

    signal loadLastSuspendPoint()

    property Item previouslyFocusedItem

    StackView.onDeactivated: function () {
        SaveManager.clearSuspendPointListModel()
    }


    Item {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 40
    }

    RowLayout {
        id: contentRow
        // anchors.top: navBar.bottom
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        spacing: 0

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.horizontalStretchFactor: 1
        }

        ButtonGroup {
            id: navButtonGroup
        }

        ColumnLayout {
            id: col
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.horizontalStretchFactor: 2

            property int index: 0

            onIndexChanged: function () {
                if (!InputMethodManager.usingMouse) {
                    sfx_player.play("menumove")
                }
            }

            FirelightMenuItem {
                id: resumeGameButton
                labelText: "Resume Game"
                focus: true
                Layout.fillWidth: true
                KeyNavigation.down: restartGameButton
                // Layout.preferredWidth: parent.width / 2
                Layout.preferredHeight: 40
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                checkable: false
                alignRight: true

                onActiveFocusChanged: function () {
                    if (activeFocus) {
                        col.index = 0
                    }
                }

                onClicked: function () {
                    if (rightSide.depth > 0) {
                        rightSide.pop()
                        root.previouslyFocusedItem = null
                    }
                    resumeGamePressed()
                }
            }
            FirelightMenuItem {
                id: restartGameButton
                labelText: "Restart Game"
                Layout.fillWidth: true
                KeyNavigation.down: rewindButton
                // Layout.preferredWidth: parent.width / 2
                Layout.preferredHeight: 40
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                checkable: false
                alignRight: true

                onActiveFocusChanged: function () {
                    if (activeFocus) {
                        col.index = 1
                    }
                }

                onClicked: function () {
                    if (rightSide.depth > 0) {
                        rightSide.pop()
                        root.previouslyFocusedItem = null
                    }
                    restartGamePressed()
                }
            }
            FirelightMenuItem {
                id: rewindButton
                labelText: "Rewind"
                KeyNavigation.down: loadSuspendPointButton
                Layout.fillWidth: true
                // Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 40
                enabled: false
                checkable: false
                alignRight: true


                onActiveFocusChanged: function () {
                    if (activeFocus) {
                        col.index = 2
                    }
                }
                onClicked: function () {
                    if (rightSide.depth > 0) {
                        rightSide.pop()
                        root.previouslyFocusedItem = null
                    }
                    rewindPressed()
                }
            }
            Rectangle {
                Layout.fillWidth: true
                // Layout.preferredWidth: parent.width / 2
                Layout.preferredHeight: 1
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                opacity: 0.3
                color: "#dadada"
            }
            FirelightMenuItem {
                id: loadSuspendPointButton
                labelText: "Load Suspend Point"
                Layout.fillWidth: true
                KeyNavigation.down: createSuspendPointButton
                // Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 40
                checkable: false
                enabled: true
                alignRight: true
                pseudoChildFocused: root.previouslyFocusedItem === loadSuspendPointButton

                ButtonGroup.group: navButtonGroup


                onActiveFocusChanged: function () {
                    if (activeFocus) {
                        col.index = 3
                    }
                }

                Keys.onRightPressed: function (event) {
                    if (rightSide.depth === 0) {
                        previouslyFocusedItem = loadSuspendPointButton
                        rightSide.replaceCurrentItem(suspendPoints, {creating: false}, StackView.PushTransition)
                        rightSide.forceActiveFocus()

                        event.accepted = true
                    }
                }


                Component {
                    id: loadSuspendPointLoader
                    Loader {
                        asynchronous: true
                        sourceComponent: suspendPoints
                    }
                }

                onClicked: function () {
                    if (rightSide.depth === 0) {
                        previouslyFocusedItem = loadSuspendPointButton
                        rightSide.replaceCurrentItem(suspendPoints, {creating: false}, StackView.PushTransition)
                        rightSide.forceActiveFocus()
                    } else if (root.previouslyFocusedItem === createSuspendPointButton) {
                        root.previouslyFocusedItem = loadSuspendPointButton
                        rightSide.currentItem.creating = false
                        rightSide.forceActiveFocus()
                    }
                }
            }
            FirelightMenuItem {
                id: createSuspendPointButton
                KeyNavigation.down: undo
                labelText: "Create Suspend Point"
                Layout.fillWidth: true
                // Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 40
                checkable: false
                enabled: true
                alignRight: true
                pseudoChildFocused: root.previouslyFocusedItem === createSuspendPointButton

                ButtonGroup.group: navButtonGroup

                onActiveFocusChanged: function () {
                    if (activeFocus) {
                        col.index = 4
                    }
                }

                Keys.onRightPressed: function (event) {
                    if (rightSide.depth === 0) {
                        previouslyFocusedItem = createSuspendPointButton
                        rightSide.replaceCurrentItem(suspendPoints, {creating: true}, StackView.PushTransition)
                        rightSide.forceActiveFocus()

                        event.accepted = true
                    }
                }

                Component {
                    id: createSuspendPointLoader
                    Loader {
                        asynchronous: true
                        sourceComponent: suspendPoints
                    }
                }

                onClicked: function () {
                    if (rightSide.depth === 0) {
                        previouslyFocusedItem = createSuspendPointButton
                        rightSide.replaceCurrentItem(suspendPoints, {creating: true}, StackView.PushTransition)
                        rightSide.forceActiveFocus()
                    } else if (root.previouslyFocusedItem === loadSuspendPointButton) {
                        root.previouslyFocusedItem = createSuspendPointButton
                        rightSide.currentItem.creating = true
                        rightSide.forceActiveFocus()
                    }
                }
            }
            FirelightMenuItem {
                id: undo
                labelText: "Undo Last Load"
                Layout.fillWidth: true
                KeyNavigation.down: closeGameButton
                // Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 40
                checkable: false
                enabled: root.undoEnabled
                alignRight: true

                onClicked: function () {
                    if (rightSide.depth > 0) {
                        rightSide.popCurrentItem(StackView.PopTransition)
                        root.previouslyFocusedItem = null
                    }
                    if (navButtonGroup.checkedButton) {
                        navButtonGroup.checkedButton.checked = false
                    }
                    root.loadLastSuspendPoint()
                }

                onActiveFocusChanged: function () {
                    if (activeFocus) {
                        col.index = 5
                    }
                }
            }
            Rectangle {
                Layout.fillWidth: true
                // Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 1
                opacity: 0.3
                color: "#dadada"
            }
            FirelightMenuItem {
                id: closeGameButton
                labelText: "Close Game"
                Layout.fillWidth: true
                // Layout.preferredWidth: parent.width / 2
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                Layout.preferredHeight: 40
                checkable: false
                alignRight: true

                onClicked: function () {
                    if (rightSide.depth > 0) {
                        rightSide.pop()
                        root.previouslyFocusedItem = null
                    }
                    closeGameConfirmationDialog.open()
                }

                onActiveFocusChanged: function () {
                    if (activeFocus) {
                        col.index = 7
                    }
                }
            }
        }

        StackView {
            id: rightSide
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.horizontalStretchFactor: 3

            Keys.onBackPressed: function (event) {
                if (rightSide.depth > 0) {
                    rightSide.popCurrentItem()
                    root.previouslyFocusedItem.forceActiveFocus()
                    root.previouslyFocusedItem = null

                    event.accepted = true
                }
            }

            Keys.onLeftPressed: function (event) {
                if (rightSide.depth > 0) {
                    rightSide.popCurrentItem()
                    root.previouslyFocusedItem.forceActiveFocus()
                    root.previouslyFocusedItem = null

                    event.accepted = true
                }
            }

            pushEnter: Transition {
                NumberAnimation {
                    property: "opacity"
                    from: 0.0
                    to: 1.0
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    property: "x"
                    from: 30
                    to: 0
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
            }

            popExit: Transition {
                NumberAnimation {
                    property: "opacity"
                    from: 1.0
                    to: 0.0
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
                NumberAnimation {
                    property: "x"
                    from: 0;
                    to: 30
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.horizontalStretchFactor: 1
        }
    }

    Component {
        id: suspendPoints
        FocusScope {
            property bool creating: false
            ListView {
                id: suspendPointList
                property bool creating: parent.creating
                property bool loading: !creating

                width: Math.min(800, parent.width - 128)
                focus: true
                // height: parent.height
                clip: true
                height: Math.min(parent.height * .75, contentHeight)
                // topMargin: contentHeight < parent.height ? 0 : 100
                // bottomMargin: contentHeight < parent.height ? 0 : 100
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                currentIndex: 0
                highlightMoveDuration: 80
                highlightMoveVelocity: -1
                keyNavigationEnabled: true
                // highlightRangeMode: InputMethodManager.usingMouse ? ListView.ApplyRange : ListView.StrictlyEnforceRange
                highlightRangeMode: ListView.ApplyRange
                preferredHighlightBegin: height / 3
                preferredHighlightEnd: height * 2 / 3
                // highlight: Rectangle {
                //     border.color: ColorPalette.neutral100
                //     radius: 6
                //     color: "transparent"
                // }
                highlight: Item {
                }
                // model: 8
                model: SaveManager.getSuspendPointListModel(root.contentHash, root.saveSlot)
                spacing: 8
                boundsBehavior: Flickable.StopAtBounds

                header: Pane {
                    width: ListView.view.width
                    padding: 12
                    bottomInset: 8

                    HoverHandler {
                        id: headerHover
                    }

                    background: Rectangle {
                        color: headerHover.hovered ? ColorPalette.neutral800 : "transparent"
                        radius: 4
                        Behavior on color {
                            ColorAnimation {
                                duration: 200
                                easing.type: Easing.InOutQuad
                            }
                        }

                    }

                    contentItem: Text {
                        text: "Remember, Suspend Points aren't a reliable way to save your progress - make sure to save in-game frequently!"
                        font.pixelSize: 14
                        font.family: Constants.regularFontFamily
                        font.weight: Font.DemiBold
                        horizontalAlignment: Text.AlignHCenter
                        lineHeight: 1.2
                        bottomPadding: 12
                        verticalAlignment: Text.AlignVCenter
                        color: headerHover.hovered ? ColorPalette.neutral100 : ColorPalette.neutral400
                        wrapMode: Text.WordWrap

                        Behavior on color {
                            ColorAnimation {
                                duration: 200
                                easing.type: Easing.InOutQuad
                            }
                        }

                    }
                }

                delegate: Component {
                    FocusScope {
                        id: dele
                        required property var model
                        required property var index
                        width: ListView.view.width
                        height: 120
                        Button {
                            id: deleButton

                            property bool showGlobalCursor: true

                            padding: 0
                            focus: true
                            width: parent.width - 2
                            height: parent.height - 2
                            x: 1
                            y: 1
                            hoverEnabled: true
                            background: Rectangle {
                                // gradient: Gradient {
                                //     GradientStop {
                                //         position: 0.0; color: ColorPalette.neutral700
                                //     }
                                //     GradientStop {
                                //         position: 1.0; color: ColorPalette.neutral900
                                //     }
                                // }
                                color: deleButton.hovered && !details.hovered ? ColorPalette.neutral800 : ColorPalette.neutral900
                                // border.color: ColorPalette.neutral700
                                // color: ColorPalette.neutral900
                                border.color: ColorPalette.neutral700
                                radius: 6
                            }

                            RightClickMenu {
                                id: rightClickMenu
                                // RightClickMenuItem {
                                //     text: "Edit"
                                // }
                                RightClickMenuItem {
                                    text: dele.model.locked ? "Unlock" : "Lock"
                                    onTriggered: function () {
                                        dele.model.locked = !dele.model.locked
                                    }
                                }
                                // RightClickMenuItem {
                                //     text: "Duplicate"
                                // }

                                RightClickMenuSeparator {
                                    width: parent.width
                                }

                                RightClickMenuItem {
                                    text: "Delete"
                                    dangerous: true

                                    onTriggered: function () {
                                        if (dele.model.locked) {
                                            sfx_player.play("nope")
                                            console.log("Can't delete locked Suspend Point")
                                            lockedTooltip.open()
                                            return
                                        }

                                        deleteSuspendPointDialog.doThing = function () {
                                            dele.model.has_data = false
                                        }
                                        deleteSuspendPointDialog.text = "Are you sure you want to delete the Suspend Point in slot " + (dele.index + 1) + "?"
                                        deleteSuspendPointDialog.open()
                                    }
                                }
                            }

                            Keys.onMenuPressed: function (event) {
                                if (dele.model.has_data) {
                                    rightClickMenu.popupModal(width - rightClickMenu.width + 24, 6)
                                    event.accepted = true
                                }
                            }

                            onClicked: function () {
                                if (suspendPointList.loading) {
                                    if (dele.model.has_data) {
                                        loadSuspendPointDialog.index = dele.index
                                        loadSuspendPointDialog.open()
                                    } else {
                                        sfx_player.play("nope")
                                        console.log("Can't load an empty Suspend Point")
                                    }
                                }
                                if (suspendPointList.creating) {
                                    if (dele.model.locked) {
                                        sfx_player.play("nope")
                                        console.log("Can't overwrite locked Suspend Point")
                                        lockedTooltip.open()
                                    } else {
                                        if (dele.model.has_data) {
                                            overwriteSuspendPointDialog.index = dele.index
                                            overwriteSuspendPointDialog.open()
                                        } else {
                                            writeSuspendPoint(dele.index)
                                        }
                                    }
                                }
                            }

                            TapHandler {
                                acceptedButtons: Qt.RightButton
                                onTapped: {
                                    if (dele.model.has_data) {
                                        rightClickMenu.popupNormal()
                                    }
                                }
                            }
                            contentItem: Item {
                                Item {
                                    anchors.fill: parent
                                    visible: !dele.model.has_data
                                    Rectangle {
                                        id: pic2
                                        anchors.top: parent.top
                                        anchors.bottom: parent.bottom
                                        anchors.left: parent.left
                                        anchors.topMargin: 2
                                        anchors.bottomMargin: 2
                                        topLeftRadius: 6
                                        bottomLeftRadius: 6
                                        // anchors.leftMargin: dele.leftPadding
                                        width: parent.height * 16 / 9
                                        color: ColorPalette.neutral1000
                                        // color: ColorPalette.neutral600

                                        Text {
                                            text: "No data"
                                            anchors.centerIn: parent
                                            font.pixelSize: 14
                                            font.family: Constants.regularFontFamily
                                            font.weight: Font.Medium
                                            horizontalAlignment: Text.AlignHCenter
                                            lineHeight: 1.2
                                            verticalAlignment: Text.AlignVCenter
                                            color: ColorPalette.neutral400
                                            wrapMode: Text.WordWrap
                                        }
                                    }
                                    Rectangle {
                                        anchors.top: parent.top
                                        anchors.bottom: parent.bottom
                                        anchors.left: pic2.right
                                        width: 1
                                        color: ColorPalette.neutral700
                                    }
                                    Text {
                                        id: label2
                                        anchors.top: parent.top
                                        anchors.right: parent.right
                                        anchors.left: pic2.right
                                        anchors.leftMargin: 34
                                        anchors.bottom: parent.bottom
                                        font.pixelSize: 16
                                        font.family: Constants.regularFontFamily
                                        font.weight: Font.DemiBold
                                        horizontalAlignment: Text.AlignLeft
                                        lineHeight: 1.2
                                        verticalAlignment: Text.AlignVCenter
                                        color: ColorPalette.neutral400
                                        wrapMode: Text.WordWrap

                                        text: "Slot %1".arg(dele.index + 1)
                                    }
                                }
                                Item {
                                    anchors.fill: parent
                                    visible: dele.model.has_data
                                    DetailsButton {
                                        id: details
                                        anchors.top: parent.top
                                        anchors.right: parent.right
                                        width: 36
                                        height: 36
                                        anchors.topMargin: 8
                                        anchors.rightMargin: 8
                                    }
                                    Rectangle {
                                        id: pic
                                        anchors.top: parent.top
                                        anchors.bottom: parent.bottom
                                        anchors.left: parent.left
                                        anchors.topMargin: 2
                                        anchors.bottomMargin: 2
                                        topLeftRadius: 6
                                        bottomLeftRadius: 6
                                        // anchors.leftMargin: dele.leftPadding
                                        width: parent.height * 16 / 9
                                        color: ColorPalette.neutral1000

                                        Image {
                                            height: parent.height
                                            sourceSize.height: parent.height
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            source: dele.model.image_url
                                            fillMode: Image.PreserveAspectCrop
                                        }
                                        // color: ColorPalette.neutral600
                                    }
                                    Rectangle {
                                        anchors.top: parent.top
                                        anchors.bottom: parent.bottom
                                        anchors.left: pic.right
                                        width: 1
                                        color: ColorPalette.neutral700
                                    }
                                    Text {
                                        font.family: Constants.symbolFontFamily
                                        visible: dele.model.locked
                                        font.pixelSize: 16
                                        height: 24
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                        width: 24
                                        anchors.topMargin: 4
                                        anchors.leftMargin: 4
                                        color: "white"
                                        anchors.left: pic.right
                                        anchors.top: parent.top
                                        text: "\ue897"

                                        HoverHandler {
                                            id: lockHover
                                        }

                                        ToolTip {
                                            text: "This Suspend Point is locked and cannot be overwritten or deleted"
                                            visible: lockHover.hovered
                                            delay: 300
                                        }

                                        ToolTip {
                                            id: lockedTooltip
                                            text: "This Suspend Point is locked and cannot be overwritten or deleted"
                                            visible: false
                                            timeout: 3000
                                            delay: 0
                                        }
                                    }
                                    Text {
                                        id: label
                                        anchors.top: parent.top
                                        anchors.right: parent.right
                                        anchors.left: pic.right
                                        anchors.leftMargin: 24
                                        height: parent.height / 2
                                        topPadding: 8
                                        bottomPadding: 0
                                        leftPadding: 10
                                        rightPadding: 10
                                        font.pixelSize: 16
                                        font.family: Constants.regularFontFamily
                                        font.weight: Font.DemiBold
                                        horizontalAlignment: Text.AlignLeft
                                        lineHeight: 1.2
                                        verticalAlignment: Text.AlignBottom
                                        color: ColorPalette.neutral100
                                        wrapMode: Text.WordWrap

                                        text: "Slot %1".arg(dele.index + 1)
                                    }
                                    Text {
                                        id: date
                                        anchors.top: label.bottom
                                        anchors.right: parent.right
                                        anchors.left: pic.right
                                        anchors.bottom: parent.bottom
                                        anchors.leftMargin: 24
                                        topPadding: 4
                                        bottomPadding: 8
                                        leftPadding: 10
                                        rightPadding: 10
                                        font.pixelSize: 15
                                        font.family: Constants.regularFontFamily
                                        font.weight: Font.DemiBold
                                        horizontalAlignment: Text.AlignLeft
                                        lineHeight: 1.2
                                        verticalAlignment: Text.AlignTop
                                        color: ColorPalette.neutral400
                                        wrapMode: Text.WordWrap

                                        text: dele.model.timestamp
                                    }
                                }


                            }
                            // contentItem: Rectangle {
                            //     color: "red"
                            // }
                        }
                    }

                }
            }
        }

    }

    FirelightDialog {
        id: deleteSuspendPointDialog

        property var doThing: function () {
            console.log("Doing the thing")
        }

        onAccepted: function () {
            doThing()
            doThing = null
        }
    }

    FirelightDialog {
        id: loadSuspendPointDialog
        text: "Load the Suspend Point in slot " + (index + 1) + "?"

        property int index: -1

        onAccepted: function () {
            let theIndex = index
            index = -1
            console.log("Loading suspend point " + theIndex)
            root.loadSuspendPoint(theIndex)
            root.undoEnabled = true
        }
    }

    FirelightDialog {
        id: overwriteSuspendPointDialog
        text: "Overwrite the Suspend Point in slot " + (index + 1) + "?"

        property int index: -1

        onAccepted: function () {
            let theIndex = index
            index = -1
            root.writeSuspendPoint(theIndex)
        }
    }

    FirelightDialog {
        id: closeGameConfirmationDialog
        text: "Are you sure you want to close the game?"

        onAccepted: {
            root.closeGamePressed()
        }
    }



}