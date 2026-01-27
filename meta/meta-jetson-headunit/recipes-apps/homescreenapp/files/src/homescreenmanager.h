#ifndef HOMESCREENMANAGER_H
#define HOMESCREENMANAGER_H

#include <QObject>
#include <QString>

/**
 * @brief HomeScreenManager - Backend for HomeScreen dashboard
 * 
 * Aggregates data from multiple vSOME/IP services:
 * - MediaControl: Current track + playback state
 * - AmbientControl: Current color + brightness
 */
class HomeScreenManager : public QObject
{
    Q_OBJECT
    
    // Properties exposed to QML
    Q_PROPERTY(QString currentTrack READ currentTrack NOTIFY currentTrackChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY isPlayingChanged)
    Q_PROPERTY(QString ambientColor READ ambientColor NOTIFY ambientColorChanged)
    Q_PROPERTY(qreal ambientBrightness READ ambientBrightness NOTIFY ambientBrightnessChanged)

public:
    explicit HomeScreenManager(QObject *parent = nullptr);
    virtual ~HomeScreenManager();
    
    // Property getters
    QString currentTrack() const { return m_currentTrack; }
    bool isPlaying() const { return m_isPlaying; }
    QString ambientColor() const { return m_ambientColor; }
    qreal ambientBrightness() const { return m_ambientBrightness; }

signals:
    void currentTrackChanged();
    void isPlayingChanged();
    void ambientColorChanged();
    void ambientBrightnessChanged();

public slots:
    // Slots to receive updates from service clients
    void onTrackChanged(const QString& title, bool playing);
    void onColorChanged(const QString& color);
    void onBrightnessChanged(qreal brightness);

private:
    QString m_currentTrack;
    bool m_isPlaying;
    QString m_ambientColor;
    qreal m_ambientBrightness;
};

#endif // HOMESCREENMANAGER_H
