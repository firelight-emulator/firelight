import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import QtQuick.Layouts
import Firelight 1.0

Pane {
    id: root

    property color startingColor: "#330000"
    property int currentIndex: 0

    background: Rectangle {
        color: "#1b1d27"
        radius: 8
    }

    Flickable {
        anchors.fill: parent
        contentHeight: flickableContentColumn.implicitHeight
        boundsBehavior: Flickable.StopAtBounds

        ColumnLayout {
            id: flickableContentColumn
            width: parent.width

            FlexboxLayout {
                Layout.preferredHeight: 500
                Layout.preferredWidth: 800
                Layout.alignment: Qt.AlignHCenter
                Pane {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    background: Rectangle {
                        color: Qt.darker("#1b1d27", 1.25)
                        radius: 8
                    }

                    ColumnLayout {
                        anchors.fill: parent

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 36

                            Text {
                                Layout.leftMargin: 8
                                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                                text: "Activity"
                                color: "white"
                                font.pointSize: 13
                                font.weight: Font.DemiBold
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignLeft
                                font.family: Constants.regularFontFamily
                            }

                            Item {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 1
                            }

                            ButtonBar {

                            }


                        }

                        Item {
                            id: buttonBar
                            Layout.fillWidth: true
                            Layout.preferredHeight: 48
                            Layout.topMargin: 8

                            Button {
                                anchors.right: bucketRangeLabelText.left
                                width: 34
                                height: 34
                                anchors.verticalCenter: parent.verticalCenter
                                enabled: root.currentIndex > 0

                                background: Rectangle {
                                    color: Qt.darker("#1b1d27", 1.12)
                                    radius: 8
                                    border.color: Qt.lighter("#1b1d27", 1.3)
                                    border.width: 1
                                }

                                Image {
                                    anchors.centerIn: parent
                                    source: "qrc:/icons/chevron-back"
                                    width: 16
                                    height: 16
                                    fillMode: Image.PreserveAspectFit
                                }

                                onClicked: {
                                    var newIndex = root.currentIndex - 1
                                    var playtimeAtNewIndex = ActivityBucketsModel.get(newIndex).playtimeSeconds;
                                    while (playtimeAtNewIndex === 0 && newIndex > 0) {
                                        newIndex--;
                                        playtimeAtNewIndex = ActivityBucketsModel.get(newIndex).playtimeSeconds;
                                    }

                                    root.currentIndex = newIndex
                                }
                            }

                            Text {
                                id: bucketRangeLabelText
                                anchors.centerIn: parent
                                width: 180
                                text: ActivityBucketsModel.get(root.currentIndex).label ?? ""
                                color: "white"
                                font.pointSize: 13
                                font.weight: Font.Medium
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                                font.family: Constants.regularFontFamily
                            }

                            Button {
                                anchors.left: bucketRangeLabelText.right
                                width: 34
                                height: 34
                                anchors.verticalCenter: parent.verticalCenter
                                enabled: root.currentIndex < ActivityBucketsModel.count - 1

                                background: Rectangle {
                                    color: Qt.darker("#1b1d27", 1.12)
                                    radius: 8
                                    border.color: Qt.lighter("#1b1d27", 1.3)
                                    border.width: 1
                                }

                                Image {
                                    anchors.centerIn: parent
                                    source: "qrc:/icons/chevron-forward"
                                    width: 16
                                    height: 16
                                    fillMode: Image.PreserveAspectFit
                                }
                                onClicked: {
                                    var newIndex = root.currentIndex + 1;
                                    var playtimeAtNewIndex = ActivityBucketsModel.get(newIndex).playtimeSeconds;
                                    while (playtimeAtNewIndex === 0 && newIndex < ActivityBucketsModel.count - 1) {
                                        newIndex++;
                                        playtimeAtNewIndex = ActivityBucketsModel.get(newIndex).playtimeSeconds;
                                    }

                                    root.currentIndex = newIndex;
                                }
                            }
                        }

                        Item {
                            id: activityDisplay
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            property int displayedIndex: root.currentIndex
                            property var dayData: ActivityBucketsModel.get(displayedIndex)

                            Text {
                                anchors.top: parent.top
                                anchors.horizontalCenter: parent.horizontalCenter
                                color: "lime"
                                text: "playtime=" + activityDisplay.dayData.playtimeSeconds + "s | buckets=" + activityDisplay.dayData.numberOfBuckets
                                z: 99
                            }

                            Item {
                                id: yAxisBars
                                anchors.fill: parent

                                property var sectionHeight: activityDisplay.height / activityDisplay.dayData.yAxisValues.length

                                Repeater {
                                    model: activityDisplay.dayData.yAxisValues
                                    delegate: Item {
                                        required property var modelData
                                        required property int index

                                        width: parent.width
                                        height: yAxisBars.sectionHeight
                                        y: parent.height - (yAxisBars.sectionHeight * index) - height

                                        Text {
                                            anchors.left: parent.left
                                            anchors.leftMargin: 4
                                            anchors.bottom: parent.top
                                            anchors.bottomMargin: 4
                                            color: "white"
                                            opacity: 0.5
                                            font.family: Constants.regularFontFamily
                                            font.weight: Font.Normal
                                            font.pointSize: 12
                                            text: modelData
                                        }

                                        Rectangle {
                                            anchors.left: parent.left
                                            width: parent.width
                                            height: 1
                                            color: "white"
                                            opacity: 0.5
                                        }
                                    }
                                }
                            }

                            Row {
                                anchors.fill: parent

                                Repeater {
                                    model: activityDisplay.dayData.buckets
                                    delegate: Item {
                                        id: hourItem
                                        required property var modelData
                                        required property int index
                                        width: activityDisplay.width / 24
                                        height: activityDisplay.height

                                        readonly property real barHeight: activityDisplay.dayData.maxPlaytimeSeconds > 0
                                            ? activityDisplay.height / activityDisplay.dayData.maxPlaytimeSeconds
                                            : 0

                                        Rectangle {
                                            anchors.fill: parent
                                            color: "white"
                                            opacity: barHoverHandler.hovered ? 0.05 : 0
                                        }

                                        Text {
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            anchors.top: parent.bottom
                                            color: "white"
                                            text: modelData.label
                                        }

                                        HoverHandler {
                                            id: barHoverHandler
                                        }

                                        ToolTip {
                                            parent: Overlay.overlay
                                            visible: barHoverHandler.hovered
                                            height: hoverGameList.height + padding * 2
                                            padding: 8
                                            x: barHoverHandler.point.scenePosition.x + 16
                                            y: barHoverHandler.point.scenePosition.y + 16

                                            background: Rectangle {
                                                color: Qt.lighter("#12131A", 1.4)
                                                radius: 4
                                                border.color: Qt.lighter("#12131A", 2)
                                                border.width: 1
                                                // bottomRightRadius: 8
                                                // bottomLeftRadius: 8

                                                layer.enabled: true
                                                layer.effect: MultiEffect {
                                                    // autoPaddingEnabled: false
                                                    // paddingRect: Qt.rect(-16, -16, 32, 32)
                                                    shadowEnabled: true
                                                    shadowColor: Qt.darker("#12131A", 2)
                                                    shadowBlur: 1.0
                                                    shadowVerticalOffset: 4.0
                                                    shadowHorizontalOffset: 4.0
                                                }
                                            }

                                            ColumnLayout {
                                                id: hoverGameList
                                                width: parent.width
                                                spacing: 8

                                                Text {
                                                    text: "Games played during this hour:"
                                                    font.pointSize: 12
                                                    font.family: Constants.regularFontFamily
                                                    font.weight: Font.DemiBold
                                                    verticalAlignment: Text.AlignVCenter
                                                    color: "white"
                                                    Layout.fillWidth: true
                                                    Layout.bottomMargin: 8
                                                }

                                                Repeater {
                                                    model: hourItem.modelData.gameActivityRecords
                                                    delegate: RowLayout {
                                                        required property int index
                                                        required property var modelData
                                                        implicitHeight: 50
                                                        spacing: 12
                                                        Text {
                                                            text: "#" + (index + 1)
                                                            font.pointSize: 11
                                                            font.family: Constants.regularFontFamily
                                                            font.weight: Font.DemiBold
                                                            verticalAlignment: Text.AlignVCenter
                                                            color: "white"
                                                            Layout.fillHeight: true
                                                            Layout.preferredWidth: 24
                                                        }
                                                        Image {
                                                            Layout.fillHeight: true
                                                            Layout.preferredWidth: height
                                                            sourceSize.width: 32
                                                            sourceSize.height: 32
                                                            source: modelData.iconSourceUrl
                                                            fillMode: Image.PreserveAspectFit
                                                            visible: modelData.iconSourceUrl !== undefined && modelData.iconSourceUrl !== ""
                                                        }

                                                        Rectangle {
                                                            color: "grey"
                                                            Layout.fillHeight: true
                                                            Layout.preferredWidth: height
                                                            visible: modelData.iconSourceUrl === undefined || modelData.iconSourceUrl === ""
                                                        }

                                                        ColumnLayout {
                                                            Layout.fillHeight: true
                                                            Layout.fillWidth: true

                                                            Text {
                                                                text: modelData.displayName
                                                                font.pointSize: 11
                                                                font.family: Constants.regularFontFamily
                                                                font.weight: Font.DemiBold
                                                                verticalAlignment: Text.AlignVCenter
                                                                color: "white"
                                                                Layout.fillHeight: true
                                                                Layout.fillWidth: true
                                                                Layout.verticalStretchFactor: 1
                                                            }

                                                            Text {
                                                                text: modelData.platformName
                                                                font.pointSize: 11
                                                                font.family: Constants.regularFontFamily
                                                                font.weight: Font.Medium
                                                                verticalAlignment: Text.AlignVCenter
                                                                color: "#9e9e9e"
                                                                Layout.fillHeight: true
                                                                Layout.fillWidth: true
                                                                Layout.verticalStretchFactor: 1
                                                            }
                                                        }

                                                        Text {
                                                            text: {
                                                                const seconds = modelData.secondsPlayed;
                                                                const hours = Math.floor(seconds / 3600);
                                                                const minutes = Math.floor((seconds % 3600) / 60);

                                                                let result = "";
                                                                if (hours > 0) {
                                                                    result += hours + "h "
                                                                }
                                                                if (minutes > 0) {
                                                                    result += minutes + "m"
                                                                }
                                                                if (result === "") {
                                                                    result = seconds + "s"
                                                                }
                                                                return result
                                                            }
                                                            font.pointSize: 11
                                                            font.family: Constants.regularFontFamily
                                                            font.weight: Font.DemiBold
                                                            verticalAlignment: Text.AlignVCenter
                                                            color: "white"
                                                            Layout.fillHeight: true
                                                        }

                                                        Item {
                                                            Layout.fillWidth: true
                                                            Layout.fillHeight: true
                                                        }
                                                    }
                                                }
                                            }

                                        }

                                        Column {
                                            anchors.bottom: parent.bottom
                                            width: parent.width
                                            spacing: 0

                                            Repeater {
                                                model: hourItem.modelData.gameActivityRecords
                                                delegate: Rectangle {
                                                    id: gameSegment
                                                    required property var modelData
                                                    required property int index
                                                    width: hourItem.width
                                                    height: modelData.secondsPlayed * hourItem.barHeight
                                                    // height: modelData.percentPlayed * hourItem.barHeight
                                                    color: Qt.hsla(index * 0.3, 0.7, 0.5, 1)

                                                    //
                                                    // Pane {
                                                    //     x: gameHoverHandler.point.position.x + 20
                                                    //     y: gameHoverHandler.point.position.y
                                                    //     z: 1000
                                                    //     height: 50
                                                    //     visible: gameHoverHandler.hovered
                                                    //     padding: 6
                                                    //
                                                    //     background: Rectangle {
                                                    //         color: Qt.lighter("#12131A", 1.4)
                                                    //         radius: 4
                                                    //         border.color: Qt.lighter("#12131A", 2)
                                                    //         border.width: 1
                                                    //         // bottomRightRadius: 8
                                                    //         // bottomLeftRadius: 8
                                                    //
                                                    //         layer.enabled: true
                                                    //         layer.effect: MultiEffect {
                                                    //             // autoPaddingEnabled: false
                                                    //             // paddingRect: Qt.rect(-16, -16, 32, 32)
                                                    //             shadowEnabled: true
                                                    //             shadowColor: Qt.darker("#12131A", 2)
                                                    //             shadowBlur: 1.0
                                                    //             shadowVerticalOffset: 4.0
                                                    //             shadowHorizontalOffset: 4.0
                                                    //         }
                                                    //     }
                                                    //
                                                    //
                                                    // }

                                                    HoverHandler {
                                                        id: gameHoverHandler
                                                        blocking: true
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                    }




                }
                Pane {
                    Layout.preferredWidth: 500
                    Layout.minimumWidth: 400
                    Layout.fillHeight: true
                    background: Rectangle {
                        color: Qt.darker("#1b1d27", 1.25)
                        radius: 8
                    }

                    ListView {
                        anchors.fill: parent
                        model: ActivityBucketsModel.get(root.currentIndex).gameActivityRecords
                        delegate: Item {
                            required property var modelData
                            required property int index
                            width: parent.width
                            height: 50

                            RowLayout {
                                anchors.fill: parent
                                spacing: 8

                                Image {
                                    Layout.preferredWidth: 32
                                    Layout.fillHeight: true
                                    sourceSize.width: 32
                                    sourceSize.height: 32
                                    source: modelData.iconSourceUrl
                                    fillMode: Image.PreserveAspectFit
                                }

                                Text {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    text: modelData.displayName + " (" + modelData.platformName + ")"
                                    color: "white"
                                    font.pointSize: 12
                                    font.family: Constants.regularFontFamily
                                    font.weight: Font.Medium
                                }

                                Text {
                                    Layout.preferredWidth: 60
                                    Layout.fillHeight: true
                                    text: {
                                        const seconds = modelData.secondsPlayed;
                                        const hours = Math.floor(seconds / 3600);
                                        const minutes = Math.floor((seconds % 3600) / 60);

                                        let result = "";
                                        if (hours > 0) {
                                            result += hours + "h "
                                        }
                                        if (minutes > 0) {
                                            result += minutes + "m"
                                        }
                                        if (result === "") {
                                            result = seconds + "s"
                                        }
                                        return result;
                                    }
                                    color: "white"
                                    font.pointSize: 12
                                    font.family: Constants.regularFontFamily
                                    font.weight: Font.Medium
                                }
                            }
                        }
                    }
                }
            }

        }
    }






}