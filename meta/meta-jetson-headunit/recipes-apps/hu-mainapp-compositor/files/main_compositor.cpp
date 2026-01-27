#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QSurfaceFormat>

int main(int argc, char *argv[])
{
    // ═══════════════════════════════════════════════════════
    // HU_MainApp - Nested Wayland Compositor (Weston client)
    // ═══════════════════════════════════════════════════════
    // IMPORTANT: HU_MainApp runs as a NESTED Wayland compositor
    // - Connects to Weston (wayland-0) as a client
    // - Shows on HDMI output via Weston (Kiosk Shell routing)
    // - Creates sub-compositor socket (wayland-1) for HU apps
    // - Manages window layout and surface routing for HU apps

    // Set platform to Wayland (connect to Weston)
    qputenv("QT_QPA_PLATFORM", "wayland");

    // Disable window decorations
    qputenv("QT_WAYLAND_DISABLE_WINDOWDECORATION", "1");

    // Ensure XDG shell is used
    qputenv("QT_WAYLAND_SHELL_INTEGRATION", "xdg-shell");

    // Create XDG_RUNTIME_DIR if not set
    if (qgetenv("XDG_RUNTIME_DIR").isEmpty()) {
        qputenv("XDG_RUNTIME_DIR", "/run/user/0");
    }

    // This app connects to Weston's wayland-0
    // And creates its own compositor socket wayland-1 for HU apps
    // Note: The nested compositor socket name will be set by Qt Wayland Compositor

    // ═══════════════════════════════════════════════════════
    // Configure OpenGL surface format for compositor
    // ═══════════════════════════════════════════════════════
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(2, 0);  // OpenGL ES 2.0 for embedded systems
    format.setRenderableType(QSurfaceFormat::OpenGLES);
    format.setSwapBehavior(QSurfaceFormat::SingleBuffer);  // Single buffer for lower latency
    QSurfaceFormat::setDefaultFormat(format);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);  // Share GL context between surfaces
#endif

    QGuiApplication app(argc, argv);

    // ═══════════════════════════════════════════════════════
    // KIOSK SHELL: Set application name for display routing
    // ═══════════════════════════════════════════════════════
    // This name must match the app-ids in weston.ini
    app.setApplicationName("HeadUnitApp");  // ← Routes to HDMI-A-1 output
    app.setApplicationVersion("2.0-Kiosk");
    app.setOrganizationName("SEA-ME");
    app.setDesktopFileName("HeadUnitApp");  // Critical for Wayland app_id

    qDebug() << "═══════════════════════════════════════════════════════";
    qDebug() << "HU_MainApp - Nested Wayland Compositor (Kiosk Shell)";
    qDebug() << "App ID: HeadUnitApp → HDMI-A-1 (1024x600)";
    qDebug() << "═══════════════════════════════════════════════════════";
    qDebug() << "Display Platform:" << app.platformName();
    qDebug() << "Parent Compositor:" << qgetenv("WAYLAND_DISPLAY");
    qDebug() << "";
    qDebug() << "📋 Role: Nested Wayland Compositor";
    qDebug() << "   - Client of Weston (wayland-0)";
    qDebug() << "   - Shows on HDMI via Kiosk Shell routing";
    qDebug() << "   - Creates wayland-1 socket for HU apps";
    qDebug() << "   - Manages app window embedding";
    qDebug() << "═══════════════════════════════════════════════════════";

    // ═══════════════════════════════════════════════════════
    // QML Engine - Compositor UI
    // ═══════════════════════════════════════════════════════
    QQmlApplicationEngine engine;

    // Add QML import path for Qt modules
    engine.addImportPath("/usr/lib/qml");

    // Load compositor QML
    const QUrl url(QStringLiteral("qrc:/qml/compositor_modular.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) {
                qCritical() << "❌ Failed to load Compositor QML:" << url;
                QCoreApplication::exit(-1);
            } else {
                qDebug() << "";
                qDebug() << "✅ Compositor UI loaded";
                qDebug() << "   Ready to embed app windows";
                qDebug() << "";
            }
        },
        Qt::QueuedConnection);

    engine.load(url);

    if (engine.rootObjects().isEmpty()) {
        qCritical() << "❌ No root objects found!";
        return -1;
    }

    qDebug() << "🚀 Compositor running...";
    qDebug() << "   Waiting for HU apps to connect...";
    qDebug() << "";

    return app.exec();
}
