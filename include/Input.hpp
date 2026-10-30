// include/Input.hpp
#pragma once
#include <SDL2/SDL.h>

// Logical phone buttons. Screens react to these, never to raw keycodes, so the same code
// works with the laptop keyboard and with GPIO buttons on the Orange Pi (exposed as a
// keyboard through the gpio-keys device-tree overlay).
enum class InputAction {
    None,
    Up, Down, Left, Right, Select, Back,           // navigation pad
    PlayPause, Stop, Next, Previous,               // dedicated XpressMusic media keys
    VolumeUp, VolumeDown,
    Quit,                                          // emulator only
};

InputAction mapKey(SDL_Keycode key);
