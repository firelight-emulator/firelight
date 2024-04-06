import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models
import FirelightStyle 1.0

Item {
    id: root

    // Pane {
    //     id: banner
    //     anchors.top: parent.top
    //     anchors.left: parent.left
    //     anchors.right: parent.right
    //
    //     background: Item {
    //     }
    //
    //     ColumnLayout {
    //         anchors.fill: parent
    //         spacing: 8
    //
    //
    //     }
    //
    // }

    Pane {
        id: content

        background: Item {
        }
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        Flickable {
            anchors.fill: parent
            contentWidth: content.width
            contentHeight: 1000
            boundsBehavior: Flickable.StopAtBounds
            flickableDirection: Flickable.VerticalFlick
            clip: true

            ColumnLayout {
                id: column
                anchors.fill: parent
                spacing: 16

                Text {
                    text: "Pokémon Radical Red"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    Layout.fillWidth: true
                    font.pointSize: 20
                    font.family: Constants.semiboldFontFamily
                    color: "white"
                }

                Text {
                    text: "By soupercell"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    Layout.fillWidth: true
                    font.pointSize: 11
                    font.family: Constants.regularFontFamily
                    color: "white"
                }

                ListView {
                    id: screenshots
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    Layout.preferredHeight: 180
                    Layout.fillWidth: true
                    clip: true
                    orientation: ListView.Horizontal
                    model: ListModel {
                        ListElement {
                            source: "file:///Users/alexs/git/firelight/build/prr1.png"
                        }
                        ListElement {
                            source: "file:///Users/alexs/git/firelight/build/prr2.png"
                        }
                        ListElement {
                            source: "file:///Users/alexs/git/firelight/build/prr3.jpg"
                        }
                        ListElement {
                            source: "file:///Users/alexs/git/firelight/build/prr4.jpg"
                        }
                    }
                    spacing: 8
                    delegate: Image {
                        source: model.source
                        fillMode: Image.Stretch
                        width: parent.height * 1.5
                        height: parent.height
                    }
                }

                Text {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    text: "<p>Pokémon Radical Red is an enhancement hack of Pokémon Fire Red.</p>
                            <p>This is a difficulty hack, with massive additional features added to help you navigate through this game.</p>
                            <p>This hack utilises the Complete Fire Red Upgrade Engine and Dynamic Pokemon Expansion built
                             by Skeli789, Ghoulslash, and others. It's responsible for most of the significant features
                              in the hack.</p>
                            <p>List of features (Most of them provided by CFRU and DPE):</p>
                            <ul>
                                <li>Much higher difficulty, with optional modes to add or mitigate difficulty</li>
                                <li>Built-in Randomizer options (Pokémon, Abilities and Learnsets)</li>
                                <li>Physical/Special split + Fairy Typing</li>
                                <li>All Pokémon up to Gen 9 obtainable (with some exceptions)</li>
                                <li>Most Moves up to Gen 9</li>
                                <li>Updated Pokémon sprites</li>
                                <li>Mega Evolutions & Z-Moves</li>
                                <li>Most Abilities up to Gen 9</li>
                                <li>All important battle items (with some exceptions)</li>
                                <li>Wish Piece Raid Battles (with Dynamax)</li>
                                <li>Mystery Gifts</li>
                                <li>Reusable TMs</li>
                                <li>Expanded TM list</li>
                                <li>Additional move tutors</li>
                                <li>EV Training Gear and NPCs</li>
                                <li>Ability popups during battle</li>
                                <li>Party Exp Share (can be disabled)</li>
                                <li>Hidden Abilities</li>
                                <li>Day, Dusk and Night cycle (syncs with RTC)</li>
                                <li>DexNav, which allows you to search for Pokémon with hidden abilities and more</li>
                                <li>Even faster turbo speed on bike and while surfing</li>
                                <li>Abilities like Magma Armor, Static, or Flash Fire have overworld effects like in recent generations</li>
                                <li>Destiny Knot, Everstone have updated breeding mechanics</li>
                                <li>Lots of Quality of Life changes</li>
                                <li>... and more!</li>
                            </ul>"
                    wrapMode: Text.WordWrap
                    font.pointSize: 12
                    font.family: Constants.regularFontFamily
                    color: "#d5d5d5"
                }
            }
        }

        // Pane {
        //     id: rightSection
        //
        //     background: Item {
        //     }
        //
        //     anchors.top: parent.top
        //     anchors.bottom: parent.bottom
        //     anchors.right: parent.right
        //     width: parent.width * 0.3
        //
        //     ColumnLayout {
        //         anchors.centerIn: parent
        //         spacing: 8
        //
        //         Button {
        //             id: addToLibraryButton
        //             text: "Add to Library"
        //             Layout.preferredWidth: 200
        //             Layout.preferredHeight: 60
        //             Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
        //         }
        //         Item {
        //             Layout.fillWidth: true
        //             Layout.preferredHeight: 8
        //         }
        //         Text {
        //             text: "You need Pokémon Fire Red in your Library to play this game"
        //             font.family: Constants.regularFontFamily
        //             color: "white"
        //             wrapMode: Text.WordWrap
        //             font.pointSize: 10
        //             Layout.preferredWidth: 300
        //             Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
        //             horizontalAlignment: Text.AlignHCenter
        //         }
        //
        //         Item {
        //             Layout.fillWidth: true
        //             Layout.fillHeight: true
        //         }
        //     }
        // }
        //
        // Pane {
        //     id: titleBar
        //
        //     background: Item {
        //     }
        //
        //     anchors.top: parent.top
        //     anchors.right: rightSection.left
        //     anchors.left: parent.left
        //
        //     ColumnLayout {
        //         anchors.fill: parent
        //         spacing: 8
        //
        //         Text {
        //             text: "Pokémon Radical Red"
        //             horizontalAlignment: Text.AlignLeft
        //             verticalAlignment: Text.AlignVCenter
        //             font.pointSize: 20
        //             font.family: Constants.semiboldFontFamily
        //             color: "white"
        //         }
        //
        //         Text {
        //             text: "By soupercell"
        //             horizontalAlignment: Text.AlignLeft
        //             verticalAlignment: Text.AlignVCenter
        //             font.pointSize: 11
        //             font.family: Constants.regularFontFamily
        //             color: "white"
        //         }
        //
        //
        //     }
        // }
        //
        // Pane {
        //     id: leftSection
        //
        //     background: Item {
        //     }
        //     anchors.top: titleBar.bottom
        //     anchors.bottom: parent.bottom
        //     anchors.left: parent.left
        //     anchors.right: rightSection.left
        //
        //     Flickable {
        //         anchors.fill: parent
        //         contentWidth: leftSection.width
        //         contentHeight: 1000
        //         boundsBehavior: Flickable.StopAtBounds
        //         flickableDirection: Flickable.VerticalFlick
        //         clip: true
        //
        //         ColumnLayout {
        //             id: column
        //             anchors.fill: parent
        //             spacing: 16
        //
        //             ListView {
        //                 id: screenshots
        //                 Layout.alignment: Qt.AlignLeft | Qt.AlignTop
        //                 Layout.preferredHeight: 240
        //                 Layout.fillWidth: true
        //                 clip: true
        //                 orientation: ListView.Horizontal
        //                 model: ListModel {
        //                     ListElement {
        //                         source: "file:///Users/alexs/git/firelight/build/prr1.png"
        //                     }
        //                     ListElement {
        //                         source: "file:///Users/alexs/git/firelight/build/prr2.png"
        //                     }
        //                     ListElement {
        //                         source: "file:///Users/alexs/git/firelight/build/prr3.jpg"
        //                     }
        //                     ListElement {
        //                         source: "file:///Users/alexs/git/firelight/build/prr4.jpg"
        //                     }
        //                 }
        //                 spacing: 8
        //                 delegate: Image {
        //                     source: model.source
        //                     fillMode: Image.Stretch
        //                     width: parent.height * 1.5
        //                     height: parent.height
        //                 }
        //             }
        //
        //             Text {
        //                 Layout.fillWidth: true
        //                 Layout.alignment: Qt.AlignLeft | Qt.AlignTop
        //                 text: "<p>Pokémon Radical Red is an enhancement hack of Pokémon Fire Red.</p>
        //                     <p>This is a difficulty hack, with massive additional features added to help you navigate through this game.</p>
        //                     <p>This hack utilises the Complete Fire Red Upgrade Engine and Dynamic Pokemon Expansion built
        //                      by Skeli789, Ghoulslash, and others. It's responsible for most of the significant features
        //                       in the hack.</p>
        //                     <p>List of features (Most of them provided by CFRU and DPE):</p>
        //                     <ul>
        //                         <li>Much higher difficulty, with optional modes to add or mitigate difficulty</li>
        //                         <li>Built-in Randomizer options (Pokémon, Abilities and Learnsets)</li>
        //                         <li>Physical/Special split + Fairy Typing</li>
        //                         <li>All Pokémon up to Gen 9 obtainable (with some exceptions)</li>
        //                         <li>Most Moves up to Gen 9</li>
        //                         <li>Updated Pokémon sprites</li>
        //                         <li>Mega Evolutions & Z-Moves</li>
        //                         <li>Most Abilities up to Gen 9</li>
        //                         <li>All important battle items (with some exceptions)</li>
        //                         <li>Wish Piece Raid Battles (with Dynamax)</li>
        //                         <li>Mystery Gifts</li>
        //                         <li>Reusable TMs</li>
        //                         <li>Expanded TM list</li>
        //                         <li>Additional move tutors</li>
        //                         <li>EV Training Gear and NPCs</li>
        //                         <li>Ability popups during battle</li>
        //                         <li>Party Exp Share (can be disabled)</li>
        //                         <li>Hidden Abilities</li>
        //                         <li>Day, Dusk and Night cycle (syncs with RTC)</li>
        //                         <li>DexNav, which allows you to search for Pokémon with hidden abilities and more</li>
        //                         <li>Even faster turbo speed on bike and while surfing</li>
        //                         <li>Abilities like Magma Armor, Static, or Flash Fire have overworld effects like in recent generations</li>
        //                         <li>Destiny Knot, Everstone have updated breeding mechanics</li>
        //                         <li>Lots of Quality of Life changes</li>
        //                         <li>... and more!</li>
        //                     </ul>"
        //                 wrapMode: Text.WordWrap
        //                 font.pointSize: 12
        //                 font.family: Constants.regularFontFamily
        //                 color: "white"
        //             }
        //         }
        //     }


        // }
    }
}