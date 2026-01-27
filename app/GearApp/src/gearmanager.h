#ifndef GEARMANAGER_H
#define GEARMANAGER_H

#include <QObject>
#include <QString>

class GearManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString gearPosition READ gearPosition WRITE setGearPosition NOTIFY gearPositionChanged)

public:
    explicit GearManager(QObject *parent = nullptr);

    QString gearPosition() const { return m_gearPosition; }
    Q_INVOKABLE void setGearPosition(const QString &position);              // Called by QML when user clicks gear button

    // FIX: Separate method for vsomeip event updates (prevents feedback loop)
    void updateGearFromService(const QString &position);                    // Called by vsomeip event handler only

signals:
    void gearPositionChanged(const QString &gear);                          // Signal to update QML UI
    void gearChangeRequested(const QString &gear);                          // Signal to trigger vsomeip RPC call

private:
    QString m_gearPosition;
};

#endif // GEARMANAGER_H
