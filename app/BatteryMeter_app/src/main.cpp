#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QDebug>
#include <QQmlError>

#include "vehiclecontrolclient.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    // Set application name for Wayland IVI-Shell routing
    app.setApplicationName("appBatteryMeter");
    app.setApplicationDisplayName("Battery Meter");
    app.setDesktopFileName("appBatteryMeter");
    
    qDebug() << "═══════════════════════════════════════════════════════";
    qDebug() << "BatteryMeter_app Starting";
    qDebug() << "App ID: appBatteryMeter";
    qDebug() << "Window Size: 280x400 (Battery Section)";
    qDebug() << "Mode: vSomeIP/CommonAPI (Real VehicleControl service)";
    qDebug() << "═══════════════════════════════════════════════════════";
    
    QQmlApplicationEngine engine;
    
    // Connect to QML engine warnings/errors for debugging
    QObject::connect(&engine, &QQmlApplicationEngine::warnings, 
                     [](const QList<QQmlError> &warnings) {
        for (const QQmlError &warning : warnings) {
            qWarning() << "QML Warning:" << warning.toString();
        }
    });
    
    // Add import paths
    engine.addImportPath("qrc:/");
    
    qDebug() << "QML Import paths:" << engine.importPathList();
    
    // Register VehicleControlClient (vSomeIP/CommonAPI mode)
    VehicleControlClient *vehicleClient = new VehicleControlClient();
    engine.rootContext()->setContextProperty("vehicleClient", vehicleClient);

    qDebug() << "✅ VehicleControlClient registered (vSomeIP/CommonAPI mode)";
    
    // Load QML from resources
    const QUrl url(QStringLiteral("qrc:/DesignContent/App.qml"));
    qDebug() << "Loading QML from:" << url.toString();
    
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            qCritical() << "❌ Failed to create QML object!";
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);
    
    engine.load(url);
    
    if (engine.rootObjects().isEmpty()) {
        qCritical() << "❌ Failed to load QML!";
        qCritical() << "   Check that App.qml exists in resources";
        return -1;
    }
    
    qDebug() << "✅ QML UI loaded";
    qDebug() << "🚀 BatteryMeter_app is now running!";
    qDebug() << "";
    
    return app.exec();
}
