// include/PlaybackController.hpp
#pragma once
#include <memory>
#include "IPlayerController.hpp"
#include "Playlist.hpp"

// Music-player behaviour (playlist navigation, auto-advance, volume steps) on top of any
// IPlayerController. Knows nothing about SDL or VLC, so screens stay thin and this logic
// can be unit tested with a fake player.
class PlaybackController {
private:
    std::unique_ptr<IPlayerController> m_player;
    Playlist m_playlist;

    void playCurrent();

public:
    PlaybackController(std::unique_ptr<IPlayerController> player, Playlist playlist);

    void update(); // call once per frame: advances to the next track when one ends

    void togglePlayPause();
    void stop();
    void next();
    void previous(); // restarts the track if more than 3 s in, like most music players
    void changeVolume(int delta);

    PlayerState state() const;
    const Playlist& playlist() const { return m_playlist; }
};
