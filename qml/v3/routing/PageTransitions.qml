// TODO: NEEDS REVIEW
pragma Singleton

import QtQuick
import Firelight 1.0

// TODO
// The named enter/exit animations a page move can be given. A rule in routing.js names one of these;
// a stack builds the pair it needs through createEnter/createExit. Nothing here touches the focus
// ring — the stack drives that once per move
QtObject {
    id: root

    readonly property Component pushEnter: Component {
        Transition {
            ParallelAnimation {
                PropertyAnimation {
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 160
                    easing.type: Easing.InOutQuad
                }
                PropertyAnimation {
                    property: "x"
                    from: 20
                    to: 0
                    duration: 160
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }

    readonly property Component pushExit: Component {
        Transition {
            ParallelAnimation {
                PropertyAnimation {
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: 160
                    easing.type: Easing.InOutQuad
                }
                PropertyAnimation {
                    property: "x"
                    from: 0
                    to: -20
                    duration: 160
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }

    readonly property Component popEnter: Component {
        Transition {
            ParallelAnimation {
                PropertyAnimation {
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 160
                    easing.type: Easing.InOutQuad
                }
                PropertyAnimation {
                    property: "x"
                    from: -20
                    to: 0
                    duration: 160
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }

    readonly property Component popExit: Component {
        Transition {
            ParallelAnimation {
                PropertyAnimation {
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: 160
                    easing.type: Easing.InOutQuad
                }
                PropertyAnimation {
                    property: "x"
                    from: 0
                    to: 20
                    duration: 160
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }

    readonly property Component replaceEnter: Component {
        Transition {
            SequentialAnimation {
                PropertyAction {
                    property: "opacity"
                    value: 0
                }
                PauseAnimation {
                    duration: AppStyle.durationBase
                }
                NumberAnimation {
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: AppStyle.durationBase
                    easing.type: AppStyle.easingStandard
                }
            }
        }
    }

    readonly property Component replaceExit: Component {
        Transition {
            NumberAnimation {
                property: "opacity"
                from: 1
                to: 0
                duration: AppStyle.durationBase
                easing.type: AppStyle.easingStandard
            }
        }
    }

    readonly property Component panelForwardEnter: Component {
        Transition {
            ParallelAnimation {
                NumberAnimation {
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: AppStyle.durationSlow
                    easing.type: AppStyle.easingStandard
                }
                NumberAnimation {
                    property: "x"
                    from: AppStyle.spacingSm
                    to: 0
                    duration: AppStyle.durationSlow
                    easing.type: AppStyle.easingStandard
                }
            }
        }
    }

    readonly property Component panelForwardExit: Component {
        Transition {
            ParallelAnimation {
                NumberAnimation {
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: AppStyle.durationSlow
                    easing.type: AppStyle.easingStandard
                }
                NumberAnimation {
                    property: "x"
                    from: 0
                    to: -AppStyle.spacingSm
                    duration: AppStyle.durationSlow
                    easing.type: AppStyle.easingStandard
                }
            }
        }
    }

    readonly property Component panelBackEnter: Component {
        Transition {
            ParallelAnimation {
                NumberAnimation {
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: AppStyle.durationSlow
                    easing.type: AppStyle.easingStandard
                }
                NumberAnimation {
                    property: "x"
                    from: -AppStyle.spacingSm
                    to: 0
                    duration: AppStyle.durationSlow
                    easing.type: AppStyle.easingStandard
                }
            }
        }
    }

    readonly property Component panelBackExit: Component {
        Transition {
            ParallelAnimation {
                NumberAnimation {
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: AppStyle.durationSlow
                    easing.type: AppStyle.easingStandard
                }
                NumberAnimation {
                    property: "x"
                    from: 0
                    to: AppStyle.spacingSm
                    duration: AppStyle.durationSlow
                    easing.type: AppStyle.easingStandard
                }
            }
        }
    }

    readonly property Component sectionDownEnter: Component {
        Transition {
            SequentialAnimation {
                PropertyAction {
                    property: "opacity"
                    value: 0
                }
                PauseAnimation {
                    duration: AppStyle.durationFast
                }
                ParallelAnimation {
                    NumberAnimation {
                        property: "opacity"
                        from: 0
                        to: 1
                        duration: AppStyle.durationBase
                        easing.type: AppStyle.easingStandard
                    }
                    NumberAnimation {
                        property: "y"
                        from: AppStyle.spacingLg
                        to: 0
                        duration: AppStyle.durationBase
                        easing.type: AppStyle.easingStandard
                    }
                }
            }
        }
    }

    readonly property Component sectionDownExit: Component {
        Transition {
            ParallelAnimation {
                NumberAnimation {
                    property: "opacity"
                    to: 0
                    duration: AppStyle.durationFast
                    easing.type: AppStyle.easingStandard
                }
                NumberAnimation {
                    property: "y"
                    to: -AppStyle.spacingLg
                    duration: AppStyle.durationBase
                    easing.type: AppStyle.easingStandard
                }
            }
        }
    }

    readonly property Component sectionUpEnter: Component {
        Transition {
            SequentialAnimation {
                PropertyAction {
                    property: "opacity"
                    value: 0
                }
                PauseAnimation {
                    duration: AppStyle.durationFast
                }
                ParallelAnimation {
                    NumberAnimation {
                        property: "opacity"
                        from: 0
                        to: 1
                        duration: AppStyle.durationBase
                        easing.type: AppStyle.easingStandard
                    }
                    NumberAnimation {
                        property: "y"
                        from: -AppStyle.spacingLg
                        to: 0
                        duration: AppStyle.durationBase
                        easing.type: AppStyle.easingStandard
                    }
                }
            }
        }
    }

    readonly property Component sectionUpExit: Component {
        Transition {
            ParallelAnimation {
                NumberAnimation {
                    property: "opacity"
                    to: 0
                    duration: AppStyle.durationFast
                    easing.type: AppStyle.easingStandard
                }
                NumberAnimation {
                    property: "y"
                    to: AppStyle.spacingLg
                    duration: AppStyle.durationBase
                    easing.type: AppStyle.easingStandard
                }
            }
        }
    }

    readonly property var _presets: ({
            "push": {
                "enter": root.pushEnter,
                "exit": root.pushExit
            },
            "pop": {
                "enter": root.popEnter,
                "exit": root.popExit
            },
            "replace": {
                "enter": root.replaceEnter,
                "exit": root.replaceExit
            },
            "panelForward": {
                "enter": root.panelForwardEnter,
                "exit": root.panelForwardExit
            },
            "panelBack": {
                "enter": root.panelBackEnter,
                "exit": root.panelBackExit
            },
            "sectionDown": {
                "enter": root.sectionDownEnter,
                "exit": root.sectionDownExit
            },
            "sectionUp": {
                "enter": root.sectionUpEnter,
                "exit": root.sectionUpExit
            }
        })

    // TODO
    // Whether a preset of this name exists. "none" deliberately has none: a move with no animation
    // never reaches a Transition
    function has(name: string): bool {
        return root._presets[name] !== undefined;
    }

    function createEnter(name: string, parent: QtObject): QtObject {
        if (!root.has(name)) {
            console.warn("No page transition preset named " + name);
            return null;
        }

        return root._presets[name].enter.createObject(parent);
    }

    function createExit(name: string, parent: QtObject): QtObject {
        if (!root.has(name)) {
            console.warn("No page transition preset named " + name);
            return null;
        }

        return root._presets[name].exit.createObject(parent);
    }
}
