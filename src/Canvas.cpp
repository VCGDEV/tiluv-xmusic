// src/Canvas.cpp
#include "Canvas.hpp"
#include <iostream>
#ifdef TALUV_HAS_SDL_IMAGE
#include <SDL2/SDL_image.h>
#endif

namespace {

// Tried in order when the skin has no usable font. Fedora paths first, then Debian/Armbian
// (the Orange Pi images).
const char* const kFallbackFonts[] = {
    "/usr/share/fonts/dejavu-sans-mono-fonts/DejaVuSansMono.ttf",
    "/usr/share/fonts/liberation-mono-fonts/LiberationMono-Regular.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
};

constexpr size_t kMaxCachedTexts = 128;

SDL_Rect toSDL(const Rect& r) {
    return SDL_Rect{r.x, r.y, r.w, r.h};
}

TTF_Font* openFont(const std::string& skinFont, int size) {
    if (!skinFont.empty()) {
        if (TTF_Font* font = TTF_OpenFont(skinFont.c_str(), size)) return font;
        std::cerr << "[Canvas] Warning: " << TTF_GetError() << std::endl;
    }
    for (const char* path : kFallbackFonts) {
        if (TTF_Font* font = TTF_OpenFont(path, size)) return font;
    }
    return nullptr;
}

} // namespace

Canvas::Canvas(SDL_Renderer* renderer, const SkinConfig& skin)
    : m_renderer(renderer), m_skin(skin) {}

Canvas::~Canvas() {
    clearTextCache();
    for (auto& entry : m_images) {
        if (entry.second) SDL_DestroyTexture(entry.second);
    }
    if (m_fontLarge) TTF_CloseFont(m_fontLarge);
    if (m_fontSmall) TTF_CloseFont(m_fontSmall);
}

bool Canvas::loadFonts() {
    m_fontLarge = openFont(m_skin.fontFile, m_skin.fontSize);
    m_fontSmall = openFont(m_skin.fontFile, m_skin.fontSizeSmall);
    if (!m_fontLarge || !m_fontSmall) {
        std::cerr << "[Canvas] Error: no usable font (skin font missing and no system fallback)." << std::endl;
        return false;
    }
    return true;
}

void Canvas::clear(Color color) {
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(m_renderer);
}

void Canvas::fillRect(const Rect& rect, Color color) {
    const SDL_Rect r = toSDL(rect);
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(m_renderer, &r);
}

void Canvas::drawRect(const Rect& rect, Color color) {
    const SDL_Rect r = toSDL(rect);
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(m_renderer, &r);
}

SDL_Texture* Canvas::image(const std::string& path) {
    auto it = m_images.find(path);
    if (it != m_images.end()) return it->second;

#ifdef TALUV_HAS_SDL_IMAGE
    SDL_Surface* surface = IMG_Load(path.c_str());
#else
    SDL_Surface* surface = SDL_LoadBMP(path.c_str()); // PNG/JPG need SDL2_image
#endif
    SDL_Texture* texture = nullptr;
    if (surface) {
        texture = SDL_CreateTextureFromSurface(m_renderer, surface);
        SDL_FreeSurface(surface);
        // Covers are downscaled from large images: smooth filtering looks much better.
        if (texture) SDL_SetTextureScaleMode(texture, SDL_ScaleModeLinear);
    }
    if (!texture) {
        std::cerr << "[Canvas] Warning: could not load image " << path << ": " << SDL_GetError() << std::endl;
    }
    m_images[path] = texture; // cache failures too, so we don't retry every frame
    return texture;
}

bool Canvas::drawImage(const std::string& path, const Rect& dst) {
    SDL_Texture* texture = image(path);
    if (!texture) return false;
    const SDL_Rect r = toSDL(dst);
    SDL_RenderCopy(m_renderer, texture, nullptr, &r);
    return true;
}

void Canvas::clearTextCache() {
    for (auto& entry : m_textCache) {
        SDL_DestroyTexture(entry.second.texture);
    }
    m_textCache.clear();
}

const Canvas::TextTexture* Canvas::text(const std::string& value, Color color, FontSize size) {
    const std::string key = std::to_string(static_cast<int>(size)) + ':' +
                            std::to_string((color.r << 24) | (color.g << 16) | (color.b << 8) | color.a) +
                            ':' + value;
    auto it = m_textCache.find(key);
    if (it != m_textCache.end()) return &it->second;

    // Track titles and the clock change constantly; a full flush is simpler than LRU here.
    if (m_textCache.size() >= kMaxCachedTexts) clearTextCache();

    TTF_Font* font = size == FontSize::Large ? m_fontLarge : m_fontSmall;
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, value.c_str(), SDL_Color{color.r, color.g, color.b, color.a});
    if (!surface) return nullptr;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
    TextTexture entry{texture, surface->w, surface->h};
    SDL_FreeSurface(surface);
    if (!texture) return nullptr;

    return &m_textCache.emplace(key, entry).first->second;
}

void Canvas::drawText(const std::string& value, const Rect& box, Color color, FontSize size, Align align) {
    if (value.empty()) return;
    const TextTexture* rendered = text(value, color, size);
    if (!rendered) return;

    int x = box.x;
    if (align == Align::Center) x = box.x + (box.w - rendered->width) / 2;
    if (align == Align::Right) x = box.x + box.w - rendered->width;
    // Left-align text that doesn't fit so the beginning stays readable.
    if (rendered->width > box.w) x = box.x;
    const int y = box.y + (box.h - rendered->height) / 2;

    const SDL_Rect clip = toSDL(box);
    const SDL_Rect dst{x, y, rendered->width, rendered->height};
    SDL_RenderSetClipRect(m_renderer, &clip);
    SDL_RenderCopy(m_renderer, rendered->texture, nullptr, &dst);
    SDL_RenderSetClipRect(m_renderer, nullptr);
}
