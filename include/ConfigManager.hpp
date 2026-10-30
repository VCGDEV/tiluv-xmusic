// include/ConfigManager.hpp
#pragma once
#include <string>
#include "SkinConfig.hpp"

class ConfigManager {
public:
    // Loads a skin JSON file. Relative asset paths inside it are resolved against `assetRoot`.
    // Never fails: missing files or keys fall back to SkinConfig defaults with a warning, and
    // referenced images/fonts that don't exist are dropped so the UI uses its fallbacks.
    static SkinConfig loadSkin(const std::string& jsonPath, const std::string& assetRoot);
};
