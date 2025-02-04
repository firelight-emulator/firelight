import QtQuick
import QtQuick.Controls

FocusScope {
    id: page
    property alias model: gridView.model

    // readonly property var Mods: [
    //     {
    //         name: "Super Mario 64: Beyond the Cursed Mirror",
    //         platformName: "Nintendo 64",
    //         tagline: "Greetings Superstar, did you know that your nemesis hid in the decrepit castle all this time? It appears he fosters his strength in a hidden compartment behind one cursed mirror. I implore you to defeat him, once and for all – for the sake of the kingdom. -Yours truly, The Showrunner",
    //         author: "Rovertronic",
    //         description: "SM64: Beyond the Cursed Mirror is a major Super Mario 64 ROM hack created by Rovert, which contains 15 custom-made courses and 120 stars waiting to be collected. With a unique in-depth story, original characters, and new mechanics, this SM64 ROM hack will provide a traditional yet unique experience.",
    //         clearLogoImageUrl: "file:system/_img/cursedmirrorclear.png",
    //         images: [
    //             "file:system/_img/cursedmirror1.png",
    //             "file:system/_img/cursedmirror2.png",
    //             "file:system/_img/cursedmirror3.png",
    //             "file:system/_img/cursedmirror4.png",
    //             "file:system/_img/cursedmirror5.png",
    //             "file:system/_img/cursedmirror6.png"
    //         ]
    //     }
    // ]

    GridView {
        id: gridView
        objectName: "GridView"
        clip: true
        header: Text {
            id: introText
            text: "The Mod Shop is where you'll be able to easily find and download mods for your games.\n\nIt's not ready yet, but here are some of the awesome mods you can look forward to!"
            color: ColorPalette.neutral100
            font.pixelSize: 16
            font.weight: Font.Normal
            width: gridView.width
            font.family: Constants.regularFontFamily
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            topPadding: 24
            bottomPadding: 24
        }
        width: Math.min(Math.floor(parent.width / cellContentWidth), 7) * cellContentWidth
        anchors.horizontalCenter: parent.horizontalCenter
        height: parent.height

        preferredHighlightBegin: 0.33 * gridView.height
        preferredHighlightEnd: 0.66 * gridView.height
        highlightRangeMode: GridView.ApplyRange

        populate: Transition {
            id: popTransition
            SequentialAnimation {
                PropertyAction {
                    property: "opacity"
                    value: 0
                }
                PauseAnimation {
                    duration: popTransition.ViewTransition.index * 50
                }
                ParallelAnimation {
                    PropertyAnimation {
                        property: "opacity"
                        from: 0
                        to: 1
                        duration: 300
                        easing.type: Easing.InOutQuad
                    }
                    PropertyAnimation {
                        property: "y"
                        from: popTransition.ViewTransition.destination.y + 20
                        to: popTransition.ViewTransition.destination.y
                        duration: 300
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        }

        contentY: 0

        readonly property int cellSpacing: 12
        readonly property int cellContentWidth: 360 + cellSpacing
        readonly property int cellContentHeight: 240 + cellSpacing

        cellWidth: cellContentWidth
        cellHeight: cellContentHeight
        focus: true

        currentIndex: 0
        boundsBehavior: Flickable.StopAtBounds

        delegate: ShopGridItemDelegate {
            id: delegate
            // opacity: 0
            cellWidth: gridView.cellWidth
            cellHeight: gridView.cellHeight
            cellSpacing: gridView.cellSpacing

            onDoTheThing: {
                page.StackView.view.push(shopItemPage, {model: delegate.model})
            }

            // Button {
            //     anchors.centerIn: parent
            //
            //     onClicked: function() {
            //         page.StackView.view.push(shopItemPage)
            //     }
            // }
        }
    }

    Component {
        id: shopItemPage
        ShopItemPage {
        }
    }
}

