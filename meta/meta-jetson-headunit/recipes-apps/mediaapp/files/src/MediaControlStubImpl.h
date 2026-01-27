#ifndef MEDIACONTROLSTUBIMPL_H
#define MEDIACONTROLSTUBIMPL_H

#include <v1/mediacontrol/MediaControlStubDefault.hpp>
#include <CommonAPI/CommonAPI.hpp>
#include "mediamanager.h"

namespace v1 {
namespace mediacontrol {

/**
 * MediaControl Service Implementation
 * 
 * MediaApp이 제공하는 vsomeip 서비스
 * - getVolume() RPC 처리
 * - volumeChanged 이벤트 발송
 */
class MediaControlStubImpl : public MediaControlStubDefault {
public:
    MediaControlStubImpl(MediaManager* mediaManager);
    virtual ~MediaControlStubImpl();

    // RPC Method Implementation
    virtual void getVolume(const std::shared_ptr<CommonAPI::ClientId> _client, getVolumeReply_t _reply) override;
    virtual void getCurrentMusic(const std::shared_ptr<CommonAPI::ClientId> _client, getCurrentMusicReply_t _reply) override;

    // Volume 변경 처리 (MediaManager 시그널과 연결)
    void onVolumeChanged(float volume);

    // Track/Playback 변경 처리 (MediaManager 시그널과 연결)
    void onTrackChanged();

private:
    MediaManager* m_mediaManager;  // 기존 MediaManager 활용
};

} // namespace mediacontrol
} // namespace v1

#endif // MEDIACONTROLSTUBIMPL_H
