import GameOne 1.0

import QtQuick 2.15

Item {
    id: gameGround

    property real cellSize: 60

    clip: true

    Rectangle {
        id: playground

        anchors.fill: parent

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

                color: model.tileColor
                clip: true

                border.color: "black"
                border.width: 1

                width: gameGround.cellSize
                height: gameGround.cellSize

                Image {
                    id: tileImage

                    anchors.fill: parent
                    source: backend.imageUrl(model.tileImageSource, model.tileImageCount, backend.ticks)
                    sourceSize: Qt.size(width, height)
                    visible: source.toString()
                }

                Image {
                    id: itemImage

                    anchors.fill: parent
                    source: backend.imageUrl(model.itemImageSource, model.itemImageCount, backend.ticks)
                    sourceSize: Qt.size(width, height)
                    visible: source.toString()
                }

                Rectangle {
                    id: itemCircle

                    anchors.centerIn: parent

                    radius: parent.width/2 - 4
                    width: 2 * radius
                    height: 2 * radius

                    color: model.itemColor
                    visible: model.itemType && !model.isStart && !itemImage.visible || false
                }
            }
        }
    }

    Item {
        id: actorGrid

        anchors.fill: gameGrid

        Repeater {
            model: backend.actors

            Item {
                id: actorView

                readonly property Actor actor: modelData

                x: actorView.actor.x * width
                y: actorView.actor.y * height

                width: gameGround.cellSize
                height: gameGround.cellSize

                visible: actorView.actor.isAlive

                Image {
                    id: actorImage

                    anchors.fill: parent
                    sourceSize: Qt.size(width, height)
                    visible: source.toString()

                    rotation: {
                        if (actorView.actor.rotationSteps > 1) {
                            let step = backend.ticks % actorView.actor.rotationSteps;
                            return 360 * step / actorView.actor.rotationSteps;
                        }

                        return 0;
                    }

                    source: backend.imageUrl(actorView.actor.imageSource,
                                             actorView.actor.imageCount,
                                             backend.ticks)
                }

                Rectangle {
                    id: actorCircle

                    anchors.centerIn: parent

                    radius: parent.width/2 - 4
                    width: 2 * radius
                    height: 2 * radius

                    color: actorView.actor.color
                    visible: !actorImage.visible
                }

                Rectangle {
                    id: energyIndicator

                    border { width: 1; color: "#000000" }
                    color: "#800000000"

                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 3

                    width: parent.width - 6
                    height: 10

                    visible: actorView.actor.energyVisible

                    Rectangle {
                        id: energyBar

                        property real level: actorView.actor.energy / actorView.actor.maximumEnergy

                        x: parent.border.width
                        y: parent.border.width
                        height: parent.height - 2 * parent.border.width
                        width: (parent.width - 2 * parent.border.width) * level
                        color: "lawngreen"
                    }

                    Text {
                        color: "black"
                        font.pixelSize: 8
                        font.bold: true
                        anchors.centerIn: parent
                        text: "%1 / %2".arg(actorView.actor.energy).arg(actorView.actor.maximumEnergy)
                    }
                }

                Text {
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 3
                    anchors.horizontalCenter: parent.horizontalCenter

                    color: "#fff"
                    font.pixelSize: 12
                    style: Text.Outline
                    styleColor: "#80000000"

                    text: actorView.actor.name
                    visible: text
                }
            }
        }
    }
}
