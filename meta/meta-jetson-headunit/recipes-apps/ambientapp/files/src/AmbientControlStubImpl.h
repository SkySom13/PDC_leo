#ifndef AMBIENTCONTROLSTUBIMPL_H
#define AMBIENTCONTROLSTUBIMPL_H

#include <v1/ambientcontrol/AmbientControlStubDefault.hpp>
#include <CommonAPI/CommonAPI.hpp>
#include "ambientmanager.h"

namespace v1 {
namespace ambientcontrol {

/**
 * AmbientControl Service Implementation
 * 
 * AmbientApp이 제공하는 vsomeip 서비스
 * - getAmbientColor() RPC 처리
 * - getBrightness() RPC 처리
 * - ambientColorChanged 이벤트 발송
 * - brightnessChanged 이벤트 발송
 */
class AmbientControlStubImpl : public AmbientControlStubDefault {
public:
    AmbientControlStubImpl(AmbientManager* ambientManager);
    virtual ~AmbientControlStubImpl();

    // RPC Method Implementation
    virtual void getAmbientColor(const std::shared_ptr<CommonAPI::ClientId> _client, getAmbientColorReply_t _reply) override;
    virtual void getBrightness(const std::shared_ptr<CommonAPI::ClientId> _client, getBrightnessReply_t _reply) override;

    // Color/Brightness 변경 처리 (AmbientManager 시그널과 연결)
    void onAmbientColorChanged();
    void onBrightnessChanged();

private:
    AmbientManager* m_ambientManager;  // 기존 AmbientManager 활용
};

} // namespace ambientcontrol
} // namespace v1

#endif // AMBIENTCONTROLSTUBIMPL_H
