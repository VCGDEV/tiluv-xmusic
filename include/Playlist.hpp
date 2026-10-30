// include/Playlist.hpp
#pragma once
#include <cstddef>
#include <string>
#include <vector>

// Ordered list of audio files with a wrap-around cursor. Pure C++, no backend dependency.
class Playlist {
private:
    std::vector<std::string> m_tracks;
    size_t m_index = 0;

public:
    // Recursively collects supported audio files under `directory`, sorted by path.
    size_t loadDirectory(const std::string& directory);

    bool empty() const { return m_tracks.empty(); }
    size_t size() const { return m_tracks.size(); }
    size_t index() const { return m_index; }

    const std::string& current() const; // precondition: !empty()
    const std::string& next();
    const std::string& previous();
};
