// src/Input.cpp
#include "Input.hpp"

InputAction mapKey(SDL_Keycode key) {
    switch (key) {
        case SDLK_UP:        return InputAction::Up;
        case SDLK_DOWN:      return InputAction::Down;
        case SDLK_LEFT:      return InputAction::Left;
        case SDLK_RIGHT:     return InputAction::Right;
        case SDLK_RETURN:    return InputAction::Select;
        case SDLK_BACKSPACE: return InputAction::Back;
        case SDLK_SPACE:     return InputAction::PlayPause;
        case SDLK_s:         return InputAction::Stop;
        case SDLK_PERIOD:    return InputAction::Next;
        case SDLK_COMMA:     return InputAction::Previous;
        case SDLK_PLUS:
        case SDLK_EQUALS:
        case SDLK_KP_PLUS:   return InputAction::VolumeUp;
        case SDLK_MINUS:
        case SDLK_KP_MINUS:  return InputAction::VolumeDown;
        case SDLK_ESCAPE:
        case SDLK_q:         return InputAction::Quit;
        default:             return InputAction::None;
    }
}
