import QtQuick 2.15

Screen {
    signal battle

    color: "white"
    focus: true

    Connections {
        target: backend.player

        function onPositionChanged() {
            console.info("START");
            gameTimer.start();
        }

        function onLivesChanged() {
            console.info("You have died");
            gameTimer.stop();
        }
    }

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
                    color: "darkgreen"
                    position: 0.00
                }

                GradientStop {
                    color: "white"
                    position: 0.50
                }

                GradientStop {
                    color: "darkgreen"
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

                columns: backend.columns
                rows: backend.rows

                Repeater {
                    model: gameGrid.columns * gameGrid.rows

                    Rectangle {
                        id: cell

                        readonly property int column: modelData % gameGrid.columns
                        readonly property int row: modelData / gameGrid.rows

                        readonly property var actor: {
                            return backend.actors.find(actor => actor.isAlive
                                                       && actor.x === cell.column
                                                       && actor.y === cell.row);
                        }

                        readonly property string type: actor && actor.type || null

                        color: ({"Player": "lawngreen", "Enemy": "red", "both": "brown"}[type]) || "silver"
                        border.color: "black"
                        border.width: 2

                        width: 60
                        height: width

                        Text {
                            anchors.centerIn: parent
                            color: "black"
                            font.bold: true

                            text: cell.type || [cell.column, cell.row].join('/')
                        }

                        Rectangle {
                            border { width: 1; color: "#000000" }
                            color: "#80000000"

                            anchors.horizontalCenter: parent.horizontalCenter
                            y: 3
                            width: parent.width - 6
                            height: 10

                            visible: cell.actor && cell.actor.isAlive || false

                            Rectangle {
                                property real energyLevel: cell.actor ? cell.actor.energy / cell.actor.maximumEnergy : 0

                                x: parent.border.width
                                y: parent.border.width
                                height: parent.height - 2 * parent.border.width
                                width: (parent.width - 2 * parent.border.width) * energyLevel
                                color: "lawngreen"
                            }

                            Text {
                                color: "white"
                                font.pixelSize: 8
                                anchors.centerIn: parent
                                text: cell.actor ? "%1 / %2".arg(cell.actor.energy).arg(cell.actor.maximumEnergy) : ""
                            }
                        }
                    }
                }
            }
        }
    }

    Timer {
        id: gameTimer

        interval: 250 //(backend.player.x / backend.player.y)
        repeat: true

        onTriggered: {
            backend.enemies.forEach(enemy => enemy.act());
        }
    }

    Keys.onLeftPressed: backend.player.moveLeft()
    Keys.onRightPressed: backend.player.moveRight()
    Keys.onUpPressed: backend.player.moveUp()
    Keys.onDownPressed: backend.player.moveDown()

    Keys.onSpacePressed: if (!backend.player.isAlive) backend.player.respawn()
                         else if (backend.player.isAlive/* && backend.enemies.isAlive === false*/) backend.enemies.energy //Feinde wiederbeleben?

//    Keys.onDigit1Pressed: backend.player.attack()

    Keys.onEscapePressed: console.info(backend.player.isAlive, backend.player.lives)

    Column {
        Text {
            id: liveText

            color: "red"

            font.pixelSize: 25

            x: 40
            y: 7.5

            text: "lives: %1".arg(backend.player.lives)
        }

        Text {
            id: info

            color: "#6f6f6f"
            font.pixelSize: 25

            y: 40

            text: "Infos:"
        }

        Text {
            id: info1_1

            color: "white"
            font.pixelSize: 25

            y: 65

            text: "weiße Rechtecke:"
        }

        Text {
            id: info1_2

            color: "white"
            font.pixelSize: 25

            y: 90

            text: "Monster"
        }

        Text {
            id: info2_1

            color: "white"
            font.pixelSize: 25

            y: 115

            text: "Kreis:"
        }

        Text {
            id: info2_2

            color: "white"
            font.pixelSize: 25

            y: 140

            text: "Bossgegner"
        }

        Text {
            id: gameOver2_1

            color: "#6f6f6f"
            font.pixelSize: 25

            y: 200

            visible: !backend.player.isAlive

            text: "Leerstaste drücken,"
        }

        Text {
            id: gameOver2_2

            color: "#6f6f6f"
            font.pixelSize: 25

            y: 210

            visible: !backend.player.isAlive

            text: "um neuzustarten"
        }
    }


    Text {
        id: game_over

        color: "red"

        font { pixelSize: 100; bold: true; italic: true }

        anchors.centerIn: parent
        visible: backend.player.lives === 0

        text: "Game Over"
    }

    MouseArea {
        anchors.fill: parent

        onClicked: {
            if (!backend.player.isAlive)
                backend.player.respawn();
        }
    }

//Hier geht´s weiter!
}
