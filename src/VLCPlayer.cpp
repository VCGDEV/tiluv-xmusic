// src/VLCPlayer.cpp
#include "VLCPlayer.hpp"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>

namespace {

std::string readMeta(libvlc_media_t* media, libvlc_meta_t key) {
    char* value = libvlc_media_get_meta(media, key);
    if (!value) return {};
    std::string result(value);
    libvlc_free(value);
    return result;
}

// VLC reports extracted artwork as a percent-encoded file:// URL.
std::string fileUrlToPath(const std::string& url) {
    const std::string prefix = "file://";
    if (url.rfind(prefix, 0) != 0) return {};

    std::string path;
    for (size_t i = prefix.size(); i < url.size(); ++i) {
        if (url[i] == '%' && i + 2 < url.size() &&
            std::isxdigit(static_cast<unsigned char>(url[i + 1])) &&
            std::isxdigit(static_cast<unsigned char>(url[i + 2]))) {
            path += static_cast<char>(std::stoi(url.substr(i + 1, 2), nullptr, 16));
            i += 2;
        } else {
            path += url[i];
        }
    }
    return path;
}

PlaybackStatus toPlaybackStatus(libvlc_state_t state) {
    switch (state) {
        case libvlc_Opening:
        case libvlc_Buffering: return PlaybackStatus::Opening;
        case libvlc_Playing:   return PlaybackStatus::Playing;
        case libvlc_Paused:    return PlaybackStatus::Paused;
        case libvlc_Stopped:   return PlaybackStatus::Stopped;
        case libvlc_Ended:     return PlaybackStatus::Ended;
        case libvlc_Error:     return PlaybackStatus::Error;
        default:               return PlaybackStatus::Idle;
    }
}

} // namespace

VLCPlayer::VLCPlayer()
    : m_vlcInstance(nullptr), m_mediaPlayer(nullptr), m_media(nullptr),
      m_metadataReady(false), m_volume(80) {
    // Audio-only device: never open a video output window.
    const char* args[] = {"--no-video", "--quiet"};
    m_vlcInstance = libvlc_new(2, args);
    if (!m_vlcInstance) {
        std::cerr << "[VLCPlayer] Error: Could not initialize LibVLC instance "
                     "(are the VLC plugins installed?)." << std::endl;
        return;
    }
    m_mediaPlayer = libvlc_media_player_new(m_vlcInstance);
}

VLCPlayer::~VLCPlayer() {
    if (m_mediaPlayer) {
        libvlc_media_player_stop(m_mediaPlayer);
        libvlc_media_player_release(m_mediaPlayer);
    }
    releaseMedia();
    if (m_vlcInstance) {
        libvlc_release(m_vlcInstance);
    }
}

bool VLCPlayer::isReady() const {
    return m_mediaPlayer != nullptr;
}

void VLCPlayer::releaseMedia() {
    if (m_media) {
        libvlc_media_release(m_media);
        m_media = nullptr;
    }
}

void VLCPlayer::play(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_mediaPlayer) return;

    releaseMedia();
    m_media = libvlc_media_new_path(m_vlcInstance, filepath.c_str());
    if (!m_media) {
        std::cerr << "[VLCPlayer] Error: Could not create media from path: " << filepath << std::endl;
        return;
    }

    m_currentTrack = TrackInfo{};
    m_currentTrack.filePath = filepath;
    m_currentTrack.title = std::filesystem::path(filepath).stem().string();
    m_metadataReady = false;

    // Read ID3 tags and extract embedded cover art into VLC's art cache, without blocking
    // the UI thread. The result is collected in refreshMetadata().
    libvlc_media_parse_with_options(
        m_media,
        static_cast<libvlc_media_parse_flag_t>(libvlc_media_parse_local | libvlc_media_fetch_local),
        3000);

    libvlc_media_player_set_media(m_mediaPlayer, m_media);
    libvlc_media_player_play(m_mediaPlayer);
}

void VLCPlayer::refreshMetadata() const {
    if (m_metadataReady || !m_media) return;
    if (libvlc_media_get_parsed_status(m_media) == 0) return; // still parsing

    // Parsing may have failed or timed out; whatever tags exist are still usable.
    std::string title = readMeta(m_media, libvlc_meta_Title);
    if (!title.empty()) m_currentTrack.title = title;
    m_currentTrack.artist = readMeta(m_media, libvlc_meta_Artist);
    m_currentTrack.album = readMeta(m_media, libvlc_meta_Album);
    m_currentTrack.artworkPath = fileUrlToPath(readMeta(m_media, libvlc_meta_ArtworkURL));
    m_currentTrack.durationMs = static_cast<int>(std::max<libvlc_time_t>(0, libvlc_media_get_duration(m_media)));
    m_metadataReady = true;
}

void VLCPlayer::togglePause() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_mediaPlayer) {
        libvlc_media_player_pause(m_mediaPlayer);
    }
}

void VLCPlayer::stop() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_mediaPlayer) {
        libvlc_media_player_stop(m_mediaPlayer);
    }
}

void VLCPlayer::seek(int positionMs) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_mediaPlayer) {
        libvlc_media_player_set_time(m_mediaPlayer, std::max(0, positionMs));
    }
}

void VLCPlayer::setVolume(int volume) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_volume = std::clamp(volume, 0, 100);
    if (m_mediaPlayer) {
        libvlc_audio_set_volume(m_mediaPlayer, m_volume);
    }
}

PlayerState VLCPlayer::getState() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    PlayerState state;
    state.volume = m_volume;
    if (!m_mediaPlayer) {
        state.status = PlaybackStatus::Error;
        return state;
    }

    refreshMetadata();
    state.status = toPlaybackStatus(libvlc_media_player_get_state(m_mediaPlayer));
    state.track = m_currentTrack;
    state.metadataReady = m_metadataReady;
    state.positionMs = static_cast<int>(std::max<libvlc_time_t>(0, libvlc_media_player_get_time(m_mediaPlayer)));
    if (state.track.durationMs <= 0) {
        // Some files (e.g. VBR MP3 without a header) only report length once playing.
        state.track.durationMs = static_cast<int>(std::max<libvlc_time_t>(0, libvlc_media_player_get_length(m_mediaPlayer)));
    }
    return state;
}
