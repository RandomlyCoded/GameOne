import GameOne 1.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

Window {
    id: animationTest

    readonly property var warmSea: allModes[0]

    readonly property var allModes: [
        {
            "name": "Warm Sea",
            "frameCount": 9,
            "imageSource": "panel/WarmSea.svg",
            "stages": ["show=background,frame(t),frame(t+1),frame(t+2)"]
        }, {
            "name": "Fire Ghost",
            "frameCount": 5,
            "imageSource": "enemies/FireGhost.svg",
            "stages": ["show=max,med_max.flames(t)", "show=med,med_max.flames(t)", "show=min,min.flames(t)"]
        }, {
            "name": "Characters",
            "frameCount": 1,
            "imageSource": "Characters.svg",
            "stages": ["show=*-human1,*-girl1", "show=*-human1,*-boy1"]
        }
    ]

    property int currentFrame: 0
    property var mode: warmSea

    readonly property int frameCount: mode && mode.frameCount || parseInt(customFrameCount.text)
    readonly property string imageSource: mode && mode.imageSource || customImageSource.text
    readonly property var stages: mode && mode.stages || customStages.text.split('\n')

    function frameNumber(i) {
        return (frameCount + currentFrame + (i || 0)) % frameCount;
    }

    function imageUrl(url) {
        return Backend.imageUrl(url, frameCount, currentFrame);
    }

    onFrameCountChanged: {
        let wasRunning = animation.running;
        animation.stop();

        currentFrame = frameNumber(currentFrame);
        animation.to = frameCount - 1;
        animation.running = wasRunning;
    }

    onModeChanged: {
        if (mode) {
            customFrameCount.text = mode.frameCount;
            customImageSource.text = mode.imageSource;
            customStages.text = mode.stages.join("\n");
        }
    }

    color: "black"
    width: 1200
    height: 900

    visibility: Window.Maximized
    visible: true

    NumberAnimation {
        id: animation

        target: animationTest
        property: "currentFrame"
        from: 0
        to: frameCount - 1
        running: false
        loops: Animation.Infinite
        duration: frameCount * 100
    }

    Column {
        anchors.centerIn: parent
        spacing: 20

        Column {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: mode === warmSea
            spacing: 20

            Repeater {
                model: [200, 100, 50]

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 50

                    Image {
                        source: "qrc:/GameOne/assets/panel/WarmSea.svg"
                        sourceSize: Qt.size(modelData, modelData)
                    }

                    Image {
                        source: "image://assets/panel/WarmSea.svg"
                        sourceSize: Qt.size(modelData, modelData)
                    }

                    Image {
                        source: imageUrl("image://assets/panel/WarmSea.svg?show=background,frame(t)")
                        sourceSize: Qt.size(modelData, modelData)
                    }

                    Image {
                        source: imageUrl("image://assets/panel/WarmSea.svg?show=background,frame(t),frame(t+1),frame(t+2)")
                        sourceSize: Qt.size(modelData, modelData)
                    }

                    Image {
                        source: imageUrl("image://assets/panel/WarmSea.svg?show=background,frame(t),frame(t+2),frame(t+4),frame(t+6)")
                        sourceSize: Qt.size(modelData, modelData)
                    }

                    Image {
                        source: imageUrl("image://assets/panel/WarmSea.svg?hide=frame(t),frame(t+1),frame(t+2)")
                        sourceSize: Qt.size(modelData, modelData)
                    }
                }
            }

            Grid {
                anchors.horizontalCenter: parent.horizontalCenter
                columns: 4
                visible: mode === warmSea

                Repeater {
                    model: parent.columns * parent.columns

                    Image {
                        source: imageUrl("image://assets/panel/WarmSea.svg?show=background,frame(t)")
                        sourceSize: Qt.size(59, 59)
                    }
                }
            }
        }

        Column {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: mode !== warmSea
            spacing: 20

            Repeater {
                model: [200, 100, 50]

                Row {
                    readonly property size imageSize: Qt.size(modelData, modelData)

                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 50

                    Image {
                        source: "qrc:/GameOne/assets/" + imageSource
                        sourceSize: Qt.size(modelData, modelData)
                    }

                    Repeater {
                        model: stages

                        Image {
                            source: imageUrl("image://assets/" + imageSource + "?" + modelData)
                            sourceSize: imageSize
                        }
                    }
                }
            }
        }

        Row {
            id: actions

            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 10

            Repeater {
                model: allModes

                Button {
                    checked: mode === modelData
                    text: modelData.name

                    onClicked: mode = modelData
                }
            }

            Button {
                text: "<"
                onClicked: currentFrame = frameNumber(-1)
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                color: "white"
                text: (currentFrame + 1) + "/" + frameCount
            }

            Button {
                text: ">"
                onClicked: currentFrame = frameNumber(1)
            }

            Button {
                text: "Animate"
                checked: animation.running
                onClicked: animation.running = !animation.running
            }
        }

        Grid {
            anchors.horizontalCenter: parent.horizontalCenter
            verticalItemAlignment: Grid.AlignVCenter
            spacing: 10
            columns: 2

            Text {
                color: "silver"
                text: "Frame Count:"
            }

            TextField {
                id: customFrameCount

                inputMask: "d99"
                readOnly: mode !== null
                width: 400
            }

            Text {
                color: "silver"
                text: "Image Source:"
            }

            TextField {
                id: customImageSource

                readOnly: mode !== null
                width: 400
            }

            Text {
                color: "silver"
                text: "Stages:"
            }

            TextArea {
                id: customStages

                background: Rectangle {
                    border { width: 2; color: parent.activeFocus ? parent.palette.highlight : parent.palette.window }
                    color: customStages.palette.window
                }

                readOnly: mode !== null
                width: 400
            }
        }
    }
}
