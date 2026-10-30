// src/ConfigManager.cpp
#include "ConfigManager.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using nlohmann::json;

namespace {

// Accepts "#RRGGBB" or "#RRGGBBAA"; keeps `fallback` on anything else.
Color parseColor(const json& node, const char* key, Color fallback) {
    if (!node.contains(key) || !node[key].is_string()) return fallback;
    const std::string hex = node[key].get<std::string>();
    if (hex.size() != 7 && hex.size() != 9) return fallback;
    if (hex[0] != '#') return fallback;
    try {
        const unsigned long value = std::stoul(hex.substr(1), nullptr, 16);
        if (hex.size() == 7) {
            return Color{uint8_t(value >> 16), uint8_t(value >> 8), uint8_t(value), 255};
        }
        return Color{uint8_t(value >> 24), uint8_t(value >> 16), uint8_t(value >> 8), uint8_t(value)};
    } catch (const std::exception&) {
        std::cerr << "[ConfigManager] Warning: invalid colour '" << hex << "' for " << key << std::endl;
        return fallback;
    }
}

// Rects are written as [x, y, w, h].
Rect parseRect(const json& node, const char* key, Rect fallback) {
    if (!node.contains(key)) return fallback;
    const json& r = node[key];
    if (!r.is_array() || r.size() != 4) {
        std::cerr << "[ConfigManager] Warning: layout." << key << " must be [x, y, w, h]" << std::endl;
        return fallback;
    }
    return Rect{r[0].get<int>(), r[1].get<int>(), r[2].get<int>(), r[3].get<int>()};
}

// Resolves a path from the skin against the asset root; returns empty if it doesn't exist.
std::string resolveAsset(const std::string& value, const fs::path& assetRoot, const char* what) {
    if (value.empty()) return {};
    fs::path path(value);
    if (path.is_relative()) path = assetRoot / path;
    if (!fs::exists(path)) {
        std::cerr << "[ConfigManager] Warning: " << what << " not found: " << path.string() << std::endl;
        return {};
    }
    return path.string();
}

} // namespace

SkinConfig ConfigManager::loadSkin(const std::string& jsonPath, const std::string& assetRoot) {
    SkinConfig skin;

    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        std::cerr << "[ConfigManager] Warning: could not open skin " << jsonPath
                  << ", using built-in skin." << std::endl;
        return skin;
    }

    json data;
    try {
        file >> data;
    } catch (const json::parse_error& e) {
        std::cerr << "[ConfigManager] JSON parse error in " << jsonPath << ": " << e.what() << std::endl;
        return skin;
    }

    try {
        const fs::path root(assetRoot);
        skin.name = data.value("name", skin.name);
        skin.wallpaper = resolveAsset(data.value("wallpaper", ""), root, "wallpaper");

        if (data.contains("font")) {
            const json& font = data["font"];
            skin.fontFile = resolveAsset(font.value("file", ""), root, "font");
            skin.fontSize = font.value("size", skin.fontSize);
            skin.fontSizeSmall = font.value("size_small", skin.fontSizeSmall);
        }

        if (data.contains("colors")) {
            const json& c = data["colors"];
            skin.colors.background = parseColor(c, "background", skin.colors.background);
            skin.colors.text = parseColor(c, "text", skin.colors.text);
            skin.colors.textDim = parseColor(c, "text_dim", skin.colors.textDim);
            skin.colors.accent = parseColor(c, "accent", skin.colors.accent);
            skin.colors.progressTrack = parseColor(c, "progress_track", skin.colors.progressTrack);
        }

        if (data.contains("layout")) {
            const json& l = data["layout"];
            skin.layout.status = parseRect(l, "status", skin.layout.status);
            skin.layout.cover = parseRect(l, "cover", skin.layout.cover);
            skin.layout.title = parseRect(l, "title", skin.layout.title);
            skin.layout.artist = parseRect(l, "artist", skin.layout.artist);
            skin.layout.album = parseRect(l, "album", skin.layout.album);
            skin.layout.progressBar = parseRect(l, "progress_bar", skin.layout.progressBar);
            skin.layout.time = parseRect(l, "time", skin.layout.time);
        }

        if (data.contains("covers")) {
            const json& covers = data["covers"];
            skin.defaultCover = resolveAsset(covers.value("default", ""), root, "default cover");
            for (const json& rule : covers.value("rules", json::array())) {
                CoverRule parsed{rule.value("artist", ""), rule.value("album", ""),
                                 resolveAsset(rule.value("image", ""), root, "cover image")};
                if (parsed.image.empty() || (parsed.artist.empty() && parsed.album.empty())) continue;
                skin.coverRules.push_back(std::move(parsed));
            }
        }
    } catch (const json::exception& e) {
        // Wrong value types (e.g. a string where a number is expected).
        std::cerr << "[ConfigManager] Invalid value in " << jsonPath << ": " << e.what()
                  << ". Remaining keys use defaults." << std::endl;
    }

    std::cout << "[ConfigManager] Loaded skin '" << skin.name << "' with "
              << skin.coverRules.size() << " cover rule(s)." << std::endl;
    return skin;
}
