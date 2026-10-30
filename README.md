# Taluv-xmusic

Open-source modular slider phone in the spirit of the Nokia XpressMusic.
Target: Orange Pi Zero 2W + ILI9341 2.4" SPI TFT (240x320) + MAX98357 I2S amp.
Development happens on a Fedora laptop, where the app runs in an emulator window.

## Build & run (Fedora)

```bash
sudo dnf install gcc-c++ cmake SDL2-devel SDL2_ttf-devel SDL2_image-devel json-devel \
                 vlc-devel vlc-plugins-base vlc-plugin-ffmpeg vlc-plugin-pulseaudio
mkdir -p build && cd build && cmake .. && make
./taluv_xmusic --music ~/Music --scale 2
```

| Key | Action |
|---|---|
| Space / Enter | Play / pause |
| ← → or , . | Previous / next track |
| ↑ ↓ or + - | Volume |
| S | Stop |
| Esc / Q | Quit |

## Architecture

```
            UI layer (SDL2)                     Logic                 Backend
 ┌───────────────────────────────────┐   ┌────────────────────┐   ┌────────────────────┐
 │ UIApp ── IScreen ── NowPlaying    │──▶│ PlaybackController │──▶│ IPlayerController  │
 │   │        │           │          │   │   + Playlist       │   │   └─ VLCPlayer     │
 │ Input    Canvas    CoverResolver  │   └────────────────────┘   └────────────────────┘
 └───────────────────────────────────┘
             ▲
   ConfigManager ── SkinConfig  (JSON → plain structs)
```

- **`IPlayerController`**: audio backend contract plus a `PlayerState` snapshot. No VLC or SDL types.
- **`VLCPlayer`**: the LibVLC implementation. Tags and embedded cover art are parsed asynchronously and picked up in `getState()`, so VLC never calls back into our threads.
- **`PlaybackController` + `Playlist`**: music behaviour (next/previous, auto-advance, volume steps). Pure C++, so it can be tested with a fake player.
- **`UIApp`**: window, 30 FPS frame loop, owns the active `IScreen`. Draws in 240x320 logical pixels and scales the window up on the laptop.
- **`Input`**: maps keys to logical `InputAction`s. On the Orange Pi, wire the buttons with the `gpio-keys` overlay so they arrive as key events and no code changes.
- **`Canvas`**: drawing helpers, fonts and texture caches.
- **`ConfigManager` → `SkinConfig`**: loads the skin JSON with a default for every key.
- **`CoverResolver`**: chooses which cover art to show.

Phase 2 (Telegram/WhatsApp) adds new `IScreen`s and services beside `PlaybackController`; the existing layers don't change.

## Skins (`assets/config/skin_default.json`)

Relative paths are resolved against `assets/`. Every key is optional; missing or invalid values fall back to built-in defaults with a warning.

```jsonc
{
  "name": "XpressMusic Classic",
  "wallpaper": "images/bg_retro.png",
  "font": { "file": "fonts/PxPlus_IBM_VGA8.ttf", "size": 16, "size_small": 12 },
  "colors": { "background": "#111111", "text": "#E8E8E8", "text_dim": "#8A8A8A",
              "accent": "#00FF00", "progress_track": "#333333" },   // #RRGGBB or #RRGGBBAA
  "layout": { "cover": [40, 26, 160, 160], ... },                    // [x, y, w, h]
  "covers": {
    "default": "images/cover_default.png",
    "rules": [
      { "artist": "Café Tacvba", "album": "Re", "image": "images/covers/re.jpg" },
      { "artist": "Caifanes", "image": "images/covers/caifanes.jpg" }
    ]
  }
}
```

Cover art priority:
1. The most specific matching `rules` entry (artist + album beats a single field; matching ignores ASCII case).
2. Artwork embedded in the file (ID3).
3. `cover.jpg`, `folder.jpg`, `cover.png` or `folder.png` in the track's folder.
4. `covers.default`.
5. A drawn placeholder.
