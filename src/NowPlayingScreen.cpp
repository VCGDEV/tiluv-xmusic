// src/NowPlayingScreen.cpp
#include "NowPlayingScreen.hpp"
#include <cstdio>

namespace {

constexpr int kVolumeStep = 5;

std::string formatTime(int ms) {
    const int totalSeconds = ms / 1000;
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%d:%02d", totalSeconds / 60, totalSeconds % 60);
    return buffer;
}

const char* statusLabel(PlaybackStatus status) {
    switch (status) {
        case PlaybackStatus::Opening: return "LOADING";
        case PlaybackStatus::Playing: return "PLAYING";
        case PlaybackStatus::Paused:  return "PAUSED";
        case PlaybackStatus::Stopped: return "STOPPED";
        case PlaybackStatus::Error:   return "ERROR";
        default:                      return "READY";
    }
}

} // namespace

NowPlayingScreen::NowPlayingScreen(PlaybackController& controller, const SkinConfig& skin)
    : m_controller(controller), m_skin(skin), m_covers(skin) {}

void NowPlayingScreen::handleAction(InputAction action) {
    switch (action) {
        case InputAction::Select:
        case InputAction::PlayPause:  m_controller.togglePlayPause(); break;
        case InputAction::Stop:       m_controller.stop(); break;
        case InputAction::Right:
        case InputAction::Next:       m_controller.next(); break;
        case InputAction::Left:
        case InputAction::Previous:   m_controller.previous(); break;
        case InputAction::Up:
        case InputAction::VolumeUp:   m_controller.changeVolume(kVolumeStep); break;
        case InputAction::Down:
        case InputAction::VolumeDown: m_controller.changeVolume(-kVolumeStep); break;
        default: break;
    }
}

void NowPlayingScreen::update() {
    m_state = m_controller.state();
    if (m_state.track.filePath != m_coverTrack || m_state.metadataReady != m_coverHadMetadata) {
        m_coverPath = m_covers.resolve(m_state.track);
        m_coverTrack = m_state.track.filePath;
        m_coverHadMetadata = m_state.metadataReady;
    }
}

void NowPlayingScreen::render(Canvas& canvas) {
    const SkinColors& colors = m_skin.colors;
    const SkinLayout& layout = m_skin.layout;
    const Playlist& playlist = m_controller.playlist();

    canvas.clear(colors.background);
    if (!m_skin.wallpaper.empty()) {
        canvas.drawImage(m_skin.wallpaper, Rect{0, 0, SCREEN_WIDTH, SCREEN_HEIGHT});
    }

    // Status line: state on the left, volume centred, track position on the right.
    canvas.drawText(statusLabel(m_state.status), layout.status, colors.accent, FontSize::Small);
    canvas.drawText("VOL " + std::to_string(m_state.volume), layout.status, colors.textDim,
                    FontSize::Small, Align::Center);
    if (!playlist.empty()) {
        canvas.drawText(std::to_string(playlist.index() + 1) + "/" + std::to_string(playlist.size()),
                        layout.status, colors.textDim, FontSize::Small, Align::Right);
    }

    if (m_coverPath.empty() || !canvas.drawImage(m_coverPath, layout.cover)) {
        canvas.fillRect(layout.cover, colors.progressTrack);
        canvas.drawText("NO COVER", layout.cover, colors.textDim, FontSize::Small, Align::Center);
    }
    canvas.drawRect(layout.cover, colors.accent);

    if (playlist.empty()) {
        canvas.drawText("No music found", layout.title, colors.text, FontSize::Large, Align::Center);
        canvas.drawText("Use --music <dir>", layout.artist, colors.textDim, FontSize::Small, Align::Center);
        return;
    }
    if (m_state.track.filePath.empty()) {
        canvas.drawText("Press SPACE to play", layout.title, colors.text, FontSize::Large, Align::Center);
        return;
    }

    const TrackInfo& track = m_state.track;
    canvas.drawText(track.title, layout.title, colors.text, FontSize::Large, Align::Center);
    canvas.drawText(track.artist.empty() ? "Unknown artist" : track.artist,
                    layout.artist, colors.textDim, FontSize::Small, Align::Center);
    canvas.drawText(track.album, layout.album, colors.textDim, FontSize::Small, Align::Center);

    canvas.fillRect(layout.progressBar, colors.progressTrack);
    if (track.durationMs > 0) {
        Rect filled = layout.progressBar;
        filled.w = static_cast<int>(static_cast<long long>(layout.progressBar.w) * m_state.positionMs / track.durationMs);
        if (filled.w > layout.progressBar.w) filled.w = layout.progressBar.w;
        canvas.fillRect(filled, colors.accent);
    }

    canvas.drawText(formatTime(m_state.positionMs), layout.time, colors.text, FontSize::Small);
    canvas.drawText(formatTime(track.durationMs), layout.time, colors.text, FontSize::Small, Align::Right);
}
