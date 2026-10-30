// include/IPlayerController.hpp
// Backend-agnostic audio player contract. Nothing here may include VLC or SDL headers:
// the UI and PlaybackController only ever see this interface.
#pragma once
#include <string>

struct TrackInfo {
    std::string filePath;
    std::string title;       // falls back to the file name when the tag is missing
    std::string artist;
    std::string album;
    std::string artworkPath; // embedded artwork extracted by the backend, empty if none
    int durationMs = 0;
};

enum class PlaybackStatus { Idle, Opening, Playing, Paused, Stopped, Ended, Error };

// Snapshot of the player, polled by the UI once per frame.
struct PlayerState {
    PlaybackStatus status = PlaybackStatus::Idle;
    TrackInfo track;
    bool metadataReady = false; // true once tags/artwork have been read
    int positionMs = 0;
    int volume = 0;             // 0 to 100
};

class IPlayerController {
public:
    virtual ~IPlayerController() = default;

    virtual bool isReady() const = 0; // false if the backend failed to start

    virtual void play(const std::string& filepath) = 0;
    virtual void togglePause() = 0;
    virtual void stop() = 0;
    virtual void seek(int positionMs) = 0;
    virtual void setVolume(int volume) = 0; // 0 to 100

    virtual PlayerState getState() const = 0;
};
