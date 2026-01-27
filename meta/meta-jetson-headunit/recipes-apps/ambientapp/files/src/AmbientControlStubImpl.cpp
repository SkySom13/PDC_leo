#include "AmbientControlStubImpl.h"
#include <QDebug>

namespace v1 {
namespace ambientcontrol {

AmbientControlStubImpl::AmbientControlStubImpl(AmbientManager* ambientManager)
    : m_ambientManager(ambientManager)
{
    qDebug() << "[AmbientApp] AmbientControlStubImpl: Service initialized";

    // AmbientManager의 ambientColorChanged 시그널을 vsomeip 이벤트로 연결
    if (m_ambientManager) {
        QObject::connect(m_ambientManager, &AmbientManager::ambientColorChanged,
                        [this]() {
                            onAmbientColorChanged();
                        });

        // AmbientManager의 brightnessChanged 시그널을 vsomeip 이벤트로 연결
        QObject::connect(m_ambientManager, &AmbientManager::brightnessChanged,
                        [this]() {
                            onBrightnessChanged();
                        });
    }
}

AmbientControlStubImpl::~AmbientControlStubImpl() {
    qDebug() << "[AmbientApp] AmbientControlStubImpl: Service destroyed";
}

void AmbientControlStubImpl::getAmbientColor(const std::shared_ptr<CommonAPI::ClientId> _client, getAmbientColorReply_t _reply) {
    std::string color = "";

    if (m_ambientManager) {
        QString qColor = m_ambientManager->ambientColor();
        color = qColor.toStdString();
    }

    qDebug() << "[AmbientApp] AmbientControlStubImpl: getAmbientColor() called, returning:" << QString::fromStdString(color);
    _reply(color);
}

void AmbientControlStubImpl::getBrightness(const std::shared_ptr<CommonAPI::ClientId> _client, getBrightnessReply_t _reply) {
    float brightness = 0.0f;

    if (m_ambientManager) {
        brightness = static_cast<float>(m_ambientManager->brightness());
    }

    qDebug() << "[AmbientApp] AmbientControlStubImpl: getBrightness() called, returning:" << brightness;
    _reply(brightness);
}

void AmbientControlStubImpl::onAmbientColorChanged() {
    std::string color = "";

    if (m_ambientManager) {
        QString qColor = m_ambientManager->ambientColor();
        color = qColor.toStdString();
    }

    qDebug() << "[AmbientApp] AmbientControlStubImpl: Ambient color changed to:" << QString::fromStdString(color);
    // 부모 클래스의 fireAmbientColorChangedEvent 메서드 호출
    AmbientControlStubDefault::fireAmbientColorChangedEvent(color);
    qDebug() << "[AmbientApp] AmbientControlStubImpl: ambientColorChanged event broadcasted";
}

void AmbientControlStubImpl::onBrightnessChanged() {
    float brightness = 0.0f;

    if (m_ambientManager) {
        brightness = static_cast<float>(m_ambientManager->brightness());
    }

    qDebug() << "[AmbientApp] AmbientControlStubImpl: Brightness changed to:" << brightness;
    // 부모 클래스의 fireBrightnessChangedEvent 메서드 호출
    AmbientControlStubDefault::fireBrightnessChangedEvent(brightness);
    qDebug() << "[AmbientApp] AmbientControlStubImpl: brightnessChanged event broadcasted";
}

} // namespace ambientcontrol
} // namespace v1
