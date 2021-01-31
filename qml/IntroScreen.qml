import "." as GameOne
import QtQuick 2.15

GameOne.Screen {
    signal screenFinished()

    color: "skyblue"

    Text {
        id:info1

        color: "orange"

        font { pixelSize: 25; italic: true }
        x: 30
        y: 25

        text: "You move with the <b>arrow keys</b>. The <b>red rectangles</b> are enemies."
    }

    Text {
        id:info2

        color: "orange"

        font { pixelSize: 25; italic: true }
        x: 30
        y: 30 + 25

        text: "You must attack them with running into them, but you can´t"
//               The enemies will attack you also.
/*               The <b>circle</b> is the*/
    }

    Text {
        id: info3
        color: "orange"

        font { pixelSize: 25; italic: true }
        x: 30
        y: 60 + 25

        text: "move into them. You will stay at your field, but the enemy takes"
//demage."
    }

    Text {
        id: info4

        color: "orange"

        font { pixelSize: 25; italic: true }
        x: 30
        y: 90 + 25

        text: "demage. The enemies will <b>attack you also</b>. The <b>white rectangle</b>"
    }

    Text {
        id: info5

        color: "orange"

        font { pixelSize: 25; italic: true }
        x: 30
        y: 120 + 25

        text: "is the boss opponent."
    }
    Text {
        id: disapearedLevelsHelp

        color: "orange"

        font { pixelSize: 25; italic: true }
        x: 30
        y: 145 + 25

        text: "If you don´t see the level, please play in fullscreen-mode or change the position of the window."
    }

    Text {
        id: requestToContinue

        anchors.centerIn: parent

        text: "Press again to continue and starting game"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: screenFinished()
    }
}
