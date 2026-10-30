// include/UIApp.hpp
#pragma once
#include <SDL2/SDL.h>
#include <memory>
#include "Canvas.hpp"
#include "IScreen.hpp"
#include "PlaybackController.hpp"
#include "SkinConfig.hpp"

// Owns the SDL window, the frame loop and the active screen. Contains no music logic:
// everything player-related goes through PlaybackController.
class UIApp {
private:
    PlaybackController& m_controller;
    SkinConfig m_skin;
    int m_scale; // window magnification for the laptop emulator; 1 on the real screen

    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    std::unique_ptr<Canvas> m_canvas;
    std::unique_ptr<IScreen> m_screen;
    bool m_sdlStarted = false;
    bool m_ttfStarted = false;
    bool m_isRunning = false;

    void handleEvents();

public:
    UIApp(PlaybackController& controller, SkinConfig skin, int scale);
    ~UIApp();
    UIApp(const UIApp&) = delete;
    UIApp& operator=(const UIApp&) = delete;

    bool init();
    void run();
};
