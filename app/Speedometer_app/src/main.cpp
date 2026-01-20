#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtQml>
#include <QTimer>
#include <QWindow>
#include "vehiclecontrolclient.h"

int main(int argc, char *argv[])
{
    qDebug() << "Speedometer_app Starting...";
    QGuiApplication app(argc, argv);

    // ═══════════════════════════════════════════════════════
    // IVI SHELL: Set application name for display routing
    // ═══════════════════════════════════════════════════════
    // This name will be routed by ivi-shell
    app.setApplicationName("appSpeedometer");  // ← IVI Shell routing
    app.setApplicationDisplayName("Speedometer");
    app.setDesktopFileName("appSpeedometer");

    qDebug() << "═══════════════════════════════════════════════════════";
    qDebug() << "Speedometer_app Starting (IVI Shell - Weston)";
    qDebug() << "App ID: appSpeedometer";
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
    // VehicleControlClient (vsomeip - Speed)
    VehicleControlClient *vehicleClient = new VehicleControlClient();
    engine.rootContext()->setContextProperty("vehicleClient", vehicleClient);

    const QUrl url(QStringLiteral("qrc:/DesignContent/App.qml"));
    engine.load(url);

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
