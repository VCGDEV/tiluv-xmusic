// src/UIApp.cpp
#include "UIApp.hpp"
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <utility>
#include "NowPlayingScreen.hpp"

namespace {
// A music UI doesn't need 60 FPS; 30 halves CPU wake-ups, which matters on battery.
constexpr Uint32 kFrameMs = 1000 / 30;
}

UIApp::UIApp(PlaybackController& controller, SkinConfig skin, int scale)
    : m_controller(controller), m_skin(std::move(skin)), m_scale(scale) {}

UIApp::~UIApp() {
    // Textures and fonts must go before the renderer and TTF shut down.
    m_screen.reset();
    m_canvas.reset();
    if (m_renderer) SDL_DestroyRenderer(m_renderer);
    if (m_window) SDL_DestroyWindow(m_window);
    if (m_ttfStarted) TTF_Quit();
    if (m_sdlStarted) SDL_Quit();
}

bool UIApp::init() {
    // Audio is LibVLC's job; SDL only handles video and input.
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "[UIApp] SDL_Init Error: " << SDL_GetError() << std::endl;
        return false;
    }
    m_sdlStarted = true;

    if (TTF_Init() < 0) {
        std::cerr << "[UIApp] TTF_Init Error: " << TTF_GetError() << std::endl;
        return false;
    }
    m_ttfStarted = true;

    m_window = SDL_CreateWindow(
        "Taluv-xmusic Emulator (240x320)",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH * m_scale, SCREEN_HEIGHT * m_scale,
        SDL_WINDOW_SHOWN
    );
    if (!m_window) {
        std::cerr << "[UIApp] SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        return false;
    }

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    if (!m_renderer) {
        std::cerr << "[UIApp] SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        return false;
    }
    // Screens always draw in 240x320 coordinates; SDL scales up to the window.
    SDL_RenderSetLogicalSize(m_renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    m_canvas = std::make_unique<Canvas>(m_renderer, m_skin);
    if (!m_canvas->loadFonts()) return false;

    m_screen = std::make_unique<NowPlayingScreen>(m_controller, m_skin);
    m_isRunning = true;
    return true;
}

void UIApp::run() {
    while (m_isRunning) {
        const Uint32 frameStart = SDL_GetTicks();

        handleEvents();
        m_controller.update();
        m_screen->update();
        m_screen->render(*m_canvas);
        SDL_RenderPresent(m_renderer);

        const Uint32 elapsed = SDL_GetTicks() - frameStart;
        if (elapsed < kFrameMs) SDL_Delay(kFrameMs - elapsed);
    }
}

void UIApp::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            m_isRunning = false;
        } else if (event.type == SDL_KEYDOWN) {
            const InputAction action = mapKey(event.key.keysym.sym);
            if (action == InputAction::Quit) {
                m_isRunning = false;
            } else if (action != InputAction::None) {
                m_screen->handleAction(action);
            }
        }
    }
}
