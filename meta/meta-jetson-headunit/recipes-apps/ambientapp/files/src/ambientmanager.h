#ifndef AMBIENTMANAGER_H
#define AMBIENTMANAGER_H

#include <QObject>
#include <QString>
#include <QColor>

class AmbientManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool ambientLightEnabled READ ambientLightEnabled WRITE setAmbientLightEnabled NOTIFY ambientLightEnabledChanged)
    Q_PROPERTY(QString ambientColor READ ambientColor WRITE setAmbientColor NOTIFY ambientColorChanged)
    Q_PROPERTY(qreal brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged)

public:
    explicit AmbientManager(QObject *parent = nullptr);
    
    bool ambientLightEnabled() const { return m_ambientLightEnabled; }
    QString ambientColor() const { return m_ambientColor; }
    qreal brightness() const { return m_brightness; }
    
    void setAmbientLightEnabled(bool enabled);
    void setAmbientColor(const QString &color);
    void setBrightness(qreal brightness);

public slots:
    // GearManager 연결: 기어 상태에 따라 색상 변경
    void onGearPositionChanged(const QString &gear);
    
    // MediaManager 연결: 볼륨에 따라 밝기 조절
    void onVolumeChanged(int volume);

signals:
    void ambientLightEnabledChanged();
    void ambientColorChanged();
    void brightnessChanged();

private:
    QString getColorForGear(const QString &gear) const;
    qreal calculateBrightnessFromVolume(int volume) const;
    
    bool m_ambientLightEnabled;
    QString m_ambientColor;
    qreal m_brightness;  // 0.0 ~ 1.0
};

#endif // AMBIENTMANAGER_H
