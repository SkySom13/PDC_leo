#include "mediamanager.h"
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QDebug>
#include <QUrl>
#include <QSet>

const QStringList MediaManager::s_supportedFormats = { //지원하는 미디어 파일 확장자 목록
    "mp3", "wav", "flac", "m4a", "aac", "ogg", "wma"
};

MediaManager::MediaManager(QObject *parent)
    : QObject(parent)
    , m_isPlaying(false)
    , m_currentIndex(-1)
    , m_volume(0.8)
    , m_usbWatcher(new QFileSystemWatcher(this))
    , m_scanTimer(new QTimer(this))
    , m_mediaPlayer(new QMediaPlayer(this))
    , m_playlist(new QMediaPlaylist(this))
{
    // USB 디렉터리 감시
    m_usbWatcher->addPath("/run/media");  // Raspberry Pi systemd mount point
    m_usbWatcher->addPath("/media");
    m_usbWatcher->addPath("/mnt");
    connect(m_usbWatcher, &QFileSystemWatcher::directoryChanged,
            this, &MediaManager::onUsbDeviceChanged);
    
    // 디바운스용 타이머
    m_scanTimer->setSingleShot(true);
    m_scanTimer->setInterval(2000);
    connect(m_scanTimer, &QTimer::timeout,
            this, &MediaManager::scanForMedia);
    
    // 미디어 플레이어 설정
    m_mediaPlayer->setPlaylist(m_playlist);
    m_mediaPlayer->setVolume(m_volume * 100);
    connect(m_mediaPlayer, QOverload<QMediaPlayer::MediaStatus>::of(&QMediaPlayer::mediaStatusChanged),
            this, &MediaManager::onMediaStatusChanged);
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged,
            this, &MediaManager::onPositionChanged);
    connect(m_mediaPlayer, QOverload<QMediaPlayer::State>::of(&QMediaPlayer::stateChanged),
            [this](QMediaPlayer::State state) {
                m_isPlaying = (state == QMediaPlayer::PlayingState);
                emit playbackStateChanged();
            });

    // 초기 스캔
    QTimer::singleShot(1000, this, &MediaManager::scanForMedia); // 앱 시작 후 1초 뒤 usb 스캔
}

void MediaManager::scanForMedia()
{
    qDebug() << "MediaManager: Scanning for USB media files...";

    m_mediaFiles.clear();
    refreshUsbMountsInternal();

    for (const QString &mountPoint : m_usbMounts) {
        qDebug() << "MediaManager: Scanning USB mount:" << mountPoint;
        scanDirectory(mountPoint);
    }

    updatePlaylist();
    emit mediaFilesChanged();

    qDebug() << "MediaManager: Scan complete. Found" << m_mediaFiles.size() << "media files";

    if (!m_mediaFiles.isEmpty() && m_currentIndex < 0) {
        m_currentIndex = 0;
        updateCurrentFile();
        emit currentIndexChanged();
        qDebug() << "MediaManager: Auto-selected first media file:" << m_currentFile;
    }
}

void MediaManager::scanDirectory(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) {
        qDebug() << "MediaManager: Directory does not exist:" << path;
        return;
    }

    QDirIterator it(path, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = it.next();
        if (isMediaFile(filePath)) {
            // Check for duplicates before adding
            if (!m_mediaFiles.contains(filePath)) {
                m_mediaFiles.append(filePath);
            } else {
                qDebug() << "MediaManager: Skipping duplicate file:" << filePath;
            }
        }
    }
}

bool MediaManager::isMediaFile(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();
    return s_supportedFormats.contains(suffix);
}

void MediaManager::updatePlaylist()
{
    m_playlist->clear();
    for (const QString &filePath : m_mediaFiles) {
        m_playlist->addMedia(QUrl::fromLocalFile(filePath));
    }
}

void MediaManager::updateCurrentFile()
{
    if (m_currentIndex >= 0 && m_currentIndex < m_mediaFiles.size()) {
        QString newCurrentFile = m_mediaFiles.at(m_currentIndex);
        if (m_currentFile != newCurrentFile) {
            m_currentFile = newCurrentFile;
            emit currentFileChanged();
        }
    } else {
        if (!m_currentFile.isEmpty()) {
            m_currentFile.clear();
            emit currentFileChanged();
        }
    }
}

void MediaManager::playFile(int index)
{
    if (index >= 0 && index < m_mediaFiles.size()) {
        m_currentIndex = index;
        m_playlist->setCurrentIndex(index);
        m_mediaPlayer->play();
        updateCurrentFile();
        emit currentIndexChanged();
    }
}

void MediaManager::play()
{
    qDebug() << "MediaManager: play() called. Current isPlaying:" << m_isPlaying;
    qDebug() << "MediaManager: Playlist count:" << m_playlist->mediaCount();

    if (m_playlist->mediaCount() > 0) {
        if (m_currentIndex < 0) {
            m_currentIndex = 0;
            m_playlist->setCurrentIndex(0);
            updateCurrentFile();
            emit currentIndexChanged();
        }

        qDebug() << "MediaManager: Calling QMediaPlayer::play()";
        m_mediaPlayer->play();

        // Manually update state for embedded systems where state signal might not fire
        if (!m_isPlaying) {
            m_isPlaying = true;
            emit playbackStateChanged();
            qDebug() << "MediaManager: State changed to PLAYING";
        } else {
            qDebug() << "MediaManager: Already in PLAYING state";
        }
    } else {
        qDebug() << "MediaManager: Cannot play - playlist is empty";
    }
}

