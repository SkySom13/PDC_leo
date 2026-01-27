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
        emit gearPositionChanged(position);  // Emit signal for UI update
    }

    // Also request gear change via vsomeip
    qDebug() << "GearManager: Requesting gear change via vsomeip:" << position;

    // ═══════════════════════════════════════════════════════════
    // vsomeip RPC 호출: VehicleControlECU에 기어 변경 요청
    // ═══════════════════════════════════════════════════════════
    emit gearChangeRequested(position);

    // Note: If VehicleControl service is available, it will broadcast the change
    // and all clients will receive the gearChanged event for synchronization
}
