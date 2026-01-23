#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDebug>

int main(int argc, char *argv[])
{
    // Simple, clean - no environment variables set here
    QGuiApplication app(argc, argv);
    app.setApplicationName("IC_Compositor");
    app.setOrganizationName("IVI");

    qDebug() << "═══════════════════════════════════════════════════════";
    qDebug() << "IC Wayland Compositor Starting";
    qDebug() << "Display: 1024x600 (Instrument Cluster)";
    qDebug() << "Shell: XDG Shell";
    qDebug() << "═══════════════════════════════════════════════════════";

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    
    if (engine.rootObjects().isEmpty()) {
        qCritical() << "Failed to load compositor QML!";
        return -1;
    }

    qDebug() << "✓ Compositor initialized";
    qDebug() << "✓ Listening for XDG Shell applications...";
    qDebug() << "═══════════════════════════════════════════════════════";

    return app.exec();
}
