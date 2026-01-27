#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QTimer>
#include "gearmanager.h"
#include "VehicleControlClient.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("GearApp");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("SEA-ME");
    
    qDebug() << "═══════════════════════════════════════════════════════";
    qDebug() << "GearApp Process Starting...";
    qDebug() << "Service: GearManager (Gear Control + vsomeip Client)";
    qDebug() << "═══════════════════════════════════════════════════════";
    
    // ═══════════════════════════════════════════════════════
    // VehicleControlClient (vsomeip) 생성 및 연결
    // ═══════════════════════════════════════════════════════
    VehicleControlClient vehicleControlClient;
    vehicleControlClient.connectToService();
    
    // ═══════════════════════════════════════════════════════
    // GearManager 백엔드 로직 생성
    // ═══════════════════════════════════════════════════════
    GearManager gearManager;
    
    // ═══════════════════════════════════════════════════════
    // VehicleControlClient → GearManager 연결 (vsomeip 이벤트 수신)
    // ═══════════════════════════════════════════════════════
    QObject::connect(&vehicleControlClient, &VehicleControlClient::currentGearChanged,
                     [&gearManager](const QString& gear) {
                         qDebug() << "[vsomeip → GearManager] Gear update:" << gear;
                         // 같은 기어여도 항상 업데이트 (GUI 동기화 보장)
                         gearManager.setProperty("gearPosition", gear);
                         emit gearManager.gearPositionChanged(gear);
                     });
    
    qDebug() << "✅ Connection established: VehicleControlClient → GearManager";
    
    // ═══════════════════════════════════════════════════════
    // GearManager → VehicleControlClient 연결 (QML에서 기어 변경 요청 시)
    // ═══════════════════════════════════════════════════════
    QObject::connect(&gearManager, &GearManager::gearChangeRequested,
                     [&vehicleControlClient](const QString& gear) {
                         qDebug() << "[GearManager → vsomeip] Requesting gear change:" << gear;
                         vehicleControlClient.requestGearChange(gear);
                     });
    
    qDebug() << "✅ Connection established: GearManager → VehicleControlClient";
    
    // 디버그: Signal 연결 확인
    QObject::connect(&gearManager, &GearManager::gearPositionChanged,
                     [](const QString &gear){ 
                         qDebug() << "[GearApp] gearPositionChanged signal emitted:" << gear; 
                     });
    
    qDebug() << "";
    qDebug() << "✅ GearManager initialized";
    qDebug() << "   - Current Gear:" << gearManager.gearPosition();
    qDebug() << "";
    qDebug() << "✅ VehicleControlClient initialized";
    qDebug() << "   - Connected:" << vehicleControlClient.connected();
    qDebug() << "   - Service: VehicleControl @ ECU1 (192.168.1.100)";
    qDebug() << "";
    qDebug() << "📌 NOTE: vsomeip 통합 완료 - VehicleControlECU와 통신합니다";
    qDebug() << "";
    qDebug() << "GearApp is running...";
    qDebug() << "═══════════════════════════════════════════════════════";
    
    // ═══════════════════════════════════════════════════════
    // QML GUI 로드 (테스트/개발 모드)
    // ═══════════════════════════════════════════════════════
    QQmlApplicationEngine engine;
    
    // C++ 객체를 QML에 노출
    engine.rootContext()->setContextProperty("gearManager", &gearManager);
    engine.rootContext()->setContextProperty("vehicleControlClient", &vehicleControlClient);
    
    // QML 파일 로드
    const QUrl url(QStringLiteral("qrc:/qml/GearSelectionWidget.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            qCritical() << "❌ Failed to load QML file!";
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);
    engine.load(url);
    
    if (!engine.rootObjects().isEmpty()) {
        qDebug() << "✅ QML GUI loaded: GearSelectionWidget.qml";
        qDebug() << "🖥️  Window should appear now!";
    }
    
    qDebug() << "";
    
    // ═══════════════════════════════════════════════════════
    // 테스트: 10초마다 기어 변경 시뮬레이션
    // ═══════════════════════════════════════════════════════
    QTimer *testTimer = new QTimer(&app);
    QStringList gears = {"P", "R", "N", "D"};
    int gearIndex = 0;
    QObject::connect(testTimer, &QTimer::timeout, [&gearManager, &gears, &gearIndex]() {
        gearIndex = (gearIndex + 1) % gears.size();
        QString nextGear = gears[gearIndex];
        qDebug() << "";
        qDebug() << "🧪 [Test] Setting gear to:" << nextGear;
        gearManager.setGearPosition(nextGear);
    });
    // testTimer->start(10000);  // 테스트용 타이머 (필요시 주석 해제)
    
    return app.exec();
}
