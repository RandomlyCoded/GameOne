import GameOne 1.0
import QtQuick 2.15

//Item {
Rectangle {
    id: sidebar

    enum Detail {
        Inventory,
        Levels
    }

    property int currentDetail: Sidebar.Detail.Inventory

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

            text: "lives: %1".arg(Backend && Backend.player.lives || 0)
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
            visible: Backend && !Backend.player.isAlive

            text: "press space to"
        }

        Text {
            id: gameOver2_2

            color: "#afafaf"
            font.pixelSize: 25
            visible: Backend && !Backend.player.isAlive

            text: "respawn."
        }

        /*
        Text {
            color: "#afafaf"
            font.pixelSize: 25
            text: Backend.levelFileName
            width: parent.width
            wrapMode: Text.Wrap
        }

        Text {
            color: "#afafaf"
            font.pixelSize: 25
            text: Backend.levelName
            width: parent.width
            wrapMode: Text.Wrap
        }
        */

        Button {
            text: "Respawn"
            width: parent.width

            onActivated: Backend.respawn()
        }

        Button {
            text: "Inventory"
            width: parent.width

            checked: sidebar.currentDetail === Sidebar.Detail.Inventory
            onActivated: sidebar.currentDetail = Sidebar.Detail.Inventory
        }

        Button {
            text: "Levels"
            width: parent.width

            checked: sidebar.currentDetail === Sidebar.Detail.Levels
            onActivated: sidebar.currentDetail = Sidebar.Detail.Levels
        }

        Item {
            width: parent.width
            height: parent.height - y

            GridView {
                clip: true
                width: parent.width
                height: parent.height * opacity

                model: Backend.player.inventory

                opacity: sidebar.currentDetail === Sidebar.Detail.Inventory ? 1 : 0
                Behavior on opacity { NumberAnimation {} }
                visible: opacity > 0

                cellWidth: Math.floor(width/3)
                cellHeight: cellWidth

                delegate: Item {
                    width: GridView.view.cellWidth
                    height: GridView.view.cellHeight

                    Image {
                        anchors.fill: parent
                        source: model.imageSource
                        sourceSize: Qt.size(width, height)
                    }

                    Text {
                        anchors.bottom: parent.bottom
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        width: parent.width

                        color: "#ffffff"
                        font { pixelSize: 11 }
                        style: Text.Outline
                        styleColor: "#80000000"
                        text: model.itemName
                    }

                    Rectangle {
                        id: amountBadge

                        anchors { right: parent.right; top: parent.top }
                        height: amountLabel.height + 2
                        width: Math.max(amountLabel.width + 2, height)
                        radius: height/2

                        color: "orange"
                    }

                    Text {
                        id: amountLabel

                        anchors.centerIn: amountBadge

                        color: "white"
                        font { bold: true; pixelSize: 14 }
                        style: Text.Outline
                        styleColor: "#000"

                        text: model.amount
                    }

                    MouseArea {
                        anchors.fill: parent

                        function returning() {
                            if(model.amount > 1)
                                return 1

                            else
                                return 0;
                        }

                        onClicked: console.info(model.amount + " " + model.itemName + [" is", "s are"][returning()] + " selected.")
                    }
                }
            }

            ListView {
                clip: true
                width: parent.width
                height: parent.height * opacity

                model: LevelModel {}

                opacity: sidebar.currentDetail === Sidebar.Detail.Levels ? 1 : 0
                Behavior on opacity { NumberAnimation {} }
                visible: opacity > 0

                delegate: Button {
                    property bool isCurrentLevel: model.fileName === Backend.levelFileName

                    borderColor: "transparent"
                    color: isCurrentLevel ? "white" : "transparent"
                    textColor: isCurrentLevel ? "black" : "white"

                    elide: Text.ElideRight
                    width: parent && parent.width || 0
                    wrapMode: Text.Wrap
                    text: model.levelName

                    onActivated: Backend.load(model.fileName)
                }
            }
        }
    }
}
