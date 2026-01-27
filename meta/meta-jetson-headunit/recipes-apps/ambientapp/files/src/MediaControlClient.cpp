#include "MediaControlClient.h"
#include <QDebug>

MediaControlClient::MediaControlClient(AmbientManager* ambientManager, QObject* parent)
    : QObject(parent)
    , m_ambientManager(ambientManager)
    , m_availabilityTimer(new QTimer(this))
    , m_isAvailable(false)
{
    qDebug() << "[AmbientApp] MediaControlClient: Initializing...";
}

MediaControlClient::~MediaControlClient() {
    qDebug() << "[AmbientApp] MediaControlClient: Destroyed";
}

bool MediaControlClient::initialize() {
    qDebug() << "[AmbientApp] MediaControlClient: Creating proxy...";
    
    // CommonAPI Runtime 가져오기
    std::shared_ptr<CommonAPI::Runtime> runtime = CommonAPI::Runtime::get();
    
    // Proxy 생성
    m_proxy = runtime->buildProxy<v1::mediacontrol::MediaControlProxy>(
        "local",                           // domain
        "mediacontrol.MediaControl"        // instance
    );
    
    if (!m_proxy) {
        qDebug() << "[AmbientApp] MediaControlClient: Failed to create proxy!";
        return false;
    }
    
    qDebug() << "[AmbientApp] MediaControlClient: Proxy created successfully";
    
    // 서비스 가용성 체크 타이머 설정
    connect(m_availabilityTimer, &QTimer::timeout, this, &MediaControlClient::checkServiceAvailability);
    m_availabilityTimer->start(1000);  // 1초마다 체크
    
    return true;
}

void MediaControlClient::checkServiceAvailability() {
    if (!m_proxy) {
        return;
    }
    
    if (m_proxy->isAvailable()) {
        if (!m_isAvailable) {
            qDebug() << "[AmbientApp] MediaControlClient: Service is now AVAILABLE!";
            m_isAvailable = true;
            m_availabilityTimer->stop();
            
            // 서비스 사용 가능하면 이벤트 구독
            subscribeToEvents();
            
            // 초기 볼륨 값 가져오기
            getVolume();
        }
    } else {
        if (m_isAvailable) {
            qDebug() << "[AmbientApp] MediaControlClient: Service became UNAVAILABLE!";
            m_isAvailable = false;
        }
    }
}

void MediaControlClient::subscribeToEvents() {
    qDebug() << "[AmbientApp] MediaControlClient: Subscribing to volumeChanged event...";
    
    // volumeChanged 이벤트 구독
    m_proxy->getVolumeChangedEvent().subscribe([this](const float& newVolume) {
        qDebug() << "[AmbientApp] MediaControlClient: Received volumeChanged event:" << newVolume;
        onVolumeChanged(newVolume);
    });
    
    qDebug() << "[AmbientApp] MediaControlClient: Successfully subscribed to events";
}

void MediaControlClient::getVolume() {
    if (!m_proxy || !m_isAvailable) {
        qDebug() << "[AmbientApp] MediaControlClient: Cannot get volume - service not available";
        return;
    }
    
    qDebug() << "[AmbientApp] MediaControlClient: Requesting current volume...";
    
    // 비동기 getVolume 호출
    m_proxy->getVolumeAsync([this](const CommonAPI::CallStatus& callStatus, const float& volume) {
        if (callStatus == CommonAPI::CallStatus::SUCCESS) {
            qDebug() << "[AmbientApp] MediaControlClient: Current volume:" << volume;
            onVolumeChanged(volume);
        } else {
            qDebug() << "[AmbientApp] MediaControlClient: Failed to get volume, status:" 
                     << static_cast<int>(callStatus);
        }
    });
}

void MediaControlClient::onVolumeChanged(float newVolume) {
    qDebug() << "[AmbientApp] MediaControlClient: Volume changed to:" << newVolume;
    updateAmbientBrightness(newVolume);
}

void MediaControlClient::updateAmbientBrightness(float volume) {
    if (!m_ambientManager) {
        qDebug() << "[AmbientApp] MediaControlClient: AmbientManager is null!";
        return;
    }

    // Volume (0.0-1.0) -> Brightness (0.5 - 1.0) linear mapping
    // IMPORTANT: MediaManager volume is already 0.0-1.0, NOT 0-100!
    // Volume 0.0 (0%)   = brightness 0.5 (50% - minimum, not too dark)
    // Volume 0.5 (50%)  = brightness 0.75 (75%)
    // Volume 1.0 (100%) = brightness 1.0 (100% - maximum)
    // Formula: brightness = 0.5 + (volume * 0.5)
    float brightness = 0.5f + (volume * 0.5f);

    qDebug() << "[AmbientApp] MediaControlClient: Setting brightness to:" << brightness
             << "(from volume:" << volume << ")";

    m_ambientManager->setBrightness(brightness);

    qDebug() << "[AmbientApp] MediaControlClient: Brightness updated successfully";
}
