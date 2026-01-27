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
    void setGearPosition(const QString &position);

signals:
    void gearPositionChanged(const QString &gear);
    void gearChangeRequested(const QString &gear);  // vsomeip RPC 호출용

private:
    QString m_gearPosition;
};

#endif // GEARMANAGER_H
