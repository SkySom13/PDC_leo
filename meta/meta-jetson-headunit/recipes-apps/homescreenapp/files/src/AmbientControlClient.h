#ifndef AMBIENTCONTROLCLIENT_H
#define AMBIENTCONTROLCLIENT_H

#include <QObject>
#include <QString>
#include <CommonAPI/CommonAPI.hpp>
#include <v1/ambientcontrol/AmbientControlProxy.hpp>

using namespace v1::ambientcontrol;

/**
 * @brief AmbientControl Service Client for HomeScreen
 * 
 * Subscribes to AmbientControl service to receive:
 * - Current ambient color
 * - Current brightness
 * - Color/brightness change notifications
 */
class AmbientControlClient : public QObject
{
    Q_OBJECT

public:
    explicit AmbientControlClient(QObject *parent = nullptr);
    virtual ~AmbientControlClient();

    bool initialize();
    void fetchCurrentState();  // Query current color and brightness

signals:
    void ambientColorChanged(QString color);                                // Emitted when color changes
    void brightnessChanged(float brightness);                               // Emitted when brightness changes
    void serviceAvailable(bool available);                                  // Service availability

private:
    void setupEventSubscriptions();
    void onAmbientColorChanged(std::string newColor);
    void onBrightnessChanged(float newBrightness);
    void onAvailabilityChanged(CommonAPI::AvailabilityStatus status);
    
    std::shared_ptr<AmbientControlProxy<>> m_proxy;
    std::shared_ptr<CommonAPI::Runtime> m_runtime;
    bool m_isConnected;
};

#endif // AMBIENTCONTROLCLIENT_H