void MediaManager::pause()
{
    qDebug() << "MediaManager: pause() called. Current isPlaying:" << m_isPlaying;
    qDebug() << "MediaManager: Calling QMediaPlayer::pause()";
    m_mediaPlayer->pause();

    // Manually update state for embedded systems where state signal might not fire
    if (m_isPlaying) {
        m_isPlaying = false;
        emit playbackStateChanged();
        qDebug() << "MediaManager: State changed to PAUSED";
    } else {
        qDebug() << "MediaManager: Already in PAUSED state";
    }
}

void MediaManager::stop()
{
    m_mediaPlayer->stop();
    m_currentIndex = -1;
    updateCurrentFile();
    emit currentIndexChanged();
}

void MediaManager::next()
{
    if (m_mediaFiles.isEmpty())
        return;

    // Circular navigation: wrap to first track if at the end
    if (m_currentIndex < m_mediaFiles.size() - 1) {
        playFile(m_currentIndex + 1);
    } else {
        playFile(0);  // Go to first track
        qDebug() << "MediaManager: Wrapped to first track";
    }
}

void MediaManager::previous()
{
    if (m_mediaFiles.isEmpty())
        return;

    // Circular navigation: wrap to last track if at the beginning
    if (m_currentIndex > 0) {
        playFile(m_currentIndex - 1);
    } else {
        playFile(m_mediaFiles.size() - 1);  // Go to last track
        qDebug() << "MediaManager: Wrapped to last track";
    }
}

void MediaManager::onUsbDeviceChanged()
{
    qDebug() << "MediaManager: USB device change detected";
    m_scanTimer->start();
}

void MediaManager::onMediaStatusChanged() {}
void MediaManager::onPositionChanged(qint64 position) { Q_UNUSED(position) }

// USB 관련 메서드들
QStringList MediaManager::listUsbMounts()
{
    refreshUsbMountsInternal();
    return m_usbMounts;
}

QStringList MediaManager::scanUsbAt(const QString& path)
{
    qDebug() << "MediaManager: Scanning USB at:" << path;
    
    m_mediaFiles.clear();
    scanDirectory(path);
    
    updatePlaylist();
    emit mediaFilesChanged();
    
    if (!m_mediaFiles.isEmpty() && m_currentIndex < 0) {
        m_currentIndex = 0;
        updateCurrentFile();
        emit currentIndexChanged();
    }
    
    qDebug() << "MediaManager: Found" << m_mediaFiles.size() << "media files in" << path;
    return m_mediaFiles;
}

QStringList MediaManager::scanAllUsbMounts()
{
    qDebug() << "MediaManager: Scanning all USB mounts...";
    
    m_mediaFiles.clear();
    refreshUsbMountsInternal();
    
    for (const QString& mountPoint : m_usbMounts) {
        scanDirectory(mountPoint);
    }
    
    updatePlaylist();
    emit mediaFilesChanged();
    
    if (!m_mediaFiles.isEmpty() && m_currentIndex < 0) {
        m_currentIndex = 0;
        updateCurrentFile();
        emit currentIndexChanged();
    }
    
    qDebug() << "MediaManager: Found" << m_mediaFiles.size() << "total media files";
    return m_mediaFiles;
}

void MediaManager::refreshUsbMounts()
{
    refreshUsbMountsInternal();
}

void MediaManager::refreshUsbMountsInternal()
{
    m_usbMounts.clear();
    QSet<QString> seenDevices;  // Track devices we've already added
    const auto vols = QStorageInfo::mountedVolumes();

    for (const auto& v : vols) {
        if (!v.isValid() || !v.isReady()) continue;
        QString root = v.rootPath();
        QString device = v.device();
        QString fsType = v.fileSystemType();

        // Skip EFI partitions
        if (root.contains("EFI", Qt::CaseInsensitive) ||
            fsType == "vfat" && root.contains("sda1")) {
            qDebug() << "MediaManager: Skipping EFI partition:" << root;
            continue;
        }

        // Skip SD card boot partition
        if (root.contains("mmcblk0p1") || root.contains("boot")) {
            qDebug() << "MediaManager: Skipping boot partition:" << root;
            continue;
        }

        // Skip if we've already seen this device
        if (seenDevices.contains(device)) {
            qDebug() << "MediaManager: Skipping duplicate mount:" << root << "for device:" << device;
            continue;
        }

        // Check if it's a USB device
        // Ubuntu: /media/<username>/<drive>
        // Raspberry Pi: /media/usb*, /run/media/<username>/<drive>, /mnt/*
        bool isUsbDevice = (root.startsWith("/media/usb") ||
                           root.startsWith("/run/media") ||
                           root.startsWith("/mnt") ||
                           root.startsWith("/media/")) &&
                           device.startsWith("/dev/sd");

        // Additional filter: skip root filesystem and bare /media directory
        if (root == "/" || root == "/media") {
            qDebug() << "MediaManager: Skipping system partition:" << root;
            continue;
        }

        if (isUsbDevice) {
            m_usbMounts << root;
            seenDevices.insert(device);
            qDebug() << "MediaManager: Added USB mount:" << root << "device:" << device;
        }
    }

    m_usbMounts.removeDuplicates();
    emit usbMountsChanged();
}

void MediaManager::setVolume(qreal volume)
{
    volume = qBound(0.0, volume, 1.0);
    if (qAbs(m_volume - volume) > 0.001) {
        m_volume = volume;
        int playerVolume = static_cast<int>(volume * 100);
        m_mediaPlayer->setVolume(playerVolume);
        emit volumeChanged();
    }
}
