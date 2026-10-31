// src/PlaybackController.cpp
#include "PlaybackController.hpp"
#include <utility>

namespace {
constexpr int kRestartThresholdMs = 3000;
}

PlaybackController::PlaybackController(std::unique_ptr<IPlayerController> player, Playlist playlist)
    : m_player(std::move(player)), m_playlist(std::move(playlist)) {}

void PlaybackController::playCurrent() {
    if (!m_playlist.empty()) {
        m_player->play(m_playlist.current());
    }
}

void PlaybackController::update() {
    if (m_player->getState().status == PlaybackStatus::Ended) {
        next();
    }
}

void PlaybackController::togglePlayPause() {
    switch (m_player->getState().status) {
        case PlaybackStatus::Playing:
        case PlaybackStatus::Paused:  m_player->togglePause(); break;
        case PlaybackStatus::Opening: break;
        default:                      playCurrent(); break;
    }
}

void PlaybackController::stop() {
    m_player->stop();
}

void PlaybackController::next() {
    if (m_playlist.empty()) return;
    m_playlist.next();
    playCurrent();
}

void PlaybackController::previous() {
    if (m_playlist.empty()) return;
    if (m_player->getState().positionMs > kRestartThresholdMs) {
        m_player->seek(0);
        return;
    }
    m_playlist.previous();
    playCurrent();
}

void PlaybackController::changeVolume(int delta) {
    m_player->setVolume(m_player->getState().volume + delta);
}

PlayerState PlaybackController::state() const {
    return m_player->getState();
}
