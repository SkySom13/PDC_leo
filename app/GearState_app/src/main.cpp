#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtQml>
#include <QTimer>
#include <QWindow>
#include "vehiclecontrolclient.h"

int main(int argc, char *argv[])
{
    qDebug() << "GearState_app Starting...";
    QGuiApplication app(argc, argv);

    // ═══════════════════════════════════════════════════════
    // IVI SHELL: Set application name for display routing
    // ═══════════════════════════════════════════════════════
    // This name will be routed by ivi-shell
    app.setApplicationName("appGearState");  // ← IVI Shell routing
    app.setApplicationDisplayName("Gear State");
    app.setDesktopFileName("appGearState");

    qDebug() << "═══════════════════════════════════════════════════════";
    qDebug() << "GearState_app Starting (IVI Shell - Weston)";
    qDebug() << "App ID: appGearState";
    qDebug() << "Mock Mode: Using simulated vehicle data";
    qDebug() << "═══════════════════════════════════════════════════════";
    
    QQmlApplicationEngine engine;

    // Connect to QML engine warnings/errors
    QObject::connect(&engine, &QQmlApplicationEngine::warnings, [](const QList<QQmlError> &warnings) {
        for (const QQmlError &warning : warnings) {
            Q_UNUSED(warning);
        }
    });

    engine.addImportPath("qrc:/");

    qmlRegisterSingletonType(QUrl(QStringLiteral("qrc:/Design/Constants.qml")),
                             "Design", 1, 0, "Constants");

    // ═══════════════════════════════════════════════════════
    // Register C++ objects to QML context
    // ═══════════════════════════════════════════════════════
    // VehicleControlClient (vsomeip - Gear State)
    VehicleControlClient *vehicleClient = new VehicleControlClient();
    engine.rootContext()->setContextProperty("vehicleClient", vehicleClient);

    const QUrl url(QStringLiteral("qrc:/DesignContent/App.qml"));
    engine.load(url);

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
