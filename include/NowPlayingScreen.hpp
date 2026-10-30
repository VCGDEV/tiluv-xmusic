// include/NowPlayingScreen.hpp
#pragma once
#include <string>
#include "CoverResolver.hpp"
#include "IScreen.hpp"
#include "PlaybackController.hpp"

class NowPlayingScreen : public IScreen {
private:
    PlaybackController& m_controller;
    const SkinConfig& m_skin;
    CoverResolver m_covers;

    PlayerState m_state;
    std::string m_coverPath;
    // What m_coverPath was resolved for; re-resolve when the track or its metadata changes.
    std::string m_coverTrack;
    bool m_coverHadMetadata = false;

public:
    NowPlayingScreen(PlaybackController& controller, const SkinConfig& skin);

    void handleAction(InputAction action) override;
    void update() override;
    void render(Canvas& canvas) override;
};
