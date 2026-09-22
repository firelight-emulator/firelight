// TODO: NEEDS REVIEW
import QtQuick
import QtQuick.Controls
import Firelight 1.0

FocusScope {
    id: root
    required property int index

    required property string displayName
    required property string icon1x1SourceUrl
    required property int platformId
    required property int entryId
    required property string contentHash
    required property bool playable
    required property string statusText
    required property bool favorite
    required property int achievementsEarned
    required property int achievementsTotal

    readonly property bool selecting: root.GridView.view.selectionGroup.active
    readonly property bool selected: root.GridView.view.selectionGroup.selected[root.index] === true

    property var titleBoxHeight: 0
    property bool canLaunch: true

    signal clicked(var tapPoint)
    signal launchRequested(string entryId, string contentHash, string platformId, bool playable, string statusText)

    property GameContextMenu _contextMenu: null

    function contextMenu(): GameContextMenu {
        if (root._contextMenu === null) {
            root._contextMenu = contextMenuComponent.createObject(control);
        }

        return root._contextMenu;
    }

    Component {
        id: contextMenuComponent

        GameContextMenu {
            entry: root.model
        }
    }

    Button {
        id: control
        anchors.fill: parent
        anchors.margins: AppearanceSettings.libraryIconGridTileSpacing / 2
        padding: 0
        horizontalPadding: 0
        hoverEnabled: true
        focus: true

        objectName: "GameGridViewItem|" + root.displayName

        FLFocus.showCursor: true
        FLFocus.spacing: 3
        FLFocus.fill: "black"
        FLFocus.focusSound: SoundEffects.gameTileFocus
        FLFocus.radius: gameTile.radius + Math.round(FLFocus.spacing / 2)

        TapHandler {
            id: selectTap
            acceptedButtons: Qt.LeftButton
            onSingleTapped: {
                if (root.selecting) {
                    root.GridView.view.selectionGroup.select(root.index, selectTap.point.modifiers);
                    return;
                }

                root.clicked(selectTap.point)
            }

            onDoubleTapped: {
                if (!root.selecting && root.canLaunch) {
                    activatedAnimation.restart()
                }
            }
        }

        HoverHandler {
            cursorShape: Qt.PointingHandCursor
        }

        ContextMenu.onRequested: position => {
            if (root.selecting || !root.canLaunch) {
                return;
            }

            root.contextMenu().popup(position);
        }

        FLFocus.actions: [
            FLAction {
                keys: [Qt.Key_Enter, Qt.Key_Select, Qt.Key_Return, Qt.Key_Space]
                label: root.selecting ? root.selected ? qsTr("Deselect") : qsTr("Select") : qsTr("Play")
                sound: root.selecting ? root.selected ? SoundEffects.deselectItem : SoundEffects.selectItem : SoundEffects.activateGame
                enabled: root.selecting || root.canLaunch
                hidden: !root.selecting && !root.canLaunch
                onTriggered: {
                    if (root.selecting) {
                        root.GridView.view.selectionGroup.select(root.index, Qt.NoModifier);
                        return;
                    }

                    activatedAnimation.restart()
                }
            },
            FLAction {
                keys: [Qt.Key_Menu]
                label: qsTr("Options")
                sound: SoundEffects.openPopup
                enabled: !root.selecting && root.canLaunch
                hidden: root.selecting || !root.canLaunch
                onTriggered: root.contextMenu().popupFor(control, control.width + AppStyle.spacingSm, 0)
            },
            FLAction {
                keys: [Qt.Key_Return, Qt.Key_Enter, Qt.Key_Space, Qt.Key_Select]
                modifiers: Qt.ControlModifier
                label: qsTr("Options")
                sound: SoundEffects.openPopup
                enabled: !root.selecting && root.canLaunch
                hidden: root.selecting || !root.canLaunch
                onTriggered: root.contextMenu().popupFor(control, control.width + AppStyle.spacingSm, 0)
            }
        ]

        SequentialAnimation {
            id: activatedAnimation
            ParallelAnimation {
                NumberAnimation {
                    target: control
                    property: "scale"
                    from: 1.0
                    to: 0.95
                    duration: AppStyle.durationVeryFast
                    easing.type: Easing.InOutQuad
                }
            }
            NumberAnimation {
                target: control
                property: "scale"
                from: 0.95
                to: 1.0
                duration: AppStyle.durationVeryFast
                easing.type: Easing.InOutQuad
            }
            PauseAnimation {
                duration: AppStyle.durationBase
            }
            ScriptAction {
                script: {
                    root.launchRequested(root.entryId, root.contentHash, root.platformId, root.playable, root.statusText);
                }
            }
        }

        transform: Translate {
            y: control.hovered ? -2 : 0

            Behavior on y {
                NumberAnimation {
                    duration: AppStyle.durationFast
                    easing.type: Easing.InOutQuad
                }
            }
        }

        background: Item {}
        contentItem: Column {
            GameTile {
                id: gameTile
                source: FLUtil.toUrl(root.icon1x1SourceUrl)
                size: control.width
                topLeftRadius: AppStyle.radiusMd
                topRightRadius: AppStyle.radiusMd
                bottomLeftRadius: titleBox.height > 0 ? 0 : AppStyle.radiusMd
                bottomRightRadius: titleBox.height > 0 ? 0 : AppStyle.radiusMd
                platformId: root.platformId
                title: root.displayName
                titleVisible: control.hovered || control.activeFocus

                GridItemSelectionOverlay {
                    selectionGroup: root.GridView.view.selectionGroup
                    index: root.index

                    topLeftRadius: gameTile.topLeftRadius
                    topRightRadius: gameTile.topRightRadius
                    bottomLeftRadius: gameTile.bottomLeftRadius
                    bottomRightRadius: gameTile.bottomRightRadius
                }
            }

            Rectangle {
                id: titleBox
                width: parent.width
                height: AppearanceSettings.libraryIconGridShowTitleBox ? root.titleBoxHeight : 0

                Behavior on height {
                    NumberAnimation {
                        duration: AppStyle.durationBase
                        easing.type: Easing.InOutQuad
                    }
                }

                color: Theme.surface
                topLeftRadius: 0
                topRightRadius: 0
                bottomRightRadius: AppStyle.radiusMd
                bottomLeftRadius: AppStyle.radiusMd

                Icon {
                    id: favoriteIcon
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.margins: AppStyle.spacingSm
                    name: "favorite"
                    filled: true
                    visible: root.favorite && AppearanceSettings.libraryIconGridShowFavoriteIcon
                    size: Math.round(16 * AppStyle.scale)
                    color: Theme.favorite
                }

                Text {
                    anchors.left: parent.left
                    anchors.right: favoriteIcon.visible ? favoriteIcon.left : parent.right
                    anchors.bottom: parent.bottom
                    anchors.top: parent.top
                    anchors.margins: AppStyle.spacingSm
                    text: root.displayName
                    color: Theme.textPrimary
                    font.family: AppStyle.fontFamily
                    font.pixelSize: AppStyle.fontSizeSmall
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                    wrapMode: Text.WordWrap
                    maximumLineCount: 2
                }
            }
        }

        Item {
            z: 2
            anchors.fill: parent
            anchors.margins: AppStyle.spacingXs
            visible: !root.selecting

            Row {
                anchors.left: parent.left
                anchors.top: parent.top
                spacing: AppStyle.spacingXs

                Rectangle {
                    visible: !root.playable
                    width: Math.round(22 * AppStyle.scale)
                    height: width
                    radius: AppStyle.radiusSm
                    color: "#99000000"
                    Icon {
                        anchors.centerIn: parent
                        name: "cancel"
                        size: Math.round(14 * AppStyle.scale)
                        color: Theme.textMuted
                    }
                }
            }

            Rectangle {
                id: achPill
                readonly property bool done: root.achievementsTotal > 0 && root.achievementsEarned >= root.achievementsTotal
                visible: root.achievementsTotal > 0
                anchors.right: parent.right
                anchors.top: parent.top
                height: Math.round(18 * AppStyle.scale)
                width: achLabel.implicitWidth + Math.round(10 * AppStyle.scale)
                radius: height / 2
                color: achPill.done ? Theme.gold : "#aa000000"
                Text {
                    id: achLabel
                    anchors.centerIn: parent
                    text: root.achievementsEarned + "/" + root.achievementsTotal
                    color: achPill.done ? Theme.onAccent : "white"
                    font.family: AppStyle.fontFamily
                    font.pixelSize: AppStyle.fontSizeSmall
                    font.weight: Font.DemiBold
                }
            }
        }
    }
}