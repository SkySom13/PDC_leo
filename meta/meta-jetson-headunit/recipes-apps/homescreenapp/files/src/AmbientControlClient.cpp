#include "AmbientControlClient.h"
#include <QDebug>
#include <QTimer>

AmbientControlClient::AmbientControlClient(QObject *parent)
    : QObject(parent)
    , m_isConnected(false)
{
    qDebug() << "[HomeScreen] AmbientControlClient created";
}

AmbientControlClient::~AmbientControlClient()
{
    if (m_proxy) {
        m_proxy.reset();
    }
    qDebug() << "[HomeScreen] AmbientControlClient destroyed";
}

bool AmbientControlClient::initialize()
{
    qDebug() << "🔌 [HomeScreen] Connecting to AmbientControl service...";
    
    // Get CommonAPI runtime
    m_runtime = CommonAPI::Runtime::get();
    if (!m_runtime) {
        qCritical() << "❌ Failed to get CommonAPI runtime!";
        return false;
    }
    
    // Build proxy
    const std::string domain = "local";
    const std::string instance = "ambientcontrol.AmbientControl";
    const std::string connection = "client-homescreen-ambient";
    
    m_proxy = m_runtime->buildProxy<AmbientControlProxy>(domain, instance, connection);
    
    if (!m_proxy) {
        qCritical() << "❌ Failed to build AmbientControl proxy!";
        return false;
    }
    
    qDebug() << "✅ AmbientControl proxy created";
    
    // Subscribe to availability status
    m_proxy->getProxyStatusEvent().subscribe(
        std::bind(&AmbientControlClient::onAvailabilityChanged, this, std::placeholders::_1)
    );
    
    // Setup event subscriptions
    setupEventSubscriptions();
    
    qDebug() << "✅ Connected to AmbientControl service";
    return true;
}

void AmbientControlClient::setupEventSubscriptions()
{
    if (!m_proxy) {
        qWarning() << "Cannot setup subscriptions: proxy is null";
        return;
    }
    
    qDebug() << "📡 [HomeScreen] Subscribing to AmbientControl events...";
    
    // Subscribe to ambientColorChanged event
    m_proxy->getAmbientColorChangedEvent().subscribe(
        [this](std::string newColor) {
            this->onAmbientColorChanged(newColor);
        }
    );
    
    // Subscribe to brightnessChanged event
    m_proxy->getBrightnessChangedEvent().subscribe(
        [this](float newBrightness) {
            this->onBrightnessChanged(newBrightness);
        }
    );
    
    qDebug() << "✅ AmbientControl event subscriptions complete";
}

void AmbientControlClient::onAmbientColorChanged(std::string newColor)
{
    QString qColor = QString::fromStdString(newColor);
    
    qDebug() << "📡 [HomeScreen] ambientColorChanged:" << qColor;
    
    emit ambientColorChanged(qColor);
}

void AmbientControlClient::onBrightnessChanged(float newBrightness)
{
    qDebug() << "📡 [HomeScreen] brightnessChanged:" << newBrightness;
    
    emit brightnessChanged(newBrightness);
}

void AmbientControlClient::fetchCurrentState()
{
    if (!m_proxy || !m_isConnected) {
        qWarning() << "[HomeScreen] Cannot fetch state: service not available";
        return;
    }

    qDebug() << "🔍 [HomeScreen] Fetching current ambient state...";

    // Fetch current color
    CommonAPI::CallStatus colorCallStatus;
    std::string currentColor;
    m_proxy->getAmbientColor(colorCallStatus, currentColor);

    if (colorCallStatus == CommonAPI::CallStatus::SUCCESS) {
        QString qColor = QString::fromStdString(currentColor);
        qDebug() << "   Current color:" << qColor;
        emit ambientColorChanged(qColor);
    } else {
        qWarning() << "   Failed to get ambient color";
    }

    // Fetch current brightness
    CommonAPI::CallStatus brightnessCallStatus;
    float currentBrightness;
    m_proxy->getBrightness(brightnessCallStatus, currentBrightness);

    if (brightnessCallStatus == CommonAPI::CallStatus::SUCCESS) {
        qDebug() << "   Current brightness:" << currentBrightness;
        emit brightnessChanged(currentBrightness);
    } else {
        qWarning() << "   Failed to get brightness";
    }
}

void AmbientControlClient::onAvailabilityChanged(CommonAPI::AvailabilityStatus status)
{
    bool wasConnected = m_isConnected;
    m_isConnected = (status == CommonAPI::AvailabilityStatus::AVAILABLE);

    if (m_isConnected != wasConnected) {
        qDebug() << "🔗 [HomeScreen] AmbientControl service:"
                 << (m_isConnected ? "AVAILABLE" : "NOT AVAILABLE");
        emit serviceAvailable(m_isConnected);

        // When service becomes available, fetch initial state
        if (m_isConnected) {
            QTimer::singleShot(500, this, &AmbientControlClient::fetchCurrentState);
        }
    }
}
