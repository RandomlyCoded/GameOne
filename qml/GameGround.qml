import QtQuick 2.15

Item {
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

                readonly property var actor: {
                    return backend && backend.actors.find(
                                actor => actor.x === model.column
                                && actor.y === model.row);
                }

                color: model.tileColor
                clip: true

                border.color: "black"
                border.width: 1

                width: 60
                height: width

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
                    visible: !actor && model.item && !itemImage.visible || false
                }

                Image {
                    id: actorImage

                    anchors.fill: parent
                    sourceSize: Qt.size(width, height)
                    visible: source.toString()

                    rotation: {
                        if (actor && actor.isAlive && actor.rotationSteps > 1)
                            return 360 * (backend.ticks % actor.rotationSteps) / actor.rotationSteps;

                        return 0;
                    }

                    source: {
                        if (actor && actor.isAlive)
                            return backend.imageUrl(actor.imageSource, actor.imageCount, backend.ticks);

                        return "";
                    }
                }

                Rectangle {
                    id: actorCircle

                    anchors.centerIn: parent

                    radius: parent.width/2 - 4
                    width: 2 * radius
                    height: 2 * radius

                    color: actor && actor.isAlive && actor.color || ""
                    visible: actor && actor.isAlive && !actorImage.visible || false
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

                    visible: cell.actor && cell.actor.energyVisible && cell.actor.isAlive || false

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

                Text {
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 3
                    anchors.horizontalCenter: parent.horizontalCenter

                    color: "#fff"
                    font.pixelSize: 12
                    style: Text.Outline
                    styleColor: "#80000000"

                    text: cell.actor && cell.actor.name || ""
                    visible: cell.actor && cell.actor.isAlive && cell.actor.name || false
                }
            }
        }
    }
}
