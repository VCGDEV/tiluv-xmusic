// include/Canvas.hpp
#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <unordered_map>
#include "SkinConfig.hpp"

enum class FontSize { Large, Small };
enum class Align { Left, Center, Right };

// Thin drawing layer over SDL_Renderer. Owns the fonts and caches image and text textures,
// so screens describe *what* to draw in skin coordinates and never touch SDL directly.
class Canvas {
private:
    struct TextTexture {
        SDL_Texture* texture;
        int width;
        int height;
    };

    SDL_Renderer* m_renderer;
    const SkinConfig& m_skin;
    TTF_Font* m_fontLarge = nullptr;
    TTF_Font* m_fontSmall = nullptr;
    std::unordered_map<std::string, SDL_Texture*> m_images; // nullptr cached for failed loads
    std::unordered_map<std::string, TextTexture> m_textCache;

    SDL_Texture* image(const std::string& path);
    const TextTexture* text(const std::string& value, Color color, FontSize size);
    void clearTextCache();

public:
    Canvas(SDL_Renderer* renderer, const SkinConfig& skin);
    ~Canvas();
    Canvas(const Canvas&) = delete;
    Canvas& operator=(const Canvas&) = delete;

    bool loadFonts(); // skin font first, then common system fonts

    void clear(Color color);
    void fillRect(const Rect& rect, Color color);
    void drawRect(const Rect& rect, Color color);
    bool drawImage(const std::string& path, const Rect& dst); // false if it can't be loaded
    void drawText(const std::string& value, const Rect& box, Color color, FontSize size, Align align = Align::Left);
};
