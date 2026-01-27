#ifndef VEHICLECONTROLCLIENT_H
#define VEHICLECONTROLCLIENT_H

#include <QObject>
#include <QString>
#include <CommonAPI/CommonAPI.hpp>
#include <v1/vehiclecontrol/VehicleControlProxy.hpp>

using namespace v1::vehiclecontrol;

/**
 * @brief Client for VehicleControl service (vsomeip communication)
 * 
 * This class connects to the VehicleControl service running on ECU1
 * and subscribes to gear change events for ambient light color control
 */
class VehicleControlClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentGear READ currentGear NOTIFY currentGearChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)

public:
    explicit VehicleControlClient(QObject *parent = nullptr);
    virtual ~VehicleControlClient();

    // Property getters
    QString currentGear() const { return m_currentGear; }
    bool connected() const { return m_isConnected; }

    // Initialize and connect to service
    bool initialize();

signals:
    void currentGearChanged(const QString& gear);
    void connectedChanged(bool connected);

private:
    // CommonAPI proxy
    std::shared_ptr<VehicleControlProxy<>> m_proxy;
    std::shared_ptr<CommonAPI::Runtime> m_runtime;
    
    // Current state
    QString m_currentGear;
    bool m_isConnected;
    
    // Event subscriptions
    void setupEventSubscriptions();
    void onGearChanged(std::string newGear, std::string oldGear, uint64_t timestamp);
    void onVehicleStateChanged(std::string gear, uint16_t speed, uint8_t battery, uint64_t timestamp);
    void onAvailabilityChanged(CommonAPI::AvailabilityStatus status);
};

#endif // VEHICLECONTROLCLIENT_H
