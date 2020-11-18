import GameOne 1.0
import QtQuick 2.15

// bitte in den IntroScreen gucken!

Screen {
    property int levelCount: 11
    signal battle

    color: "white"
    focus: true

    Row {
        anchors.fill: parent

        Rectangle {
            id: sidebar

            color: "black"

            height: parent.height
            width: parent.width - playground.width
        }

        Rectangle {
            id: playground

            height: parent.height
            width: height

            gradient: Gradient {
                GradientStop {
                    color: "white"
                    position: 0.00
                }

                GradientStop {
                    color: "green"
                    position: 0.50
                }

                GradientStop {
                    color: "yellow"
                    position: 1.00
                }
            }

//            Debug {
//                value: [backend]
//            }

            Grid {
                id: gameGrid

                anchors.centerIn: parent
                spacing: 0

                columns: backend && backend.columns || 0
                rows: backend && backend.rows || 0

                Repeater {
                    model: backend && backend.map || 0

                    Rectangle {
                        id: cell

                        readonly property var actor: {
                            return backend && backend.actors.find(
                                        actor => actor.isAlive
                                        && actor.x === model.column
                                        && actor.y === model.row);
                        }

                        function colorOf(type) {
                            return {
                                "Player": "silver",
                                "Enemy": "red",
                                "Hill": "brown",
                                "Mountain": "gray",
                                "DeepWater": "darkblue",
                                "Water": "blue",
                                "Grass": "lawngreen",
                                "Tree": "darkgreen",
                                "Sand": "#ffff60",
                                "Ice": "white",
                                "Fence": "saddlebrown",
                                "Lava": "#cd0000",
                            }[type] || "white";
                        }

                        color: colorOf(model.type)

                        border.color: "black"
                        border.width: 2

                        width: 60
                        height: width

                        Rectangle {
                            anchors.centerIn: parent

                            radius: parent.width/2 - 4
                            width: 2 * radius
                            height: 2 * radius

                            color: colorOf(model.item)
                            visible: !!model.item
                        }

                        Rectangle {
                            anchors.centerIn: parent

                            radius: parent.width/2 - 4
                            width: 2 * radius
                            height: 2 * radius

                            color: colorOf(actor && actor.type)
                            visible: !!actor
                        }

                        Rectangle {
                            border { width: 1; color: "#000000" }
                            color: "#800000000"

                            anchors.horizontalCenter: parent.horizontalCenter
                            y: 3
                            width: parent.width - 6
                            height: 10

                            visible: cell.actor && cell.actor.isAlive || false

                            Rectangle {
                                id: energyBar

                                property real energyLevel: cell.actor ? cell.actor.energy / cell.actor.maximumEnergy : 0

                                x: parent.border.width
                                y: parent.border.width
                                height: parent.height - 2 * parent.border.width
                                width: (parent.width - 2 * parent.border.width) * energyLevel
                                color: "lawngreen"
                            }

                            Text {
                                color: "black"
                                font.pixelSize: 8
                                font.bold: true
                                anchors.centerIn: parent
                                text: cell.actor ? "%1 / %2".arg(cell.actor.energy).arg(cell.actor.maximumEnergy) : ""
                            }
                        }
                    }
                }
            }
        }
    }

    Keys.onLeftPressed: { backend.player.moveLeft() }
    Keys.onRightPressed: { backend.player.moveRight() }
    Keys.onUpPressed: { backend.player.moveUp() }
    Keys.onDownPressed: { backend.player.moveDown() }

    Keys.onSpacePressed: if (!backend.player.isAlive) backend.player.respawn()
    Keys.onEscapePressed: console.info("isAlive:", backend.player.isAlive, ";",
                                       "lives:", backend.player.lives, ";",
                                       "energy:", backend.player.energy)

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

            text: "red rectangles:"
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

        Repeater {
            model: levelCount

            Button {
                text: "Level %1".arg(modelData + 1)

                onActivated: {
                    if (!backend.load("level%1.json".arg(modelData + 1)))
                        backend.load("map%1.txt".arg(modelData + 1));
                }
            }
        }

        Repeater {
            model: 10

            Keys.onDigit%1Pressed.arg(modelData + 1): {
                if (!backend.load("level%1.json".arg(modelData + 1)))
                    backend.load("map%1.txt".arg(modelData + 1));
            }
        }

        Button {
            text: "Respawn"
            onActivated: backend.player.respawn()
        }

//        Debug {
//            value: [backend.map.columns, backend.map.rows]
//        }

//        Debug {
//            value: [gameGrid.columns, gameGrid.rows]
//        }
    }


    Text {
        id: game_over

        color: "red"

        font { pixelSize: 100; bold: true; italic: true }

        anchors.centerIn: parent
        visible: backend && backend.player.lives === 0

        text: "Game Over"
    }

//Continue here!
}
