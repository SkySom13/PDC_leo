import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
// HeadUnit 모듈 제거: C++ backend는 contextProperty로 노출됨

// GearApp 독립 실행용 메인 윈도우
// Left panel: 130px width, full height minus nav bar
Window {
    id: window
    width: 130
    height: 1000  // 1080 - 80 (nav bar)
    visible: true
    title: "GearApp"
    color: "#1a1a1a"
    flags: Qt.Window | Qt.WindowStaysOnTopHint | Qt.FramelessWindowHint

    // 재사용 가능한 기어 선택 위젯 컴포넌트
    Column {
        id: root
        anchors.centerIn: parent

        property string currentGear: gearManager.gearPosition

        spacing: 15

        // GearManager의 gearPositionChanged 시그널을 감지하여 UI 업데이트
        Connections {
            target: gearManager
            function onGearPositionChanged(gear) {
                root.currentGear = gear
                console.log("GearApp QML: Gear position updated to:", gear)
            }
        }

        // 기어 선택 버튼들
        Repeater {
            model: ["P", "R", "N", "D"]

            Rectangle {
                width: 100
                height: 100
                color: {
                    if (root.currentGear === modelData) {
                        switch(modelData) {
                            case "P": return "#3498db"  // Blue (Park)
                            case "R": return "#e74c3c"  // Red (Reverse)
                            case "N": return "#f39c12"  // Yellow (Neutral)
                            case "D": return "#27ae60"  // Green (Drive)
                        }
                    }
                    return "#7f8c8d"  // Inactive gray
                }
                radius: 12
                border.color: {
                    if (root.currentGear === modelData) {
                        switch(modelData) {
                            case "P": return "#2980b9"  // Dark blue
                            case "R": return "#c0392b"  // Dark red
                            case "N": return "#e67e22"  // Dark yellow
                            case "D": return "#229954"  // Dark green
                        }
                    }
                    return "#95a5a6"
                }
                border.width: 3

                // Glow effect for selected gear
                Rectangle {
                    anchors.centerIn: parent
                    width: parent.width + 6
                    height: parent.height + 6
                    color: "transparent"
                    radius: parent.radius + 3
                    border.color: parent.color
                    border.width: 2
                    opacity: root.currentGear === modelData ? 0.6 : 0
                    visible: root.currentGear === modelData

                    Behavior on opacity {
                        NumberAnimation { duration: 200 }
                    }
                }

                Text {
                    anchors.centerIn: parent
                    text: modelData
                    font.pixelSize: 52
                    font.bold: true
                    color: "#ecf0f1"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        gearManager.gearPosition = modelData
                        console.log("Gear changed to:", modelData)
                    }

                    // Touch feedback
                    onPressed: {
                        parent.scale = 0.95
                        parent.opacity = 0.8
                    }
                    onReleased: {
                        parent.scale = 1.0
                        parent.opacity = 1.0
                    }
                }

                // Animation effects
                Behavior on color {
                    ColorAnimation { duration: 200 }
                }

                Behavior on scale {
                    NumberAnimation { duration: 100 }
                }

                Behavior on opacity {
                    NumberAnimation { duration: 100 }
                }
            }
        }
    } // Column (root)
} // Window
