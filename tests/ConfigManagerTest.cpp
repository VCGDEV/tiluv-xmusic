// tests/ConfigManagerTest.cpp
#include <gtest/gtest.h>
#include "ConfigManager.hpp"
#include "TempDir.hpp"

namespace {

void expectColor(const Color& c, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    EXPECT_EQ(c.r, r);
    EXPECT_EQ(c.g, g);
    EXPECT_EQ(c.b, b);
    EXPECT_EQ(c.a, a);
}

SkinConfig loadJson(const TempDir& dir, const std::string& json) {
    return ConfigManager::loadSkin(dir.touch("skin.json", json), dir.path().string());
}

} // namespace

TEST(ConfigManager, MissingFileGivesBuiltInSkin) {
    const SkinConfig skin = ConfigManager::loadSkin("/nonexistent/skin.json", "/nonexistent");
    EXPECT_EQ(skin.name, "Built-in");
    EXPECT_EQ(skin.fontSize, 16);
}

TEST(ConfigManager, InvalidJsonGivesBuiltInSkin) {
    TempDir dir;
    EXPECT_EQ(loadJson(dir, "{ \"name\": ").name, "Built-in");
}

TEST(ConfigManager, ParsesColorsAndLayout) {
    TempDir dir;
    const SkinConfig skin = loadJson(dir, R"({
        "name": "Test",
        "font": { "size": 20, "size_small": 10 },
        "colors": { "background": "#102030", "accent": "#A0B0C080" },
        "layout": { "cover": [1, 2, 3, 4] }
    })");

    EXPECT_EQ(skin.name, "Test");
    EXPECT_EQ(skin.fontSize, 20);
    EXPECT_EQ(skin.fontSizeSmall, 10);
    expectColor(skin.colors.background, 0x10, 0x20, 0x30, 0xFF);
    expectColor(skin.colors.accent, 0xA0, 0xB0, 0xC0, 0x80);
    expectColor(skin.colors.text, 0xE8, 0xE8, 0xE8, 0xFF); // untouched default
    EXPECT_EQ(skin.layout.cover.x, 1);
    EXPECT_EQ(skin.layout.cover.h, 4);
}

TEST(ConfigManager, InvalidColorsKeepDefaults) {
    TempDir dir;
    const SkinConfig skin = loadJson(dir, R"({
        "colors": { "background": "#12ZZ56", "text": "#0x1234", "text_dim": "123456",
                    "accent": "#FFF", "progress_track": 42 }
    })");

    const SkinColors defaults;
    expectColor(skin.colors.background, defaults.background.r, defaults.background.g, defaults.background.b, 255);
    expectColor(skin.colors.text, defaults.text.r, defaults.text.g, defaults.text.b, 255);
    expectColor(skin.colors.textDim, defaults.textDim.r, defaults.textDim.g, defaults.textDim.b, 255);
    expectColor(skin.colors.accent, defaults.accent.r, defaults.accent.g, defaults.accent.b, 255);
    expectColor(skin.colors.progressTrack, defaults.progressTrack.r, defaults.progressTrack.g, defaults.progressTrack.b,
                255);
}

TEST(ConfigManager, MalformedRectKeepsDefault) {
    TempDir dir;
    const SkinConfig skin = loadJson(dir, R"({ "layout": { "cover": [1, 2, 3] } })");
    EXPECT_EQ(skin.layout.cover.x, SkinLayout{}.cover.x);
    EXPECT_EQ(skin.layout.cover.w, SkinLayout{}.cover.w);
}

TEST(ConfigManager, WrongValueTypeStopsParsingButKeepsEarlierKeys) {
    TempDir dir;
    const SkinConfig skin = loadJson(dir, R"({ "name": "Typo", "font": { "size": "big" } })");
    EXPECT_EQ(skin.name, "Typo");
    EXPECT_EQ(skin.fontSize, 16);
}

TEST(ConfigManager, ResolvesRelativeAssetsAndDropsMissingOnes) {
    TempDir dir;
    const std::string wallpaper = dir.touch("images/bg.png");
    const SkinConfig skin = loadJson(dir, R"({
        "wallpaper": "images/bg.png",
        "font": { "file": "fonts/missing.ttf" }
    })");

    EXPECT_EQ(skin.wallpaper, wallpaper);
    EXPECT_EQ(skin.fontFile, "");
}

TEST(ConfigManager, KeepsOnlyUsableCoverRules) {
    TempDir dir;
    const std::string image = dir.touch("covers/re.jpg");
    const SkinConfig skin = loadJson(dir, R"({
        "covers": {
            "rules": [
                { "artist": "Café Tacvba", "album": "Re", "image": "covers/re.jpg" },
                { "image": "covers/re.jpg" },
                { "artist": "Caifanes", "image": "covers/missing.jpg" }
            ]
        }
    })");

    ASSERT_EQ(skin.coverRules.size(), 1u);
    EXPECT_EQ(skin.coverRules[0].artist, "Café Tacvba");
    EXPECT_EQ(skin.coverRules[0].image, image);
}

// Guards the files shipped in assets/: the default skin must find all of them.
TEST(ConfigManager, DefaultSkinFindsAllShippedAssets) {
    const std::string root = TILUV_ASSET_DIR;
    const SkinConfig skin = ConfigManager::loadSkin(root + "/config/skin_default.json", root);

    EXPECT_EQ(skin.name, "XpressMusic Classic");
    EXPECT_FALSE(skin.fontFile.empty());
    EXPECT_FALSE(skin.wallpaper.empty());
    EXPECT_FALSE(skin.defaultCover.empty());
}
