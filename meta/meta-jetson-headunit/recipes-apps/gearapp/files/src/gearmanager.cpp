#include "gearmanager.h"
#include <QDebug>

GearManager::GearManager(QObject *parent)
    : QObject(parent)
    , m_gearPosition("P")  // 기본값: Park
{
    qDebug() << "GearManager initialized with position:" << m_gearPosition;
}

void GearManager::setGearPosition(const QString &position)
{
    // Update internal state immediately for instant UI feedback
    if (m_gearPosition != position) {
        m_gearPosition = position;
        qDebug() << "GearManager: Setting gear position to:" << position;
        emit gearPositionChanged(position);                                 // Emit signal for UI update
    }

    // Also request gear change via vsomeip
    qDebug() << "GearManager: Requesting gear change via vsomeip:" << position;

    // ═══════════════════════════════════════════════════════════
    // vsomeip RPC 호출: VehicleControlECU에 기어 변경 요청
    // ═══════════════════════════════════════════════════════════
    emit gearChangeRequested(position);                                     // Trigger RPC call to VehicleControlMock

    // Note: If VehicleControl service is available, it will broadcast the change
    // and all clients will receive the gearChanged event for synchronization
}

// FIX: New method to update gear from vsomeip events WITHOUT triggering RPC
void GearManager::updateGearFromService(const QString &position)
{
    // This method is called when receiving vsomeip gearChanged events
    // It updates the gear state WITHOUT emitting gearChangeRequested signal
    // This prevents the feedback loop: event → update → RPC → event → ...
    if (m_gearPosition != position) {
        m_gearPosition = position;
        qDebug() << "GearManager: Gear updated from service:" << position;  // Log service update
        emit gearPositionChanged(position);                                 // Update QML UI only (no RPC)
    }
}
