// tests/FakePlayer.hpp
// In-memory IPlayerController: records calls and lets a test set the state directly.
#pragma once
#include <algorithm>
#include <string>
#include <vector>
#include "IPlayerController.hpp"

class FakePlayer : public IPlayerController {
public:
    PlayerState state;
    std::vector<std::string> played; // every play() call, in order
    std::vector<int> seeks;
    int togglePauseCalls = 0;
    int stopCalls = 0;

    bool isReady() const override { return true; }

    void play(const std::string& filepath) override {
        played.push_back(filepath);
        state.status = PlaybackStatus::Playing;
        state.track.filePath = filepath;
        state.positionMs = 0;
    }

    void togglePause() override {
        ++togglePauseCalls;
        state.status = state.status == PlaybackStatus::Playing ? PlaybackStatus::Paused : PlaybackStatus::Playing;
    }

    void stop() override {
        ++stopCalls;
        state.status = PlaybackStatus::Stopped;
    }

    void seek(int positionMs) override {
        seeks.push_back(positionMs);
        state.positionMs = positionMs;
    }

    // Clamps like VLCPlayer, as the interface promises 0 to 100.
    void setVolume(int volume) override { state.volume = std::clamp(volume, 0, 100); }

    PlayerState getState() const override { return state; }
};
