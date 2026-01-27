#include "VehicleControlClient.h"
#include <QDebug>
#include <QDateTime>
#include <functional>

VehicleControlClient::VehicleControlClient(QObject *parent)
    : QObject(parent)
    , m_currentGear("P")
    , m_isConnected(false)
{
    qDebug() << "[AmbientApp] VehicleControlClient created";
}

VehicleControlClient::~VehicleControlClient()
{
    if (m_proxy) {
        m_proxy.reset();
    }
    qDebug() << "[AmbientApp] VehicleControlClient destroyed";
}

bool VehicleControlClient::initialize()
{
    qDebug() << "🔌 [AmbientApp] Connecting to VehicleControl service...";
    
    // Get CommonAPI runtime
    m_runtime = CommonAPI::Runtime::get();
    if (!m_runtime) {
        qCritical() << "❌ Failed to get CommonAPI runtime!";
        return false;
    }
    
    // Build proxy
    const std::string domain = "local";
    const std::string instance = "vehiclecontrol.VehicleControl";
    const std::string connection = "client-ambient";
    
    m_proxy = m_runtime->buildProxy<VehicleControlProxy>(domain, instance, connection);
    
    if (!m_proxy) {
        qCritical() << "❌ Failed to build VehicleControl proxy!";
        return false;
    }
    
    qDebug() << "✅ Proxy created successfully";
    
    // Subscribe to availability status
    m_proxy->getProxyStatusEvent().subscribe(
        std::bind(&VehicleControlClient::onAvailabilityChanged, this, std::placeholders::_1)
    );
    
    // Setup event subscriptions
    setupEventSubscriptions();
    
    qDebug() << "✅ Connected to VehicleControl service";
    qDebug() << "   Domain:" << QString::fromStdString(domain);
    qDebug() << "   Instance:" << QString::fromStdString(instance);
    
    return true;
}

void VehicleControlClient::setupEventSubscriptions()
{
    if (!m_proxy) {
        qWarning() << "Cannot setup subscriptions: proxy is null";
        return;
    }
    
    qDebug() << "📡 [AmbientApp] Subscribing to VehicleControl events...";
    
    // Subscribe to gearChanged event
    m_proxy->getGearChangedEvent().subscribe(
        [this](std::string newGear, std::string oldGear, uint64_t timestamp) {
            this->onGearChanged(newGear, oldGear, timestamp);
        }
    );
    
    // Subscribe to vehicleStateChanged event (for full state updates)
    m_proxy->getVehicleStateChangedEvent().subscribe(
        [this](std::string gear, uint16_t speed, uint8_t battery, uint64_t timestamp) {
            this->onVehicleStateChanged(gear, speed, battery, timestamp);
        }
    );
    
    qDebug() << "✅ Event subscriptions setup complete";
}

void VehicleControlClient::onGearChanged(std::string newGear, std::string oldGear, uint64_t timestamp)
{
    QString qNewGear = QString::fromStdString(newGear);
    QString qOldGear = QString::fromStdString(oldGear);
    
    qDebug() << "📡 [AmbientApp] gearChanged event:"
             << qOldGear << "→" << qNewGear
             << "@ timestamp:" << timestamp;
    
    if (m_currentGear != qNewGear) {
        m_currentGear = qNewGear;
        emit currentGearChanged(m_currentGear);
    }
}

void VehicleControlClient::onVehicleStateChanged(std::string gear, uint16_t speed, uint8_t battery, uint64_t timestamp)
{
    QString qGear = QString::fromStdString(gear);
    
    // Only care about gear changes for ambient lighting
    if (m_currentGear != qGear) {
        m_currentGear = qGear;
        qDebug() << "📡 [AmbientApp] vehicleStateChanged: Gear changed to" << m_currentGear;
        emit currentGearChanged(m_currentGear);
    }
}

void VehicleControlClient::onAvailabilityChanged(CommonAPI::AvailabilityStatus status)
{
    bool wasConnected = m_isConnected;
    m_isConnected = (status == CommonAPI::AvailabilityStatus::AVAILABLE);
    
    if (m_isConnected != wasConnected) {
        qDebug() << "🔗 [AmbientApp] Service availability changed:"
                 << (m_isConnected ? "AVAILABLE" : "NOT AVAILABLE");
        emit connectedChanged(m_isConnected);
    }
    
    if (m_isConnected) {
        qDebug() << "✅ VehicleControl service is now available!";
    } else {
        qWarning() << "⚠️  VehicleControl service is not available";
    }
}
