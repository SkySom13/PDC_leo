#include "ambientmanager.h"
#include <QDebug>

AmbientManager::AmbientManager(QObject *parent)
    : QObject(parent)
    , m_ambientLightEnabled(true)
    , m_ambientColor("")  // Will be set by QML color wheel
    , m_brightness(1.0)   // 기본 밝기 100%
{
    qDebug() << "AmbientManager initialized - Enabled:" << m_ambientLightEnabled 
             << "Brightness:" << m_brightness
             << "Color: (waiting for QML or gear change)";
}

void AmbientManager::setAmbientLightEnabled(bool enabled)
{
    if (m_ambientLightEnabled == enabled)
        return;
    
    qDebug() << "Ambient light enabled changed:" << m_ambientLightEnabled << "->" << enabled;
    m_ambientLightEnabled = enabled;
    emit ambientLightEnabledChanged();
}

void AmbientManager::setAmbientColor(const QString &color)
{
    if (m_ambientColor == color)
        return;
    
    qDebug() << "AmbientManager: Color changed:" << m_ambientColor << "->" << color;
    m_ambientColor = color;
    emit ambientColorChanged();
}

void AmbientManager::setBrightness(qreal brightness)
{
    // ═══════════════════════════════════════════════════════════
    // HU 내부 통신: Volume → Brightness
    // ═══════════════════════════════════════════════════════════
    // MediaManager의 볼륨 변경에 따라 앰비언트 밝기 조절
    // 범위: 0.0 (어두움) ~ 1.0 (밝음)
    // ═══════════════════════════════════════════════════════════
    
    // Clamp to valid range
    brightness = qBound(0.0, brightness, 1.0);
    
    if (qFuzzyCompare(m_brightness, brightness))
        return;
    
    qDebug() << "AmbientManager: Brightness changed:" << m_brightness << "->" << brightness;
    m_brightness = brightness;
    emit brightnessChanged();
}

void AmbientManager::onGearPositionChanged(const QString &gear)
{
    // ═══════════════════════════════════════════════════════════
    // HU 내부 통신: Gear → Color
    // ═══════════════════════════════════════════════════════════
    // GearManager로부터 기어 상태를 받아 색상 자동 변경
    //
    // 기어별 색상 매핑:
    //   P (Park)    : 파란색 #3498db (정차)
    //   R (Reverse) : 빨간색 #e74c3c (후진 주의)
    //   N (Neutral) : 노란색 #f39c12 (중립 대기)
    //   D (Drive)   : 녹색   #27ae60 (주행)
    // ═══════════════════════════════════════════════════════════
    
    QString newColor = getColorForGear(gear);
    
    if (!newColor.isEmpty() && newColor != m_ambientColor) {
        qDebug() << "AmbientManager: [Gear → Color] Gear" << gear << "→ Color" << newColor;
        setAmbientColor(newColor);
    }
}

void AmbientManager::onVolumeChanged(int volume)
{
    // ═══════════════════════════════════════════════════════════
    // HU 내부 통신: Volume → Brightness
    // ═══════════════════════════════════════════════════════════
    // MediaManager로부터 볼륨 값을 받아 밝기 자동 조절
    // 
    // 볼륨-밝기 매핑:
    //   볼륨 0   → 밝기 30%  (0.3)
    //   볼륨 50  → 밝기 65%  (0.65)
    //   볼륨 100 → 밝기 100% (1.0)
    //
    // 공식: brightness = 0.3 + (volume / 100.0) * 0.7
    // ═══════════════════════════════════════════════════════════
    
    qreal newBrightness = calculateBrightnessFromVolume(volume);
    qDebug() << "AmbientManager: [Volume → Brightness] Volume" << volume << "→ Brightness" << newBrightness;
    setBrightness(newBrightness);
}

QString AmbientManager::getColorForGear(const QString &gear) const
{
    // 기어별 색상 매핑 테이블
    if (gear == "P") return "#3498db";      // 파란색 (Park)
    if (gear == "R") return "#e74c3c";      // 빨간색 (Reverse)
    if (gear == "N") return "#f39c12";      // 노란색 (Neutral)
    if (gear == "D") return "#27ae60";      // 녹색 (Drive)

    qDebug() << "AmbientManager: Unknown gear position:" << gear;
    return "";  // 알 수 없는 기어는 색상 변경 안 함
}

qreal AmbientManager::calculateBrightnessFromVolume(int volume) const
{
    // 볼륨을 0~100 범위로 제한
    volume = qBound(0, volume, 100);
    
    // 볼륨 0 → 0.3, 볼륨 100 → 1.0
    // 선형 매핑: y = 0.3 + (x / 100) * 0.7
    return 0.3 + (volume / 100.0) * 0.7;
}
