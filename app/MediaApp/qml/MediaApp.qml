import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
// HeadUnit 모듈 제거: C++ backend는 contextProperty로 노출됨

// MediaApp 독립 실행용 메인 윈도우
// Content area: right of GearApp, above navigation bar
Window {
    id: window
    width: 1790  // 1920 - 130
    height: 1000  // 1080 - 80
    visible: true
    visibility: Window.Maximized
    title: "MediaApp"
    flags: Qt.Window | Qt.FramelessWindowHint

    Rectangle {
        id: root
        anchors.fill: parent

        // Background gradient layer with brightness control (matching AmbientApp)
        Rectangle {
            id: backgroundGradient
            anchors.fill: parent

            // Get color and brightness from ambientTheme
            property color displayColor: ambientTheme ? ambientTheme.ambientColor : "#3498db"
            property real brightnessValue: ambientTheme ? ambientTheme.brightness : 0.8

            // Calculate brightness-adjusted color
            property color brightColor: Qt.rgba(
                displayColor.r * brightnessValue,
                displayColor.g * brightnessValue,
                displayColor.b * brightnessValue,
                1.0
            )

            // Same gradient as AmbientApp
            gradient: Gradient {
                GradientStop {
                    position: 0.0
                    color: Qt.lighter(backgroundGradient.brightColor, 1.3)
                }
                GradientStop {
                    position: 0.5
                    color: backgroundGradient.brightColor
                }
                GradientStop {
                    position: 1.0
                    color: Qt.darker(backgroundGradient.brightColor, 1.5)
                }
            }

            // Smooth transitions
            Behavior on brightColor {
                ColorAnimation { duration: 300 }
            }
        }

    // Use actual media manager properties
    property bool isPlaying: mediaManager.isPlaying
    property string currentSong: mediaManager.currentFile
    property int currentIndex: mediaManager.currentIndex
    property var mediaFiles: mediaManager.mediaFiles

    // Function to get just the filename from full path
    function getFileName(filePath) {
        if (!filePath) return "No song selected"
        var parts = filePath.split('/')
        return parts[parts.length - 1]
    }

    // 컴포넌트 로드 시 자동으로 미디어 스캔
    Component.onCompleted: {
        console.log("MediaApp: Starting auto scan...")
        autoScanTimer.start()
    }

    // 자동 스캔 타이머
    Timer {
        id: autoScanTimer
        interval: 1000
        onTriggered: {
            // UsbMedia 스타일의 전체 USB 스캔 사용
            var files = mediaManager.scanAllUsbMounts()
            console.log("Auto scan completed. Found", files.length, "media files")
        }
    }

    // USB Controls Section
    Row {
        id: usbControls
        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 20  // 콤보박스와 refresh 버튼 사이 간격 증가 (10 -> 20)

        ComboBox {
            id: mountBox
            width: 350  // 콤보박스 너비 증가 (250 -> 350)
            model: mediaManager.usbMounts
            displayText: currentText || "No USB Device"
            background: Rectangle {
                color: "#3498db"
                radius: 5
            }
            contentItem: Text {
                text: mountBox.displayText
                color: "white"
                leftPadding: 10
                rightPadding: 10
                verticalAlignment: Text.AlignVCenter
            }
        }

        // Refresh Button with SVG icon
        Rectangle {
            width: 50
            height: 50
            color: "transparent"

            Image {
                id: refreshIcon
                anchors.centerIn: parent
                source: "qrc:/images/refresh.svg"
                sourceSize.width: 50
                sourceSize.height: 50
                fillMode: Image.PreserveAspectFit
                visible: true
            }

            MouseArea {
                id: refreshMouseArea
                anchors.fill: parent
                onClicked: {
                    console.log("Refreshing USB mounts...")
                    mediaManager.refreshUsbMounts()
                    // 짧은 지연 후 자동으로 스캔
                    refreshScanTimer.start()
                }

                onPressed: parent.scale = 0.9
                onReleased: parent.scale = 1.0
            }

            Behavior on scale {
                NumberAnimation { duration: 100 }
            }

            Timer {
                id: refreshScanTimer
                interval: 500
                onTriggered: {
                    var files = mediaManager.scanAllUsbMounts()
                    console.log("Refresh scan completed. Found", files.length, "media files")
                }
            }
        }
    }

    Row {
        anchors.top: usbControls.bottom
        anchors.topMargin: 20
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        spacing: 20

        // Media List
        Rectangle {
            width: parent.width * 0.6
            height: parent.height
            color: "#2c3e50"
            radius: 10

            Column {
                anchors.fill: parent
                anchors.margins: 10

                Text {
                    text: "USB Media Files"
                    font.pixelSize: 18
                    font.bold: true
                    color: "#ecf0f1"
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Rectangle {
                    width: parent.width
                    height: 2
                    color: "#3498db"
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                ListView {
                    width: parent.width
                    height: parent.height - 40
                    model: root.mediaFiles

                    delegate: Rectangle {
                        width: parent.width
                        height: 50
                        color: root.currentIndex === index ? "#3498db" : "transparent"
                        radius: 5

                        Text {
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            anchors.verticalCenter: parent.verticalCenter
                            text: root.getFileName(modelData)
                            color: "#ecf0f1"
                            font.pixelSize: 14
                            elide: Text.ElideRight
                            width: parent.width - 20
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                mediaManager.playFile(index)
                                console.log("Playing: " + modelData)
                            }
                        }
                    }
                }
            }
        }

        // Player Controls
        Rectangle {
            width: parent.width * 0.35
            height: parent.height
            color: "#2c3e50"
            radius: 10

            Column {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 20

                Text {
                    text: "Now Playing"
                    font.pixelSize: 18
                    font.bold: true
                    color: "#ecf0f1"
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Rectangle {
                    width: parent.width
                    height: 60
                    color: "#34495e"
                    radius: 5
                    border.color: "#3498db"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: root.getFileName(root.currentSong)
                        color: "#ecf0f1"
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        anchors.margins: 10
                        width: parent.width - 20
                    }
                }

                // Control Buttons
                Row {
                    spacing: 15
                    anchors.horizontalCenter: parent.horizontalCenter

                    // Previous Button (Backward)
                    Rectangle {
                        width: 50
                        height: 50
                        color: "transparent"

                        Image {
                            id: backwardIcon
                            anchors.centerIn: parent
                            source: "qrc:/images/backward.svg"
                            sourceSize.width: 50
                            sourceSize.height: 50
                            fillMode: Image.PreserveAspectFit
                            visible: true
                        }

                        MouseArea {
                            id: backwardMouseArea
                            anchors.fill: parent
                            onClicked: {
                                mediaManager.previous()
                                console.log("Previous track")
                            }

                            onPressed: parent.scale = 0.9
                            onReleased: parent.scale = 1.0
                        }

                        Behavior on scale {
                            NumberAnimation { duration: 100 }
                        }
                    }

                    // Play/Pause Button
                    Rectangle {
                        width: 60
                        height: 60
                        color: "transparent"

                        Image {
                            id: playPauseIcon
                            anchors.centerIn: parent
                            source: root.isPlaying ? "qrc:/images/pause.svg" : "qrc:/images/start.svg"
                            sourceSize.width: 60
                            sourceSize.height: 60
                            fillMode: Image.PreserveAspectFit
                            visible: true
                        }

                        MouseArea {
                            id: playPauseMouseArea
                            anchors.fill: parent
                            onClicked: {
                                console.log("Play/Pause clicked. Current isPlaying state:", root.isPlaying)
                                if (root.isPlaying) {
                                    console.log("Calling pause()")
                                    mediaManager.pause()
                                } else {
                                    console.log("Calling play()")
                                    mediaManager.play()
                                }
                            }

                            onPressed: parent.scale = 0.9
                            onReleased: parent.scale = 1.0
                        }

                        Behavior on scale {
                            NumberAnimation { duration: 100 }
                        }
                    }

                    // Next Button (Forward)
                    Rectangle {
                        width: 50
                        height: 50
                        color: "transparent"

                        Image {
                            id: forwardIcon
                            anchors.centerIn: parent
                            source: "qrc:/images/forward.svg"
                            sourceSize.width: 50
                            sourceSize.height: 50
                            fillMode: Image.PreserveAspectFit
                            visible: true
                        }

                        MouseArea {
                            id: forwardMouseArea
                            anchors.fill: parent
                            onClicked: {
                                mediaManager.next()
                                console.log("Next track")
                            }

                            onPressed: parent.scale = 0.9
                            onReleased: parent.scale = 1.0
                        }

                        Behavior on scale {
                            NumberAnimation { duration: 100 }
                        }
                    }
                }

                // Volume Control
                Column {
                    width: parent.width
                    spacing: 10

                    Text {
                        text: "Volume: " + Math.round(mediaManager.volume * 100) + "%"
                        font.pixelSize: 16
                        color: "#ecf0f1"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Slider {
                        width: parent.width
                        from: 0
                        to: 100
                        value: mediaManager.volume * 100  // Convert 0.0-1.0 to 0-100 for display
                        stepSize: 1  // 1% 단위로 조절
                        anchors.horizontalCenter: parent.horizontalCenter

                        onValueChanged: {
                            mediaManager.volume = value / 100.0  // Convert back to 0.0-1.0 range
                        }

                        background: Rectangle {
                            x: parent.leftPadding
                            y: parent.topPadding + parent.availableHeight / 2 - height / 2
                            implicitWidth: 200
                            implicitHeight: 4
                            width: parent.availableWidth
                            height: implicitHeight
                            radius: 2
                            color: "#7f8c8d"
                        }

                        handle: Rectangle {
                            x: parent.leftPadding + parent.visualPosition * (parent.availableWidth - width)
                            y: parent.topPadding + parent.availableHeight / 2 - height / 2
                            implicitWidth: 20
                            implicitHeight: 20
                            radius: 10
                            color: "#3498db"
                        }
                    }
                }
            }
        }
    }

    } // Rectangle (root)
} // Window
