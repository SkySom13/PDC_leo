#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QTimer>
#include <QWindow>
#include <QQuickWindow>
#include <CommonAPI/CommonAPI.hpp>
#include "mediamanager.h"
#include "MediaControlStubImpl.h"
#include "AmbientControlClient.h"
#include "ambientthememanager.h"

int main(int argc, char *argv[])
{
    // ═══════════════════════════════════════════════════════════
    // Environment variables - use deployment paths if not already set
    // ═══════════════════════════════════════════════════════════
    if (qgetenv("VSOMEIP_APPLICATION_NAME").isEmpty()) {
        qputenv("VSOMEIP_APPLICATION_NAME", "MediaApp");
    }
    if (qgetenv("VSOMEIP_CONFIGURATION").isEmpty()) {
        qputenv("VSOMEIP_CONFIGURATION", "/etc/vsomeip/vsomeip_media.json");
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
    app.setApplicationName("MediaApp");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("SEA-ME");
    
    // Wayland App ID 설정 (Compositor가 앱을 식별하는데 사용)
    app.setDesktopFileName("MediaApp.desktop");

    qDebug() << "═══════════════════════════════════════════════════════";
    qDebug() << "MediaApp (vsomeip Service) Starting...";
    qDebug() << "Service: MediaControl (USB Media Playback + Volume Events)";
    qDebug() << "═══════════════════════════════════════════════════════";
    qDebug() << "📋 Environment Configuration:";
    qDebug() << "   VSOMEIP_CONFIGURATION:" << qgetenv("VSOMEIP_CONFIGURATION");
    qDebug() << "   COMMONAPI_CONFIG:" << qgetenv("COMMONAPI_CONFIG");
    qDebug() << "   QT_QPA_PLATFORM:" << qgetenv("QT_QPA_PLATFORM");
    qDebug() << "   WAYLAND_DISPLAY:" << qgetenv("WAYLAND_DISPLAY");
    qDebug() << "═══════════════════════════════════════════════════════";

    // ═══════════════════════════════════════════════════════
    // MediaManager backend logic
    // ═══════════════════════════════════════════════════════
    MediaManager mediaManager;

    qDebug() << "";
    qDebug() << "✅ MediaManager initialized";
    qDebug() << "   - Volume:" << mediaManager.volume();
    qDebug() << "   - isPlaying:" << mediaManager.isPlaying();

    // ═══════════════════════════════════════════════════════
    // AmbientThemeManager for ambient color/brightness
    // ═══════════════════════════════════════════════════════
    AmbientThemeManager ambientTheme;

    // ═══════════════════════════════════════════════════════
    // CommonAPI vsomeip Service Registration (MediaControl)
    // ═══════════════════════════════════════════════════════
    std::shared_ptr<CommonAPI::Runtime> runtime = CommonAPI::Runtime::get();

    std::shared_ptr<v1::mediacontrol::MediaControlStubImpl> mediaService =
        std::make_shared<v1::mediacontrol::MediaControlStubImpl>(&mediaManager);

    const std::string domain = "local";
    const std::string instance = "mediacontrol.MediaControl";

    bool success = runtime->registerService(domain, instance, mediaService);

    if (success) {
        qDebug() << "✅ MediaControl vsomeip service registered successfully";
        qDebug() << "   Domain:" << QString::fromStdString(domain);
        qDebug() << "   Instance:" << QString::fromStdString(instance);
    } else {
        qCritical() << "❌ Failed to register MediaControl service!";
        return -1;
    }

    // ═══════════════════════════════════════════════════════
    // AmbientControl Client (subscribe to ambient changes)
    // ═══════════════════════════════════════════════════════
    qDebug() << "";
    qDebug() << "🔧 Initializing AmbientControl Client...";

    AmbientControlClient* ambientClient = new AmbientControlClient(&app);

    if (!ambientClient->initialize()) {
        qCritical() << "❌ Failed to initialize AmbientControl client!";
        return -1;
    }

    qDebug() << "✅ AmbientControl client initialized";
    qDebug() << "   Waiting for AmbientApp service...";

    // Connect ambient changes to theme manager
    QObject::connect(ambientClient, &AmbientControlClient::ambientColorChanged,
                     &ambientTheme, &AmbientThemeManager::onAmbientColorChanged);

    QObject::connect(ambientClient, &AmbientControlClient::brightnessChanged,
                     &ambientTheme, &AmbientThemeManager::onBrightnessChanged);

    qDebug() << "✅ AmbientControl → AmbientThemeManager connection established";

    qDebug() << "";
    qDebug() << "📡 vsomeip Service Status:";
    qDebug() << "   - Provides: MediaControl service";
    qDebug() << "   - Subscribes: AmbientControl service";
    qDebug() << "";
    qDebug() << "MediaApp is running...";
    qDebug() << "═══════════════════════════════════════════════════════";

    // ═══════════════════════════════════════════════════════
    // QML GUI
    // ═══════════════════════════════════════════════════════
    QQmlApplicationEngine engine;

    // Expose C++ objects to QML
    engine.rootContext()->setContextProperty("mediaManager", &mediaManager);
    engine.rootContext()->setContextProperty("ambientTheme", &ambientTheme);

    // Load QML file
    const QUrl url(QStringLiteral("qrc:/qml/MediaApp.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            qCritical() << "❌ Failed to load QML file:" << url;
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);
    engine.load(url);

    if (!engine.rootObjects().isEmpty()) {
        qDebug() << "✅ QML GUI loaded: MediaApp.qml";
        qDebug() << "🖥️  Window should appear now!";
        
        // Desktop-Shell: Set window position and size for main content area
        QObject *rootObject = engine.rootObjects().first();
        if (rootObject) {
            QQuickWindow *window = qobject_cast<QQuickWindow*>(rootObject);
            if (window) {
                window->setGeometry(130, 0, 1790, 1000);  // Main area: right of GearApp
                window->setProperty("_q_waylandAppId", "MediaApp");
                qDebug() << "📐 Window geometry set: (130, 0, 1790, 1000) - Main Content Area";
                qDebug() << "✅ Wayland App ID set: MediaApp";
            } else {
                qWarning() << "⚠️  Failed to cast to QQuickWindow";
            }
        }
    }

    qDebug() << "";

    return app.exec();
}
