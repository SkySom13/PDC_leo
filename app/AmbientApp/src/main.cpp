#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QTimer>
#include <QDir>
#include <QWindow>
#include <QQuickWindow>
#include <CommonAPI/CommonAPI.hpp>
#include "ambientmanager.h"
#include "MediaControlClient.h"
#include "VehicleControlClient.h"
#include "AmbientControlStubImpl.h"

int main(int argc, char *argv[])
{
    // ═══════════════════════════════════════════════════════════
    // Environment variables - use deployment paths if not already set
    // ═══════════════════════════════════════════════════════════
    if (qgetenv("VSOMEIP_APPLICATION_NAME").isEmpty()) {
        qputenv("VSOMEIP_APPLICATION_NAME", "AmbientApp");
    }
    if (qgetenv("VSOMEIP_CONFIGURATION").isEmpty()) {
        qputenv("VSOMEIP_CONFIGURATION", "/etc/vsomeip/vsomeip_ambient.json");
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
    app.setApplicationName("AmbientApp");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("SEA-ME");
    
    // Wayland App ID 설정 (Compositor가 앱을 식별하는데 사용)
    app.setDesktopFileName("AmbientApp.desktop");

    qDebug() << "═══════════════════════════════════════════════════════";
    qDebug() << "AmbientApp Process Starting...";
    qDebug() << "Service: AmbientManager (Ambient Lighting Control)";
    qDebug() << "═══════════════════════════════════════════════════════";
    qDebug() << "📋 Environment Configuration:";
    qDebug() << "   VSOMEIP_CONFIGURATION:" << qgetenv("VSOMEIP_CONFIGURATION");
    qDebug() << "   COMMONAPI_CONFIG:" << qgetenv("COMMONAPI_CONFIG");
    qDebug() << "   QT_QPA_PLATFORM:" << qgetenv("QT_QPA_PLATFORM");
    qDebug() << "   WAYLAND_DISPLAY:" << qgetenv("WAYLAND_DISPLAY");
    qDebug() << "═══════════════════════════════════════════════════════";

    // ═══════════════════════════════════════════════════════════
    // AmbientManager 생성
    // ═══════════════════════════════════════════════════════════
    AmbientManager ambientManager;

    // ═══════════════════════════════════════════════════════════
    // AmbientControl 서비스 등록 (다른 앱이 구독할 수 있도록)
    // ═══════════════════════════════════════════════════════════
    qDebug() << "";
    qDebug() << "🔧 Registering AmbientControl Service...";

    std::shared_ptr<CommonAPI::Runtime> runtime = CommonAPI::Runtime::get();
    std::shared_ptr<v1::ambientcontrol::AmbientControlStubImpl> ambientControlService =
        std::make_shared<v1::ambientcontrol::AmbientControlStubImpl>(&ambientManager);

    const std::string domain = "local";
    const std::string instance = "ambientcontrol.AmbientControl";

    bool serviceRegistered = runtime->registerService(domain, instance, ambientControlService);

    if (!serviceRegistered) {
        qCritical() << "❌ Failed to register AmbientControl service!";
        return -1;
    }

    qDebug() << "✅ AmbientControl service registered successfully";
    qDebug() << "   Domain:" << QString::fromStdString(domain);
    qDebug() << "   Instance:" << QString::fromStdString(instance);
    qDebug() << "   Other apps can now subscribe to ambient color/brightness changes";

    // ═══════════════════════════════════════════════════════════
    // vSOMEIP 클라이언트 초기화
    // ═══════════════════════════════════════════════════════════
    qDebug() << "";
    qDebug() << "🔧 Initializing vSOMEIP Clients...";
    
    // MediaControl Client (볼륨 → 밝기)
    MediaControlClient* mediaControlClient = new MediaControlClient(&ambientManager, &app);
    
    if (!mediaControlClient->initialize()) {
        qCritical() << "❌ Failed to initialize MediaControl client!";
        return -1;
    }
    
    qDebug() << "✅ MediaControl client initialized";
    qDebug() << "   Waiting for MediaApp service...";
    
    // VehicleControl Client (기어 → 색상)
    VehicleControlClient* vehicleControlClient = new VehicleControlClient(&app);
    
    if (!vehicleControlClient->initialize()) {
        qCritical() << "❌ Failed to initialize VehicleControl client!";
        return -1;
    }
    
    qDebug() << "✅ VehicleControl client initialized";
    qDebug() << "   Waiting for VehicleControlECU service...";
    
    // Connect VehicleControl gear changes to AmbientManager
    QObject::connect(vehicleControlClient, &VehicleControlClient::currentGearChanged,
                     &ambientManager, &AmbientManager::onGearPositionChanged);
    
    qDebug() << "✅ VehicleControl → AmbientManager connection established";
    qDebug() << "   (Gear changes will update ambient light color)";
    qDebug() << "";

    // Debug: Signal 테스트
    QObject::connect(&ambientManager, &AmbientManager::ambientColorChanged,
                     [&ambientManager](){
                         qDebug() << "[AmbientApp] ambientColorChanged signal emitted:"
                                  << ambientManager.ambientColor();
                     });

    QObject::connect(&ambientManager, &AmbientManager::brightnessChanged,
                     [&ambientManager](){
                         qDebug() << "[AmbientApp] brightnessChanged signal emitted:"
                                  << ambientManager.brightness();
                     });

    qDebug() << "";
    qDebug() << "✅ AmbientManager initialized";
    qDebug() << "   - Current Color:" << ambientManager.ambientColor();
    qDebug() << "   - Brightness:" << ambientManager.brightness();
    qDebug() << "";
    qDebug() << "📋 Communication Setup:";
    qDebug() << "   - MediaApp → AmbientApp (Volume → Brightness)";
    qDebug() << "   - VehicleControlECU → AmbientApp (Gear → Color)";
    qDebug() << "";
    qDebug() << "AmbientApp is running...";
    qDebug() << "═══════════════════════════════════════════════════════";

    // ═══════════════════════════════════════════════════════════
    // QML GUI 로드
    // ═══════════════════════════════════════════════════════════
    QQmlApplicationEngine engine;

    // C++ 객체를 QML에 노출
    engine.rootContext()->setContextProperty("ambientManager", &ambientManager);

    // QML 파일 로드
    const QUrl url(QStringLiteral("qrc:/qml/AmbientLighting.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            qCritical() << "❌ Failed to load QML file!";
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);
    engine.load(url);

    if (!engine.rootObjects().isEmpty()) {
        qDebug() << "✅ QML GUI loaded: AmbientLighting.qml";
        qDebug() << "   Window should appear now!";
        
        // Desktop-Shell: Set window position and size for main content area
        QObject *rootObject = engine.rootObjects().first();
        if (rootObject) {
            QQuickWindow *window = qobject_cast<QQuickWindow*>(rootObject);
            if (window) {
                window->setGeometry(130, 0, 1790, 1000);  // Main area: right of GearApp
                window->setProperty("_q_waylandAppId", "AmbientApp");
                qDebug() << "📐 Window geometry set: (130, 0, 1790, 1000) - Main Content Area";
                qDebug() << "✅ Wayland App ID set: AmbientApp";
            } else {
                qWarning() << "⚠️  Failed to cast to QQuickWindow";
            }
        }
    }

    qDebug() << "";

    return app.exec();
}
