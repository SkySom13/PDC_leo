#include "MediaControlClient.h"
#include <QDebug>
#include <QTimer>

MediaControlClient::MediaControlClient(QObject *parent)
    : QObject(parent)
    , m_isConnected(false)
{
    qDebug() << "[HomeScreen] MediaControlClient created";
}

MediaControlClient::~MediaControlClient()
{
    if (m_proxy) {
        m_proxy.reset();
    }
    qDebug() << "[HomeScreen] MediaControlClient destroyed";
}

bool MediaControlClient::initialize()
{
    qDebug() << "🔌 [HomeScreen] Connecting to MediaControl service...";
    
    // Get CommonAPI runtime
    m_runtime = CommonAPI::Runtime::get();
    if (!m_runtime) {
        qCritical() << "❌ Failed to get CommonAPI runtime!";
        return false;
    }
    
    // Build proxy
    const std::string domain = "local";
    const std::string instance = "mediacontrol.MediaControl";
    const std::string connection = "client-homescreen-media";
    
    m_proxy = m_runtime->buildProxy<MediaControlProxy>(domain, instance, connection);
    
    if (!m_proxy) {
        qCritical() << "❌ Failed to build MediaControl proxy!";
        return false;
    }
    
    qDebug() << "✅ MediaControl proxy created";
    
    // Subscribe to availability status
    m_proxy->getProxyStatusEvent().subscribe(
        std::bind(&MediaControlClient::onAvailabilityChanged, this, std::placeholders::_1)
    );
    
    // Setup event subscriptions
    setupEventSubscriptions();
    
    qDebug() << "✅ Connected to MediaControl service";
    return true;
}

void MediaControlClient::setupEventSubscriptions()
{
    if (!m_proxy) {
        qWarning() << "Cannot setup subscriptions: proxy is null";
        return;
    }
    
    qDebug() << "📡 [HomeScreen] Subscribing to MediaControl events...";
    
    // Subscribe to currentMusicChanged event
    m_proxy->getCurrentMusicChangedEvent().subscribe(
        [this](std::string title, bool isPlaying) {
            this->onCurrentMusicChanged(title, isPlaying);
        }
    );
    
    qDebug() << "✅ MediaControl event subscriptions complete";
}

void MediaControlClient::onCurrentMusicChanged(std::string title, bool isPlaying)
{
    QString qTitle = QString::fromStdString(title);
    
    qDebug() << "📡 [HomeScreen] currentMusicChanged:"
             << "Title:" << qTitle
             << "Playing:" << isPlaying;
    
    emit currentMusicChanged(qTitle, isPlaying);
}

void MediaControlClient::fetchCurrentState()
{
    if (!m_proxy || !m_isConnected) {
        qWarning() << "[HomeScreen] Cannot fetch state: service not available";
        return;
    }

    qDebug() << "🔍 [HomeScreen] Fetching current music state...";

    // Fetch current music
    CommonAPI::CallStatus callStatus;
    std::string title;
    bool isPlaying;
    m_proxy->getCurrentMusic(callStatus, title, isPlaying);

    if (callStatus == CommonAPI::CallStatus::SUCCESS) {
        QString qTitle = QString::fromStdString(title);
        qDebug() << "   Current music:" << qTitle << "Playing:" << isPlaying;
        emit currentMusicChanged(qTitle, isPlaying);
    } else {
        qWarning() << "   Failed to get current music";
    }
}

void MediaControlClient::onAvailabilityChanged(CommonAPI::AvailabilityStatus status)
{
    bool wasConnected = m_isConnected;
    m_isConnected = (status == CommonAPI::AvailabilityStatus::AVAILABLE);

    if (m_isConnected != wasConnected) {
        qDebug() << "🔗 [HomeScreen] MediaControl service:"
                 << (m_isConnected ? "AVAILABLE" : "NOT AVAILABLE");
        emit serviceAvailable(m_isConnected);

        // When service becomes available, fetch initial state
        if (m_isConnected) {
            QTimer::singleShot(500, this, &MediaControlClient::fetchCurrentState);
        }
    }
}
