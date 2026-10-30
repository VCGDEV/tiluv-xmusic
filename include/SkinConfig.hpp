// include/SkinConfig.hpp
// Plain data describing a skin. Loaded from JSON by ConfigManager; defaults below are the
// built-in skin used when the file or a key is missing. No SDL types, so config stays
// independent of the renderer.
#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct Color {
    uint8_t r = 0, g = 0, b = 0, a = 255;
};

struct Rect {
    int x = 0, y = 0, w = 0, h = 0;
};

struct SkinColors {
    Color background{0x11, 0x11, 0x11};
    Color text{0xE8, 0xE8, 0xE8};
    Color textDim{0x8A, 0x8A, 0x8A};
    Color accent{0x00, 0xFF, 0x00};
    Color progressTrack{0x33, 0x33, 0x33};
};

// Positions in screen pixels (240x320 portrait).
struct SkinLayout {
    Rect status{6, 2, 228, 16};
    Rect cover{40, 26, 160, 160};
    Rect title{8, 194, 224, 22};
    Rect artist{8, 218, 224, 18};
    Rect album{8, 236, 224, 18};
    Rect progressBar{12, 266, 216, 6};
    Rect time{8, 278, 224, 16};
};

// Custom cover override. Empty fields act as wildcards; the most specific match wins.
struct CoverRule {
    std::string artist;
    std::string album;
    std::string image; // absolute path after loading
};

struct SkinConfig {
    std::string name = "Built-in";
    std::string fontFile;  // absolute path, empty = system fallback font
    int fontSize = 16;
    int fontSizeSmall = 12;
    std::string wallpaper;    // absolute path, empty = solid background colour
    std::string defaultCover; // absolute path, empty = drawn placeholder
    SkinColors colors;
    SkinLayout layout;
    std::vector<CoverRule> coverRules;
};
