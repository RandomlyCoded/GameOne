import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

Window {
    id: animationTest

    readonly property var warmSea: {
        "frameCount": 9,
        "imageSource": "panel/WarmSea.svg",
        "stages": ["show=background,frame%1"]
    }

    readonly property var fireGhost: {
        "frameCount": 5,
        "imageSource": "enemies/FireGhost.svg",
        "stages": ["show=max,med_max.flames%1", "show=med,med_max.flames%1", "show=min,min.flames%1"]
    }

    property int currentFrame: 0
    property var mode: warmSea

    readonly property int frameCount: mode && mode.frameCount || parseInt(customFrameCount.text)
    readonly property string imageSource: mode && mode.imageSource || customImageSource.text
    readonly property var stages: mode && mode.stages || customStages.text.split('\n')

    function frameNumber(i) {
        return (frameCount + currentFrame + (i || 0)) % frameCount;
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
                        source: "image://assets/panel/WarmSea.svg?show=background,frame%1".arg(frameNumber())
                        sourceSize: Qt.size(modelData, modelData)
                    }

                    Image {
                        source: "image://assets/panel/WarmSea.svg?show=background,frame%1,frame%2,frame%3&debug".arg(frameNumber(0)).arg(frameNumber(1)).arg(frameNumber(2))
                        sourceSize: Qt.size(modelData, modelData)
                    }

                    Image {
                        source: "image://assets/panel/WarmSea.svg?show=background,frame%1,frame%2,frame%3,frame%4".arg(frameNumber(0)).arg(frameNumber(2)).arg(frameNumber(4)).arg(frameNumber(6))
                        sourceSize: Qt.size(modelData, modelData)
                    }

                    Image {
                        source: "image://assets/panel/WarmSea.svg?hide=frame%1,frame%2,frame%3".arg(frameNumber(0)).arg(frameNumber(1)).arg(frameNumber(2))
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
                        source: "image://assets/panel/WarmSea.svg?show=background,frame%1".arg(frameNumber())
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
                        source: "qrc:/GameOne/assets/%1".arg(imageSource)
                        sourceSize: Qt.size(modelData, modelData)
                    }

                    Repeater {
                        model: stages

                        Image {
                            source: "image://assets/%1?%2".arg(imageSource).arg(modelData.arg(frameNumber()))
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

            Button {
                checked: mode === warmSea
                text: "Warm Sea"

                onClicked: mode = warmSea
            }

            Button {
                checked: mode === fireGhost
                text: "Fire Ghost"

                onClicked: mode = fireGhost
            }

            Button {
                checked: mode === null
                text: "Custom"

                onClicked: mode = null
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
