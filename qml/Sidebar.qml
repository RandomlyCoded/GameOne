import GameOne 1.0

import QtQuick 2.15

//Item {
Rectangle {
    id: sidebar

    color: "#6f6f6f"

    height: parent.height
    width: parent.width - GameGround.width

    Column {
        anchors { fill: parent; margins: 10 }
        spacing: 10

        Text {
            id: liveText

            color: "red"

            font.pixelSize: 25

            x: 40
            y: 7.5

            text: "lives: %1".arg(backend && backend.player.lives || 0)
        }

        Text {
            id: info

            color: "#afafaf"
            font.pixelSize: 25
            text: "Information:"
        }

        Text {
            id: info1_1

            color: "white"
            font.pixelSize: 25
            text: "red circles:"
        }

        Text {
            id: info1_2

            color: "white"
            font.pixelSize: 25
            text: "enemies"
        }

        Text {
            id: gameOver2_1

            color: "#afafaf"
            font.pixelSize: 25
            visible: backend && !backend.player.isAlive

            text: "press space to"
        }

        Text {
            id: gameOver2_2

            color: "#afafaf"
            font.pixelSize: 25
            visible: backend && !backend.player.isAlive

            text: "respawn."
        }

        /*
        Text {
            color: "#afafaf"
            font.pixelSize: 25
            text: backend.levelFileName
            width: parent.width
            wrapMode: Text.Wrap
        }

        Text {
            color: "#afafaf"
            font.pixelSize: 25
            text: backend.levelName
            width: parent.width
            wrapMode: Text.Wrap
        }
        */

        Button {
            text: "Respawn"

            onActivated: backend.respawn()
        }

        ListView {
            clip: true
            width: parent.width
            height: parent.height - y

            model: LevelModel {}

            delegate: Button {
                property bool isCurrentLevel: model.fileName === backend.levelFileName

                color: isCurrentLevel ? "white" : "transparent"
                textColor: isCurrentLevel ? "black" : "white"

                elide: Text.ElideRight
                width: parent.width
                text: model.levelName

                onActivated: backend.load(model.fileName)
            }
        }

    }
}
