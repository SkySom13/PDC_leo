#include "homescreenmanager.h"
#include <QDebug>

HomeScreenManager::HomeScreenManager(QObject *parent)
    : QObject(parent)
    , m_currentTrack("No song playing")
    , m_isPlaying(false)
    , m_ambientColor("#3498db")  // Default blue
    , m_ambientBrightness(1.0)   // Default full brightness
{
    qDebug() << "HomeScreenManager initialized";
    qDebug() << "   Track:" << m_currentTrack;
    qDebug() << "   Color:" << m_ambientColor;
    qDebug() << "   Brightness:" << m_ambientBrightness;
}

HomeScreenManager::~HomeScreenManager()
{
    qDebug() << "HomeScreenManager destroyed";
}

void HomeScreenManager::onTrackChanged(const QString& title, bool playing)
{
    bool changed = false;
    
    if (m_currentTrack != title) {
        m_currentTrack = title;
        emit currentTrackChanged();
        changed = true;
    }
    
    if (m_isPlaying != playing) {
        m_isPlaying = playing;
        emit isPlayingChanged();
        changed = true;
    }
    
    if (changed) {
        qDebug() << "[HomeScreen] Track updated:"
                 << "Title:" << m_currentTrack
                 << "Playing:" << m_isPlaying;
    }
}

void HomeScreenManager::onColorChanged(const QString& color)
{
    if (m_ambientColor != color) {
        m_ambientColor = color;
        emit ambientColorChanged();
        qDebug() << "[HomeScreen] Ambient color changed:" << m_ambientColor;
    }
}

void HomeScreenManager::onBrightnessChanged(qreal brightness)
{
    if (!qFuzzyCompare(m_ambientBrightness, brightness)) {
        m_ambientBrightness = brightness;
        emit ambientBrightnessChanged();
        qDebug() << "[HomeScreen] Ambient brightness changed:" << m_ambientBrightness;
    }
}
