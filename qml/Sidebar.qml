import QtQuick 2.15

Item {
    Rectangle {
        id: sidebar

        color: "black"

        height: parent.height
        width: parent.width - GameGround.width

        Column {
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

                y: 40

                text: "Information:"
            }

            Text {
                id: info1_1

                color: "white"
                font.pixelSize: 25

                y: 65

                text: "red circles:"
            }

            Text {
                id: info1_2

                color: "white"
                font.pixelSize: 25

                y: 90

                text: "enemies"
            }

            Text {
                id: info2_1

                color: "white"
                font.pixelSize: 25

                y: 115

                text: "circle:"
            }

            Text {
                id: info2_2

                color: "white"
                font.pixelSize: 25

                y: 140

                text: "boss opponent"
            }

            Text {
                id: gameOver2_1

                color: "#afafaf"
                font.pixelSize: 25

                y: 200

                visible: backend && !backend.player.isAlive

                text: "press space to"
            }

            Text {
                id: gameOver2_2

                color: "#afafaf"
                font.pixelSize: 25

                y: 210

                visible: backend && !backend.player.isAlive

                text: "respawn."
            }


            Grid {
                id: levelButtonsGrid

                columns: 7
                columnSpacing: 5
                rows: 24

                Repeater {
                    id: levelButtons

                    model: 500

                    Button {
                        text: " Level %1".arg(modelData + 1)

                        onActivated: {
                            if (!backend.load("level%1.json".arg(modelData + 1)))
                                backend.load("map%1.txt".arg(modelData + 1));

                            console.info (parent.height, parent.width, parent.rows, parent.columns)
                        }
                    }
                }
            }

            Button {
                text: "Respawn"

                onActivated: backend.player.respawn()
            }
        }


        Text {
            id: game_over

            color: "red"

            font { pixelSize: 100; bold: true; italic: true }

            anchors.centerIn: parent
            visible: backend && backend.player.lives === 0

            text: "Game Over"
        }
    }
}
