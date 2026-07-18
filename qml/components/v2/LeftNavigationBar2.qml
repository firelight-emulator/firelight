import QtQuick
import QtQuick.Controls
import QtQuick.Layouts 1.0
import Firelight 1.0

FocusScope {
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        LibraryNavigationMenuItem {
            iconSource: "qrc:/icons/home"
            displayText: "Home"

            Layout.fillWidth: true
        }

        LibraryNavigationMenuItem {
            iconSource: "qrc:/icons/browse"
            displayText: "Library"

            Layout.fillWidth: true
        }

        LibraryNavigationMenuItem {
            iconSource: "qrc:/icons/shopping-bag"
            displayText: "Mod Shop"

            Layout.fillWidth: true
        }

        LibraryNavigationMenuItem {
            iconSource: "qrc:/icons/controller"
            displayText: "Controllers"

            Layout.fillWidth: true
        }

        LibraryNavigationMenuItem {
            iconSource: "qrc:/icons/photo-library"
            displayText: "Gallery"

            Layout.fillWidth: true
        }

        LibraryNavigationMenuItem {
            iconSource: "qrc:/icons/bar-chart"
            displayText: "Activity"

            Layout.fillWidth: true
        }

        LibraryNavigationMenuItem {
            iconSource: "qrc:/icons/trophy"
            displayText: "Achievements"

            Layout.fillWidth: true
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        LibraryNavigationMenuItem {
            iconSource: "qrc:/icons/settings"
            displayText: "Settings"

            Layout.fillWidth: true
        }

        LibraryNavigationMenuItem {
            iconSource: "qrc:/icons/power"
            displayText: "Power"

            Layout.fillWidth: true
        }
    }
}
