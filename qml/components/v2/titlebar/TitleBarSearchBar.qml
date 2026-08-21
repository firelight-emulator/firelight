import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts 1.0

FocusScope {
    id: control

    Keys.onEscapePressed: function (event) {
        if (searchField.activeFocus || searchPopup.activeFocus) {
            if (searchField.text !== "") {
                searchField.text = "";
                searchField.forceActiveFocus();
                event.accepted = true;
            } else {
                control.Window.window.contentItem.forceActiveFocus();
                event.accepted = true;
            }
        }
    }

    Pane {
        anchors.fill: parent
        padding: AppStyle.spacingXs
        topPadding: 3
        focus: true

        background: Rectangle {
            color: searchField.activeFocus || searchPopup.activeFocus ? Theme.glassElevated : Theme.glass
            bottomLeftRadius: height / 2
            bottomRightRadius: height / 2
            topLeftRadius: height / 2
            topRightRadius: height / 2

            Behavior on color {
                ColorAnimation {
                    duration: 240
                    easing.type: Easing.Linear
                }
            }

            Rectangle {
                anchors.fill: parent
                color: "transparent"
                border.color: "white"
                border.width: 2
                radius: height / 2
                opacity: searchField.activeFocus || searchPopup.activeFocus ? 0.8 : 0

                Behavior on opacity {
                    NumberAnimation {
                        duration: 240
                        easing.type: Easing.Linear
                    }
                }
            }
        }

        contentItem: RowLayout {
            Button {
                Layout.fillHeight: true
                Layout.preferredWidth: height
                Layout.topMargin: AppStyle.spacingXs
                Layout.margins: 3
                focusPolicy: Qt.NoFocus

                padding: 0

                background: Rectangle {
                    color: "transparent"
                }

                HoverHandler {
                    id: magnifyingGlassHover
                    cursorShape: Qt.PointingHandCursor
                }

                Image {
                    id: searchIcon
                    anchors.fill: parent
                    source: "qrc:/icons/search"
                    sourceSize.width: 32
                    sourceSize.height: 32
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    opacity: searchField.activeFocus || searchPopup.activeFocus || magnifyingGlassHover.hovered ? 0.9 : 0.5

                    Behavior on opacity {
                        NumberAnimation {
                            duration: 240
                            easing.type: Easing.InOutQuad
                        }
                    }
                }

                onClicked: {
                    searchField.forceActiveFocus(Qt.MouseFocusReason);
                }
            }

            TextField {
                id: searchField
                leftPadding: 0
                focus: true

                Layout.fillHeight: true
                Layout.fillWidth: true

                Keys.onDownPressed: {
                    if (searchPopup.visible) {
                        // searchResultsList.currentIndex = 0
                        searchResultsList.headerItem.forceActiveFocus();
                    }
                }

                placeholderText: qsTr("Search (Ctrl + F)")
                font.pixelSize: AppStyle.fontSizeMedium
                color: Theme.textPrimary
                font.family: AppStyle.fontFamily
                background: Rectangle {
                    color: "transparent"
                }

                onTextChanged: {
                    textChangedAnimation.start();
                }
            }

            SequentialAnimation {
                id: textChangedAnimation

                running: false
                PauseAnimation {
                    duration: 240
                }
                ScriptAction {
                    script: {
                        SearchResultsModel.setFilterString(searchField.text);
                    }
                }
            }

            Button {
                Layout.fillHeight: true
                Layout.preferredWidth: height
                Layout.margins: 5
                focusPolicy: Qt.NoFocus

                padding: 0
                visible: searchField.text !== ""

                background: Rectangle {
                    color: "transparent"
                }

                HoverHandler {
                    id: hover
                    cursorShape: Qt.PointingHandCursor
                }

                Image {
                    id: cancelButton
                    anchors.fill: parent
                    source: "qrc:/icons/cancel"
                    sourceSize.width: 32
                    sourceSize.height: 32
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    opacity: parent.down ? 0.4 : hover.hovered ? 0.7 : 0.5
                }

                onClicked: {
                    searchField.text = "";
                }
            }
        }
    }

    Popup {
        id: searchPopup

        y: control.height + 8
        width: parent.width
        height: 399
        padding: 0

        onClosed: {
            searchResultsList.contentY = 0;
        }

        closePolicy: Popup.NoAutoClose

        visible: searchField.activeFocus || searchPopup.activeFocus

        background: Rectangle {
            color: Theme.surfaceElevated
            radius: AppStyle.radiusMd
            border.color: Theme.border
            border.width: 1

            layer.enabled: true
            layer.effect: MultiEffect {
                shadowEnabled: true
                shadowColor: Theme.shadow
                shadowBlur: AppStyle.elevationBlur
                shadowVerticalOffset: AppStyle.elevationOffset
                shadowHorizontalOffset: AppStyle.elevationOffset
            }
        }

        contentItem: FocusScope {
            anchors.fill: parent
            anchors.bottomMargin: 2
            clip: true
            Text {
                text: qsTr("Type something to search!")
                font.pixelSize: AppStyle.fontSizeMedium
                font.family: AppStyle.fontFamily
                font.weight: Font.DemiBold
                color: Theme.textMuted
                anchors.centerIn: parent
                visible: searchField.text === ""
            }
            ListView {
                id: searchResultsList
                visible: searchField.text !== ""
                focus: true
                anchors.fill: parent
                anchors.leftMargin: AppStyle.spacingSm
                anchors.rightMargin: AppStyle.spacingLg
                model: SearchResultsModel
                boundsBehavior: Flickable.StopAtBounds
                header: Pane {
                    id: searchResultsListHeader
                    width: ListView.view.width + 16
                    height: 48
                    background: Item {}
                    RowLayout {
                        anchors.fill: parent

                        Text {
                            text: "Search results"
                            font.pixelSize: AppStyle.fontSizeMedium
                            font.family: AppStyle.fontFamily
                            font.weight: Font.DemiBold
                            color: Theme.textMuted
                            Layout.fillHeight: true
                        }
                        Text {
                            id: countText
                            text: "(" + SearchResultsModel.rowCount() + ")"
                            font.pixelSize: AppStyle.fontSizeMedium
                            font.family: AppStyle.fontFamily
                            font.weight: Font.Medium
                            color: Theme.textMuted
                            Layout.fillHeight: true

                            Connections {
                                target: SearchResultsModel
                                function onCountChanged() {
                                    countText.text = "(" + SearchResultsModel.rowCount() + ")";
                                }
                            }
                        }
                        Item {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                        }

                        Button {
                            property bool showGlobalCursor: true
                            focus: true

                            Layout.fillHeight: true
                            Layout.topMargin: -4
                            Layout.bottomMargin: -4
                            Layout.rightMargin: AppStyle.spacingXs
                            HoverHandler {
                                id: seeAllHover
                                cursorShape: Qt.PointingHandCursor
                            }
                            background: Rectangle {
                                color: "white"
                                opacity: seeAllHover.hovered ? 0.08 : 0
                                radius: AppStyle.radiusSm
                            }
                            contentItem: Text {
                                text: "See all results"
                                font.pixelSize: AppStyle.fontSizeMedium
                                font.family: AppStyle.fontFamily
                                font.weight: Font.Medium
                                color: Theme.textMuted
                            }
                        }
                    }
                }
                footer: Item {
                    width: ListView.view.width
                    height: 6
                }

                section.criteria: ViewSection.FullString
                section.property: "category"
                section.delegate: Item {
                    required property var section

                    height: section === "Games" ? 40 : 48
                    width: ListView.view.width + 26
                    x: -7
                    Rectangle {
                        y: section === "Games" ? 0 : 8
                        width: parent.width
                        height: 32
                        color: Theme.surfaceHover

                        Text {
                            text: section
                            font.pixelSize: AppStyle.fontSizeMedium
                            font.family: AppStyle.fontFamily
                            font.weight: Font.DemiBold
                            color: Theme.textMuted
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: AppStyle.spacingMd
                        }
                    }
                }

                ScrollBar.vertical: FLScrollBar {
                    id: verticalScrollBar
                    policy: ScrollBar.AsNeeded
                    anchors.bottomMargin: AppStyle.spacingSm
                    anchors.right: parent.right
                    anchors.rightMargin: -20
                    width: 12

                    contentItem: Rectangle {
                        radius: AppStyle.radiusMd
                        implicitWidth: 16
                        color: "#ffffff"
                        opacity: verticalScrollBar.visualSize === 1 ? 0.0 : verticalScrollBar.hovered ? 0.6 : 0.3

                        Behavior on opacity {
                            NumberAnimation {
                                duration: 240
                                easing.type: Easing.InOutQuad
                            }
                        }
                    }
                }

                delegate: Button {
                    required property var index
                    required property var model

                    property bool showGlobalCursor: true

                    clip: true

                    width: ListView.view.width
                    height: 48

                    background: Rectangle {
                        color: "white"
                        opacity: down ? 0.06 : hovered ? 0.08 : 0
                        radius: AppStyle.radiusSm
                    }

                    HoverHandler {
                        cursorShape: Qt.PointingHandCursor
                    }

                    contentItem: RowLayout {
                        spacing: AppStyle.spacingMd
                        Image {
                            Layout.fillHeight: true
                            Layout.preferredWidth: height
                            sourceSize.width: 32
                            sourceSize.height: 32
                            source: model.iconSourceUrl
                            fillMode: Image.PreserveAspectFit
                            visible: model.iconSourceUrl !== undefined && model.iconSourceUrl !== ""
                        }

                        Rectangle {
                            color: "grey"
                            Layout.fillHeight: true
                            Layout.preferredWidth: height
                            visible: model.iconSourceUrl === undefined || model.iconSourceUrl === ""
                        }

                        ColumnLayout {
                            Layout.fillHeight: true
                            Layout.fillWidth: true

                            Text {
                                text: model.displayName
                                font.pixelSize: AppStyle.fontSizeMedium
                                font.family: AppStyle.fontFamily
                                font.weight: Font.DemiBold
                                verticalAlignment: Text.AlignVCenter
                                color: Theme.textPrimary
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                                Layout.verticalStretchFactor: 1
                            }

                            Text {
                                text: model.platformName
                                font.pixelSize: AppStyle.fontSizeMedium
                                font.family: AppStyle.fontFamily
                                font.weight: Font.Medium
                                verticalAlignment: Text.AlignVCenter
                                color: Theme.textMuted
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                                Layout.verticalStretchFactor: 1
                            }
                        }

                        Item {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                        }
                    }
                }
            }
        }
    }

    component RoleData: QtObject {
        property string displayName
    }
}
