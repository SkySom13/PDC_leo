#include "MediaControlStubImpl.h"
#include <QDebug>
#include <QFileInfo>

namespace v1 {
namespace mediacontrol {

MediaControlStubImpl::MediaControlStubImpl(MediaManager* mediaManager)
    : m_mediaManager(mediaManager)
{
    qDebug() << "[MediaApp] MediaControlStubImpl: Service initialized";

    // MediaManager의 volumeChanged 시그널을 vsomeip 이벤트로 연결
    if (m_mediaManager) {
        QObject::connect(m_mediaManager, &MediaManager::volumeChanged,
                        [this]() {
                            float volume = m_mediaManager->volume();
                            onVolumeChanged(volume);
                        });

        // MediaManager의 track/playback 변경 시그널을 vsomeip 이벤트로 연결
        QObject::connect(m_mediaManager, &MediaManager::currentFileChanged,
                        [this]() {
                            onTrackChanged();
                        });

        QObject::connect(m_mediaManager, &MediaManager::playbackStateChanged,
                        [this]() {
                            onTrackChanged();
                        });
    }
}

MediaControlStubImpl::~MediaControlStubImpl() {
    qDebug() << "[MediaApp] MediaControlStubImpl: Service destroyed";
}

void MediaControlStubImpl::getVolume(const std::shared_ptr<CommonAPI::ClientId> _client, getVolumeReply_t _reply) {
    float currentVolume = 0.0f;
    
    if (m_mediaManager) {
        currentVolume = static_cast<float>(m_mediaManager->volume());
    }
    
    qDebug() << "[MediaApp] MediaControlStubImpl: getVolume() called, returning:" << currentVolume;
    _reply(currentVolume);
}

void MediaControlStubImpl::getCurrentMusic(const std::shared_ptr<CommonAPI::ClientId> _client, getCurrentMusicReply_t _reply) {
    std::string title = "";
    bool isPlaying = false;

    if (m_mediaManager) {
        QString currentFile = m_mediaManager->currentFile();

        // Extract filename from full path
        if (!currentFile.isEmpty()) {
            QFileInfo fileInfo(currentFile);
            title = fileInfo.fileName().toStdString();
        }

        isPlaying = m_mediaManager->isPlaying();
    }

    qDebug() << "[MediaApp] MediaControlStubImpl: getCurrentMusic() called, returning:"
             << QString::fromStdString(title) << "playing:" << isPlaying;
    _reply(title, isPlaying);
}

void MediaControlStubImpl::onVolumeChanged(float volume) {
    qDebug() << "[MediaApp] MediaControlStubImpl: Volume changed to:" << volume;
    // 부모 클래스의 fireVolumeChangedEvent 메서드 호출
    MediaControlStubDefault::fireVolumeChangedEvent(volume);
    qDebug() << "[MediaApp] MediaControlStubImpl: volumeChanged event broadcasted";
}

void MediaControlStubImpl::onTrackChanged() {
    std::string title = "";
    bool isPlaying = false;

    if (m_mediaManager) {
        QString currentFile = m_mediaManager->currentFile();

        // Extract filename from full path
        if (!currentFile.isEmpty()) {
            QFileInfo fileInfo(currentFile);
            title = fileInfo.fileName().toStdString();
        }

        isPlaying = m_mediaManager->isPlaying();
    }

    qDebug() << "[MediaApp] MediaControlStubImpl: Track/playback changed to:"
             << QString::fromStdString(title) << "playing:" << isPlaying;
    // 부모 클래스의 fireCurrentMusicChangedEvent 메서드 호출
    MediaControlStubDefault::fireCurrentMusicChangedEvent(title, isPlaying);
    qDebug() << "[MediaApp] MediaControlStubImpl: currentMusicChanged event broadcasted";
}

} // namespace mediacontrol
} // namespace v1
