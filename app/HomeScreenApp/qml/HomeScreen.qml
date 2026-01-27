import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0

// HomeScreen Dashboard - Displays aggregated info from services
// Content area: right of GearApp, above navigation bar
Window {
    id: window
    width: 1790  // 1920 - 130
    height: 1000  // 1080 - 80
    visible: true
    visibility: Window.Maximized
    title: "HomeScreen"
    flags: Qt.Window | Qt.FramelessWindowHint

    Rectangle {
        id: homePage
        anchors.fill: parent
        color: "transparent"

        // Background gradient matching AmbientApp style
        Rectangle {
            id: backgroundGradient
            anchors.fill: parent

            // Get color and brightness from manager
            property color displayColor: homeScreenManager.ambientColor
            property real brightnessValue: homeScreenManager.ambientBrightness

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

        // Center vehicle visualization
        Item {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: -30
            width: 500
            height: 330

            // Vehicle image
            Image {
                id: carImage
                anchors.centerIn: parent
                width: 400
                height: 280
                source: "qrc:/asset/car.svg"
                fillMode: Image.PreserveAspectFit
                smooth: true
                visible: true
                opacity: 0.9
            }
        }

        // "Now Playing" card (bottom center)
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottomMargin: 30
            width: 320
            height: 100
            radius: 15
            color: "#2c3e50"
            opacity: 0.95

            Column {
                anchors.centerIn: parent
                spacing: 10

                // Now Playing title
                Text {
                    text: "Now Playing"
                    font.pixelSize: 14
                    color: "#95a5a6"
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                // Song name (from MediaControl service)
                Text {
                    id: nowPlayingSong
                    text: homeScreenManager.currentTrack
                    font.pixelSize: 18
                    font.bold: true
                    color: "#ecf0f1"
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 280
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignHCenter
                }

                // Status (from MediaControl service)
                Text {
                    id: nowPlayingStatus
                    text: homeScreenManager.isPlaying ? "▶ Playing" : "⏸ Paused"
                    font.pixelSize: 14
                    color: homeScreenManager.isPlaying ? "#27ae60" : "#7f8c8d"
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }
    }
}
