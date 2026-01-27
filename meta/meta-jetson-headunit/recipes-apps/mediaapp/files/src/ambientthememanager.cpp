#include "ambientthememanager.h"
#include <QDebug>

AmbientThemeManager::AmbientThemeManager(QObject *parent)
    : QObject(parent)
    , m_ambientColor("#27ae60")  // Default green
    , m_brightness(0.8)           // Default brightness
{
    qDebug() << "[MediaApp] AmbientThemeManager created with defaults:"
             << "color=" << m_ambientColor
             << "brightness=" << m_brightness;
}

void AmbientThemeManager::onAmbientColorChanged(QString color)
{
    if (m_ambientColor != color) {
        m_ambientColor = color;
        qDebug() << "[MediaApp] Ambient color updated to:" << color;
        emit ambientColorChanged();
    }
}

void AmbientThemeManager::onBrightnessChanged(qreal brightness)
{
    if (qAbs(m_brightness - brightness) > 0.01) {
        m_brightness = brightness;
        qDebug() << "[MediaApp] Brightness updated to:" << brightness;
        emit brightnessChanged();
    }
}
