import QtQuick 2.15

Item {
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
                    border.width: 1

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
