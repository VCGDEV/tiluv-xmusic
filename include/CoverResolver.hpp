// include/CoverResolver.hpp
#pragma once
#include <string>
#include "IPlayerController.hpp"
#include "SkinConfig.hpp"

// Decides which image to show as cover art for a track, in priority order:
//   1. a custom cover rule from the skin JSON (artist+album beats album-only or artist-only)
//   2. artwork embedded in the file (ID3 APIC), extracted by the player backend
//   3. cover.jpg / folder.jpg (or .png) next to the audio file
//   4. the skin's default cover
// Returns an empty string when nothing applies, so the screen draws a placeholder.
class CoverResolver {
private:
    const SkinConfig& m_skin;

public:
    explicit CoverResolver(const SkinConfig& skin) : m_skin(skin) {}
    std::string resolve(const TrackInfo& track) const;
};
