// src/main.cpp
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include "ConfigManager.hpp"
#include "PlaybackController.hpp"
#include "Playlist.hpp"
#include "UIApp.hpp"
#include "VLCPlayer.hpp"

namespace {

struct Options {
    std::string musicDir;
    std::string skinPath = std::string(TALUV_ASSET_DIR) + "/config/skin_default.json";
    int scale = 2;
};

void printUsage(const char* program) {
    std::cout << "Usage: " << program << " [--music DIR] [--skin FILE.json] [--scale N]\n"
              << "  --music  folder scanned recursively for audio files (default: ~/Music)\n"
              << "  --skin   skin JSON (default: assets/config/skin_default.json)\n"
              << "  --scale  emulator window magnification, 1-4 (default: 2)\n";
}

bool parseArgs(int argc, char* argv[], Options& options) {
    if (const char* home = std::getenv("HOME")) options.musicDir = std::string(home) + "/Music";

    for (int i = 1; i < argc; ++i) {
        const bool hasValue = i + 1 < argc;
        if (std::strcmp(argv[i], "--music") == 0 && hasValue) {
            options.musicDir = argv[++i];
        } else if (std::strcmp(argv[i], "--skin") == 0 && hasValue) {
            options.skinPath = argv[++i];
        } else if (std::strcmp(argv[i], "--scale") == 0 && hasValue) {
            options.scale = std::atoi(argv[++i]);
            if (options.scale < 1 || options.scale > 4) options.scale = 2;
        } else {
            printUsage(argv[0]);
            return false;
        }
    }
    return true;
}

} // namespace

int main(int argc, char* argv[]) {
    Options options;
    if (!parseArgs(argc, argv, options)) return 1;

    std::cout << "[Taluv-xmusic] Booting up system..." << std::endl;

    auto player = std::make_unique<VLCPlayer>();
    if (!player->isReady()) {
        std::cerr << "[Taluv-xmusic] Audio backend failed to start. On Fedora install:\n"
                     "  sudo dnf install vlc-plugins-base vlc-plugin-ffmpeg vlc-plugin-pulseaudio" << std::endl;
        return 1;
    }
    player->setVolume(80);

    Playlist playlist;
    const size_t trackCount = playlist.loadDirectory(options.musicDir);
    std::cout << "[Taluv-xmusic] Found " << trackCount << " track(s) in " << options.musicDir << std::endl;

    PlaybackController controller(std::move(player), std::move(playlist));
    UIApp app(controller, ConfigManager::loadSkin(options.skinPath, TALUV_ASSET_DIR), options.scale);

    if (!app.init()) {
        std::cerr << "[Taluv-xmusic] Failed to initialize UI application." << std::endl;
        return 1;
    }

    std::cout << "[Taluv-xmusic] Running emulator window. Controls:\n"
              << " - [SPACE] / [ENTER]  Play / Pause\n"
              << " - [<-] [->] or [,] [.]  Previous / Next track\n"
              << " - [UP] [DOWN] or [+] [-]  Volume\n"
              << " - [S] Stop\n"
              << " - [ESC] / [Q] Quit\n";

    app.run();

    std::cout << "[Taluv-xmusic] Shutting down gracefully." << std::endl;
    return 0;
}
