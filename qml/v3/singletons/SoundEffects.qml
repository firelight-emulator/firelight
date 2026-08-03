pragma Singleton
import QtQuick

QtObject {
    id: root

    readonly property bool enabled: true

    readonly property FLSoundEffect cursorBump: FLSoundEffect {
        source: "qrc:/sfx/bump"
        volume: 0.5
    }

    readonly property FLSoundEffect openPopup: FLSoundEffect {
        source: "qrc:/sfx/show-modal"
    }

    readonly property FLSoundEffect closePopup: FLSoundEffect {
        source: "qrc:/sfx/hide-modal"
    }

    readonly property FLSoundEffect menuNavigate: FLSoundEffect {
        source: "qrc:/sfx/blip"
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
    }
}
