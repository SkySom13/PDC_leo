#ifndef MEDIACONTROLCLIENT_H
#define MEDIACONTROLCLIENT_H

#include <QObject>
#include <QTimer>
#include <CommonAPI/CommonAPI.hpp>
#include <v1/mediacontrol/MediaControlProxy.hpp>
#include "ambientmanager.h"

/**
 * MediaControl Client (Proxy)
 * 
 * AmbientApp이 MediaApp의 volumeChanged 이벤트를 구독하고
 * brightness를 조절하는 클라이언트
 */
class MediaControlClient : public QObject {
    Q_OBJECT

public:
    explicit MediaControlClient(AmbientManager* ambientManager, QObject* parent = nullptr);
    virtual ~MediaControlClient();

    bool initialize();
    void getVolume();  // 현재 볼륨 조회

private slots:
    void onVolumeChanged(float newVolume);
    void checkServiceAvailability();

private:
    void subscribeToEvents();
    void updateAmbientBrightness(float volume);

    AmbientManager* m_ambientManager;
    std::shared_ptr<v1::mediacontrol::MediaControlProxy<>> m_proxy;
    QTimer* m_availabilityTimer;
    bool m_isAvailable;
};

#endif // MEDIACONTROLCLIENT_H
