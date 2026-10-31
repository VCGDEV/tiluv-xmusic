// src/CoverResolver.cpp
#include "CoverResolver.hpp"
#include <algorithm>
#include <cctype>
#include <filesystem>

namespace fs = std::filesystem;

namespace {

// ASCII-only case folding: good enough for "café tacvba" vs "Café Tacvba", since the
// accented letters themselves must already match byte-for-byte in UTF-8.
bool equalsIgnoreCase(const std::string& a, const std::string& b) {
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), [](unsigned char x, unsigned char y) {
               return std::tolower(x) == std::tolower(y);
           });
}

// -1 = no match; otherwise number of fields that matched (higher is more specific).
int matchScore(const CoverRule& rule, const TrackInfo& track) {
    int score = 0;
    if (!rule.artist.empty()) {
        if (!equalsIgnoreCase(rule.artist, track.artist)) return -1;
        ++score;
    }
    if (!rule.album.empty()) {
        if (!equalsIgnoreCase(rule.album, track.album)) return -1;
        ++score;
    }
    return score;
}

} // namespace

std::string CoverResolver::resolve(const TrackInfo& track) const {
    const CoverRule* best = nullptr;
    int bestScore = 0;
    for (const CoverRule& rule : m_skin.coverRules) {
        const int score = matchScore(rule, track);
        if (score > bestScore) {
            best = &rule;
            bestScore = score;
        }
    }
    if (best) return best->image;

    std::error_code ec;
    if (!track.artworkPath.empty() && fs::exists(track.artworkPath, ec)) {
        return track.artworkPath;
    }

    if (!track.filePath.empty()) {
        const fs::path dir = fs::path(track.filePath).parent_path();
        for (const char* name : {"cover.jpg", "folder.jpg", "cover.png", "folder.png"}) {
            if (fs::exists(dir / name, ec)) return (dir / name).string();
        }
    }

    return m_skin.defaultCover;
}
