import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0

/*
 * Compositor Layout - UI Structure Only
 * Defines the visual layout and containers for client apps
 */

Item {
    id: root

    // Expose containers for surface routing
    property alias gearAppContainer: gearAppContainer
    property alias homeScreenAppContainer: homeScreenAppContainer
    property alias mediaAppContainer: mediaAppContainer
    property alias ambientAppContainer: ambientAppContainer
    property alias surfaceCount: surfaceCountText.text

    // Current page control
    property int currentPage: 0

    // ═══════════════════════════════════════════════════════
    // Left Side Panel - Permanent GearApp Display
    // ═══════════════════════════════════════════════════════
    Rectangle {
        id: leftGearPanel
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: navigationBar.top
        width: 130
        color: "#1a1a1a"

        // Container for GearApp window
        Item {
            id: gearAppContainer
            objectName: "gearAppContainer"
            anchors.fill: parent
        }
    }

    // ═══════════════════════════════════════════════════════
    // Main Content Area - Switchable Pages
    // ═══════════════════════════════════════════════════════
    Item {
        id: mainContentArea
        objectName: "mainContentArea"
        anchors.left: leftGearPanel.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: navigationBar.top
        anchors.margins: 10

        // ───────────────────────────────────────────────────
        // Page 0: HOME (HomeScreenApp window container)
        // ───────────────────────────────────────────────────
        Rectangle {
            id: homePage
            anchors.fill: parent
            visible: root.currentPage === 0
            color: "transparent"

            // Container for HomeScreenApp window
            Item {
                id: homeScreenAppContainer
                objectName: "homeScreenAppContainer"
                anchors.fill: parent
            }
        }

        // ───────────────────────────────────────────────────
        // Page 1: MEDIA (MediaApp window container)
        // ───────────────────────────────────────────────────
        Rectangle {
            id: mediaPage
            anchors.fill: parent
            visible: root.currentPage === 1
            color: "transparent"

            // Container for MediaApp window
            Item {
                id: mediaAppContainer
                objectName: "mediaAppContainer"
                anchors.fill: parent
            }
        }

        // ───────────────────────────────────────────────────
        // Page 2: AMBIENT (AmbientApp window container)
        // ───────────────────────────────────────────────────
        Rectangle {
            id: ambientPage
            anchors.fill: parent
            visible: root.currentPage === 2
            color: "transparent"

            // Container for AmbientApp window
            Item {
                id: ambientAppContainer
                objectName: "ambientAppContainer"
                anchors.fill: parent
            }
        }
    }

    // ═══════════════════════════════════════════════════════
    // Bottom Navigation Bar
    // ═══════════════════════════════════════════════════════
    Rectangle {
        id: navigationBar
        anchors.bottom: parent.bottom
        anchors.left: leftGearPanel.right
        anchors.right: parent.right
        height: 80
        color: "#2d2d2d"

        Row {
            anchors.centerIn: parent
            spacing: 20

            // Home Button (car icon)
            TabButton {
                width: 80
                height: 60
                checked: root.currentPage === 0
                onClicked: root.currentPage = 0

                background: Rectangle {
                    color: parent.checked ? "#27ae60" : "#444444"
                    radius: 8

                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                }

                contentItem: Image {
                    anchors.centerIn: parent
                    width: 50
                    height: 50
                    source: "qrc:/asset/car_white.svg"
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                }
            }

            // Media Button (mp3 icon)
            TabButton {
                width: 80
                height: 60
                checked: root.currentPage === 1
                onClicked: root.currentPage = 1

                background: Rectangle {
                    color: parent.checked ? "#2196F3" : "#444444"
                    radius: 8

                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                }

                contentItem: Image {
                    anchors.centerIn: parent
                    width: 50
                    height: 50
                    source: "qrc:/asset/mp3_white.svg"
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                }
            }

            // Ambient Button (ambient_light icon)
            TabButton {
                width: 80
                height: 60
                checked: root.currentPage === 2
                onClicked: root.currentPage = 2

                background: Rectangle {
                    color: parent.checked ? "#FF9800" : "#444444"
                    radius: 8

                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                }

                contentItem: Image {
                    anchors.centerIn: parent
                    width: 50
                    height: 50
                    source: "qrc:/asset/ambient_light.svg"
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                }
            }
        }
    }

    // ═══════════════════════════════════════════════════════
    // Status Indicator (Bottom-left corner)
    // ═══════════════════════════════════════════════════════
    Rectangle {
        anchors.left: parent.left
        anchors.bottom: navigationBar.top
        anchors.bottomMargin: 5
        width: leftGearPanel.width
        height: 30
        color: "#2d2d2d"
        radius: 5

        Text {
            id: surfaceCountText
            anchors.centerIn: parent
            text: "0 apps"
            color: "#888888"
            font.pixelSize: 12
            font.bold: true
        }
    }
}
