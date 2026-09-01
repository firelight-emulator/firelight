pragma Singleton
import QtQuick

QtObject {
    id: root

    readonly property bool enabled: true

    readonly property FLSoundEffect cursorBump: FLSoundEffect {
        source: "qrc:/sfx/bump"
        volume: 0.35
    }

    readonly property FLSoundEffect openPopup: FLSoundEffect {
        source: "qrc:/sfx/show-modal"
        volume: 0.3
    }

    readonly property FLSoundEffect back: FLSoundEffect {
        source: "qrc:/sfx/back"
        volume: 0.3
    }

    readonly property FLSoundEffect menuNavigate: FLSoundEffect {
        source: "qrc:/sfx/blip"
        volume: 0.35
    }

    readonly property FLSoundEffect radioSelect: FLSoundEffect {
        source: "qrc:/sfx/select-radio"
        volume: 0.8
    }

    readonly property FLSoundEffect iconButtonFocus: FLSoundEffect {
        source: "qrc:/sfx/button-nav"
    }

    readonly property FLSoundEffect gameTileFocus: FLSoundEffect {
        source: "qrc:/sfx/game-tile-focus"
        volume: 0.35
    }

    readonly property FLSoundEffect check: FLSoundEffect {
        source: "qrc:/sfx/check"
        volume: 0.8
    }

    readonly property FLSoundEffect uncheck: FLSoundEffect {
        source: "qrc:/sfx/uncheck"
        volume: 0.8
    }

    readonly property FLSoundEffect activateGame: FLSoundEffect {
        source: "qrc:/sfx/activate-game"
        volume: 0.7
    }

    readonly property FLSoundEffect showDialog: FLSoundEffect {
        source: "qrc:/sfx/show-dialog"
        volume: 0.4
    }

    readonly property FLSoundEffect startSliderMove: FLSoundEffect {
        source: "qrc:/sfx/sticky-small"
        volume: 0.75
    }

    readonly property FLSoundEffect stopSliderMove: FLSoundEffect {
        source: "qrc:/sfx/snap-medium"
        volume: 0.4
    }

    readonly property FLSoundEffect tabBarNavigate: FLSoundEffect {
        source: "qrc:/sfx/snap-medium"
        volume: 0.4
    }
}
