// src/Playlist.cpp
#include "Playlist.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>

namespace fs = std::filesystem;

namespace {

bool isAudioFile(const fs::path& path) {
    static const std::array<std::string, 6> kExtensions = {".mp3", ".wav", ".flac", ".ogg", ".m4a", ".aac"};
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return std::find(kExtensions.begin(), kExtensions.end(), ext) != kExtensions.end();
}

} // namespace

size_t Playlist::loadDirectory(const std::string& directory) {
    m_tracks.clear();
    m_index = 0;

    std::error_code ec;
    fs::recursive_directory_iterator it(directory, fs::directory_options::skip_permission_denied, ec);
    for (; !ec && it != fs::recursive_directory_iterator(); it.increment(ec)) {
        if (it->is_regular_file(ec) && isAudioFile(it->path())) {
            m_tracks.push_back(it->path().string());
        }
    }
    std::sort(m_tracks.begin(), m_tracks.end());
    return m_tracks.size();
}

const std::string& Playlist::current() const {
    return m_tracks[m_index];
}

const std::string& Playlist::next() {
    m_index = (m_index + 1) % m_tracks.size();
    return current();
}

const std::string& Playlist::previous() {
    m_index = (m_index + m_tracks.size() - 1) % m_tracks.size();
    return current();
}
