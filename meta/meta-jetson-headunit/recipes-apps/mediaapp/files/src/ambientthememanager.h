#ifndef AMBIENTTHEMEMANAGER_H
#define AMBIENTTHEMEMANAGER_H

#include <QObject>
#include <QString>

/**
 * @brief Simple manager to expose ambient theme to QML
 *
 * Receives ambient color/brightness from AmbientControlClient
 * and exposes them as Q_PROPERTY for QML binding
 */
class AmbientThemeManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString ambientColor READ ambientColor NOTIFY ambientColorChanged)
    Q_PROPERTY(qreal brightness READ brightness NOTIFY brightnessChanged)

public:
    explicit AmbientThemeManager(QObject *parent = nullptr);

    QString ambientColor() const { return m_ambientColor; }
    qreal brightness() const { return m_brightness; }

public slots:
    void onAmbientColorChanged(QString color);
    void onBrightnessChanged(qreal brightness);

signals:
    void ambientColorChanged();
    void brightnessChanged();

private:
    QString m_ambientColor;
    qreal m_brightness;
};

#endif // AMBIENTTHEMEMANAGER_H
