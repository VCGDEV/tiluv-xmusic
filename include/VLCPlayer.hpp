// include/VLCPlayer.hpp
#pragma once
#include "IPlayerController.hpp"
#include <vlc/vlc.h>
#include <mutex>

// LibVLC implementation of IPlayerController. Metadata is parsed asynchronously and
// picked up lazily in getState(), so no VLC callbacks ever run into our code.
class VLCPlayer : public IPlayerController {
private:
    libvlc_instance_t* m_vlcInstance;
    libvlc_media_player_t* m_mediaPlayer;
    libvlc_media_t* m_media; // kept alive so getState() can read its parsed tags

    mutable std::mutex m_mutex;
    mutable TrackInfo m_currentTrack;
    mutable bool m_metadataReady;
    int m_volume;

    void releaseMedia();
    void refreshMetadata() const; // caller must hold m_mutex

public:
    VLCPlayer();
    ~VLCPlayer() override;
    VLCPlayer(const VLCPlayer&) = delete;
    VLCPlayer& operator=(const VLCPlayer&) = delete;

    bool isReady() const override;

    void play(const std::string& filepath) override;
    void togglePause() override;
    void stop() override;
    void seek(int positionMs) override;
    void setVolume(int volume) override;

    PlayerState getState() const override;
};
