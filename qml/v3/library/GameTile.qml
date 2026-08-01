import QtQuick

Item {
    id: root

    property url source: ""
    property int size: 140
    property real radius: size < 100 ? AppStyle.radiusSm : AppStyle.radiusMd
    property string title: ""
    property bool titleVisible: false
    required property int platformId

    implicitWidth: size
    implicitHeight: size

    Item {
        id: rounded
        anchors.fill: parent

        layer.enabled: root.radius > 0
        layer.smooth: true
        layer.effect: FLTileSurface {
            radius: root.radius
            topLight: AppStyle.tileTopLight
            bottomShade: AppStyle.tileBottomShade
            bevelFrac: AppStyle.tileBevelDepth
        }

        Rectangle {
            anchors.fill: parent
            color: Theme.surfaceElevated

            FLPlatformIcon {
                anchors.fill: parent
                anchors.margins: Math.round(root.size * 0.17)
                visible: artImage.status !== Image.Ready
                platformId: root.platformId
            }
        }

        FLRoundedImage {
            id: artImage
            anchors.fill: parent
            visible: status === Image.Ready
            source: root.source
            radius: 0
            fillMode: Image.PreserveAspectCrop
            background: "transparent"
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: Math.round(parent.height * 0.55)
            opacity: root.titleVisible ? 1 : 0
            gradient: Gradient {
                GradientStop {
                    position: 0.0
                    color: "transparent"
                }
                GradientStop {
                    position: 1.0
                    color: "#dd000000"
                }
            }
            Behavior on opacity {
                NumberAnimation {
                    duration: AppStyle.durationFast
                    easing.type: Easing.OutQuad
                }
            }

            Text {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: AppStyle.spacingSm
                text: root.title
                color: Theme.textPrimary
                font.family: AppStyle.fontFamily
                font.pixelSize: AppStyle.fontSizeSmall
                font.weight: Font.DemiBold
                elide: Text.ElideRight
                maximumLineCount: 2
                wrapMode: Text.WordWrap
            }
        }
    }
}
