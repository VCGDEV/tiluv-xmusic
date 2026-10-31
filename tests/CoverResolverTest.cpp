// tests/CoverResolverTest.cpp
#include <gtest/gtest.h>
#include "CoverResolver.hpp"
#include "TempDir.hpp"

namespace {

TrackInfo track(const std::string& artist, const std::string& album, const std::string& filePath = "") {
    TrackInfo info;
    info.artist = artist;
    info.album = album;
    info.filePath = filePath;
    return info;
}

} // namespace

TEST(CoverResolver, ArtistAndAlbumRuleBeatsSingleFieldRuleWhateverTheOrder) {
    SkinConfig skin;
    skin.coverRules = {{"Caifanes", "", "/covers/artist.jpg"},
                       {"", "El Silencio", "/covers/album-only.jpg"},
                       {"Caifanes", "El Silencio", "/covers/exact.jpg"}};
    CoverResolver resolver(skin);

    EXPECT_EQ(resolver.resolve(track("Caifanes", "El Silencio")), "/covers/exact.jpg");
    EXPECT_EQ(resolver.resolve(track("Caifanes", "Volumen II")), "/covers/artist.jpg");
}

TEST(CoverResolver, RulesIgnoreAsciiCase) {
    SkinConfig skin;
    skin.coverRules = {{"Café Tacvba", "Re", "/covers/re.jpg"}};
    CoverResolver resolver(skin);

    EXPECT_EQ(resolver.resolve(track("CAFé TACVBA", "re")), "/covers/re.jpg");
}

TEST(CoverResolver, RuleWithAnyMismatchedFieldIsSkipped) {
    SkinConfig skin;
    skin.coverRules = {{"Caifanes", "El Silencio", "/covers/exact.jpg"}};
    skin.defaultCover = "/covers/default.png";
    CoverResolver resolver(skin);

    EXPECT_EQ(resolver.resolve(track("Caifanes", "Volumen II")), "/covers/default.png");
}

TEST(CoverResolver, RuleBeatsEmbeddedArtwork) {
    TempDir dir;
    SkinConfig skin;
    skin.coverRules = {{"Caifanes", "", "/covers/artist.jpg"}};
    CoverResolver resolver(skin);

    TrackInfo info = track("Caifanes", "");
    info.artworkPath = dir.touch("embedded.jpg");
    EXPECT_EQ(resolver.resolve(info), "/covers/artist.jpg");
}

TEST(CoverResolver, EmbeddedArtworkIsUsedOnlyIfTheFileExists) {
    TempDir dir;
    SkinConfig skin;
    skin.defaultCover = "/covers/default.png";
    CoverResolver resolver(skin);

    TrackInfo info = track("", "", dir.touch("album/song.mp3"));
    info.artworkPath = dir.touch("embedded.jpg");
    EXPECT_EQ(resolver.resolve(info), info.artworkPath);

    info.artworkPath = (dir.path() / "deleted.jpg").string();
    EXPECT_EQ(resolver.resolve(info), "/covers/default.png");
}

TEST(CoverResolver, FolderImagesFollowNamePriority) {
    TempDir dir;
    SkinConfig skin;
    CoverResolver resolver(skin);
    const TrackInfo info = track("", "", dir.touch("album/song.mp3"));

    const std::string folderPng = dir.touch("album/folder.png");
    EXPECT_EQ(resolver.resolve(info), folderPng);
    const std::string folderJpg = dir.touch("album/folder.jpg");
    EXPECT_EQ(resolver.resolve(info), folderJpg);
    const std::string coverJpg = dir.touch("album/cover.jpg");
    EXPECT_EQ(resolver.resolve(info), coverJpg);
}

TEST(CoverResolver, EmptyWhenNothingApplies) {
    SkinConfig skin;
    CoverResolver resolver(skin);
    EXPECT_EQ(resolver.resolve(track("Unknown", "Unknown")), "");
}
