#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QTimer>
#include <QQuickWindow>
#include <CommonAPI/CommonAPI.hpp>
#include "homescreenmanager.h"
#include "MediaControlClient.h"
#include "AmbientControlClient.h"

int main(int argc, char *argv[])
{
    // ═══════════════════════════════════════════════════════════
    // Environment variables - use deployment paths if not already set
    // ═══════════════════════════════════════════════════════════
    if (qgetenv("VSOMEIP_APPLICATION_NAME").isEmpty()) {
        qputenv("VSOMEIP_APPLICATION_NAME", "HomeScreenApp");
    }
    if (qgetenv("VSOMEIP_CONFIGURATION").isEmpty()) {
        qputenv("VSOMEIP_CONFIGURATION", "/etc/vsomeip/vsomeip_homescreen.json");
    }
    if (qgetenv("COMMONAPI_CONFIG").isEmpty()) {
        qputenv("COMMONAPI_CONFIG", "/etc/commonapi/commonapi.ini");
    }

    // Wayland settings - only set if not already configured
    if (qgetenv("XDG_RUNTIME_DIR").isEmpty()) {
        qputenv("XDG_RUNTIME_DIR", "/run/user/0");
    }
    if (qgetenv("QT_QPA_PLATFORM").isEmpty()) {
        qputenv("QT_QPA_PLATFORM", "wayland");
    }
    if (qgetenv("QT_WAYLAND_DISABLE_WINDOWDECORATION").isEmpty()) {
        qputenv("QT_WAYLAND_DISABLE_WINDOWDECORATION", "1");
    }
    if (qgetenv("WAYLAND_DISPLAY").isEmpty()) {
        qputenv("WAYLAND_DISPLAY", "wayland-0");  // Changed: Direct to Weston (IVI-Shell)
    }

    QGuiApplication app(argc, argv);
    app.setApplicationName("HomeScreenApp");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("SEA-ME");
    app.setDesktopFileName("HomeScreenApp.desktop");  // Wayland App ID

    qDebug() << "═══════════════════════════════════════════════════════";
    qDebug() << "HomeScreenApp (Multi-Service Client) Starting...";
    qDebug() << "Dashboard: Aggregates data from all services";
    qDebug() << "═══════════════════════════════════════════════════════";
    qDebug() << "📋 Environment Configuration:";
    qDebug() << "   VSOMEIP_CONFIGURATION:" << qgetenv("VSOMEIP_CONFIGURATION");
    qDebug() << "   COMMONAPI_CONFIG:" << qgetenv("COMMONAPI_CONFIG");
    qDebug() << "   QT_QPA_PLATFORM:" << qgetenv("QT_QPA_PLATFORM");
    qDebug() << "   WAYLAND_DISPLAY:" << qgetenv("WAYLAND_DISPLAY");
    qDebug() << "═══════════════════════════════════════════════════════";

    // ═══════════════════════════════════════════════════════════
    // HomeScreenManager backend
    // ═══════════════════════════════════════════════════════════
    HomeScreenManager homeScreenManager;

    // ═══════════════════════════════════════════════════════════
    // vSOMEIP Service Clients
    // ═══════════════════════════════════════════════════════════
    qDebug() << "";
    qDebug() << "🔧 Initializing vSOMEIP Clients...";

    // MediaControl Client (track info)
    MediaControlClient* mediaClient = new MediaControlClient(&app);
    if (!mediaClient->initialize()) {
        qCritical() << "❌ Failed to initialize MediaControl client!";
        return -1;
    }
    qDebug() << "✅ MediaControl client initialized";

    // AmbientControl Client (color + brightness)
    AmbientControlClient* ambientClient = new AmbientControlClient(&app);
    if (!ambientClient->initialize()) {
        qCritical() << "❌ Failed to initialize AmbientControl client!";
        return -1;
    }
    qDebug() << "✅ AmbientControl client initialized";

    // ═══════════════════════════════════════════════════════════
    // Connect service clients to HomeScreenManager
    // ═══════════════════════════════════════════════════════════
    QObject::connect(mediaClient, &MediaControlClient::currentMusicChanged,
                     &homeScreenManager, &HomeScreenManager::onTrackChanged);

    QObject::connect(ambientClient, &AmbientControlClient::ambientColorChanged,
                     &homeScreenManager, &HomeScreenManager::onColorChanged);

    QObject::connect(ambientClient, &AmbientControlClient::brightnessChanged,
                     &homeScreenManager, &HomeScreenManager::onBrightnessChanged);

    qDebug() << "✅ Service clients connected to HomeScreenManager";

    // Manually fetch initial state after a delay to ensure sync
    // (In case service is already available before we subscribe)
    QTimer::singleShot(1000, [mediaClient, ambientClient]() {
        qDebug() << "[HomeScreen] Manually fetching initial states...";
        mediaClient->fetchCurrentState();
        ambientClient->fetchCurrentState();
    });

    qDebug() << "";
    qDebug() << "📡 Subscribed Services:";
    qDebug() << "   - MediaControl (track info)";
    qDebug() << "   - AmbientControl (color + brightness)";
    qDebug() << "";
    qDebug() << "✅ HomeScreenManager initialized";
    qDebug() << "═══════════════════════════════════════════════════════";

    // ═══════════════════════════════════════════════════════════
    // QML GUI
    // ═══════════════════════════════════════════════════════════
    QQmlApplicationEngine engine;

    // Expose C++ backend to QML
    engine.rootContext()->setContextProperty("homeScreenManager", &homeScreenManager);

    // Load QML file
    const QUrl url(QStringLiteral("qrc:/qml/HomeScreen.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            qCritical() << "❌ Failed to load QML file:" << url;
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);
    engine.load(url);

    if (!engine.rootObjects().isEmpty()) {
        qDebug() << "✅ QML GUI loaded: HomeScreen.qml";
        qDebug() << "🖥️  Dashboard should appear now!";
        
        // Desktop-Shell: Set window position and size for main content area
        QObject *rootObject = engine.rootObjects().first();
        if (rootObject) {
            QQuickWindow *window = qobject_cast<QQuickWindow*>(rootObject);
            if (window) {
                window->setGeometry(130, 0, 1790, 1000);  // Main area: right of GearApp
                window->setProperty("_q_waylandAppId", "HomeScreenApp");
                qDebug() << "📐 Window geometry set: (130, 0, 1790, 1000) - Main Content Area";
                qDebug() << "✅ Wayland App ID set: HomeScreenApp";
            }
        }
    }

    qDebug() << "";

    return app.exec();
}
