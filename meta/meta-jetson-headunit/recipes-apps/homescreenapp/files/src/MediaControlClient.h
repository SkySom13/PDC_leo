#ifndef MEDIACONTROLCLIENT_H
#define MEDIACONTROLCLIENT_H

#include <QObject>
#include <QString>
#include <CommonAPI/CommonAPI.hpp>
#include <v1/mediacontrol/MediaControlProxy.hpp>

using namespace v1::mediacontrol;

/**
 * @brief MediaControl Service Client for HomeScreen
 * 
 * Subscribes to MediaControl service to receive:
 * - Current track title + playback state
 * - Track change notifications
 */
class MediaControlClient : public QObject
{
    Q_OBJECT

public:
    explicit MediaControlClient(QObject *parent = nullptr);
    virtual ~MediaControlClient();

    bool initialize();
    void fetchCurrentState();  // Query current music

signals:
    void currentMusicChanged(QString title, bool isPlaying);           // Emitted when track changes
    void serviceAvailable(bool available);                              // Service availability

private:
    void setupEventSubscriptions();
    void onCurrentMusicChanged(std::string title, bool isPlaying);
    void onAvailabilityChanged(CommonAPI::AvailabilityStatus status);
    
    std::shared_ptr<MediaControlProxy<>> m_proxy;
    std::shared_ptr<CommonAPI::Runtime> m_runtime;
    bool m_isConnected;
};

#endif // MEDIACONTROLCLIENT_H
