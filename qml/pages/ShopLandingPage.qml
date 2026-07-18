import QtQuick
import QtQuick.Controls

FocusScope {
    id: page
    property alias model: gridView.model

    GridView {
        id: gridView
        objectName: "GridView"
        clip: true
        header: Text {
            id: introText
            text: "The Mod Shop is where you'll be able to easily find and download mods for your games.\n\nIt's not ready yet, but here are some of the awesome mods you can look forward to!"
            color: Theme.textPrimary
            font.pixelSize: AppStyle.fontSizeMedium
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
                Router.navigateTo("/shop/mods/" + delegate.model.id)
                // page.StackView.view.push(shopItemPage, {model: delegate.model})
            }

        }
    }

    Component {
        id: shopItemPage
        ShopItemPage {
        }
    }
}

