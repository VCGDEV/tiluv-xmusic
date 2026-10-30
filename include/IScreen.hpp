// include/IScreen.hpp
#pragma once
#include "Canvas.hpp"
#include "Input.hpp"

// One full-screen view (now playing, library, settings, and later Telegram chats).
// UIApp owns the active screen and forwards input, update and render to it.
class IScreen {
public:
    virtual ~IScreen() = default;
    virtual void handleAction(InputAction action) = 0;
    virtual void update() = 0;
    virtual void render(Canvas& canvas) = 0;
};
