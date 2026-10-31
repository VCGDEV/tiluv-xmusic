// tests/PlaylistTest.cpp
#include <gtest/gtest.h>
#include "Playlist.hpp"
#include "TempDir.hpp"

TEST(Playlist, LoadsAudioFilesRecursivelySortedByPath) {
    TempDir dir;
    const std::string b = dir.touch("b.mp3");
    const std::string a = dir.touch("a.flac");
    const std::string nested = dir.touch("album/01.ogg");

    Playlist playlist;
    EXPECT_EQ(playlist.loadDirectory(dir.path().string()), 3u);
    EXPECT_EQ(playlist.current(), a);
    EXPECT_EQ(playlist.next(), nested);
    EXPECT_EQ(playlist.next(), b);
}

TEST(Playlist, MatchesExtensionsIgnoringCaseAndSkipsOtherFiles) {
    TempDir dir;
    dir.touch("loud.MP3");
    dir.touch("song.Wav");
    dir.touch("cover.jpg");
    dir.touch("notes.txt");
    dir.touch("mp3"); // no extension

    Playlist playlist;
    EXPECT_EQ(playlist.loadDirectory(dir.path().string()), 2u);
}

TEST(Playlist, MissingDirectoryGivesEmptyPlaylist) {
    Playlist playlist;
    EXPECT_EQ(playlist.loadDirectory("/nonexistent/tiluv-music"), 0u);
    EXPECT_TRUE(playlist.empty());
}

TEST(Playlist, NextAndPreviousWrapAround) {
    TempDir dir;
    const std::string first = dir.touch("1.mp3");
    dir.touch("2.mp3");
    const std::string last = dir.touch("3.mp3");

    Playlist playlist;
    playlist.loadDirectory(dir.path().string());
    EXPECT_EQ(playlist.previous(), last);
    EXPECT_EQ(playlist.index(), 2u);
    EXPECT_EQ(playlist.next(), first);
    EXPECT_EQ(playlist.index(), 0u);
}

TEST(Playlist, ReloadingResetsTheCursor) {
    TempDir dir;
    const std::string first = dir.touch("1.mp3");
    dir.touch("2.mp3");

    Playlist playlist;
    playlist.loadDirectory(dir.path().string());
    playlist.next();
    playlist.loadDirectory(dir.path().string());
    EXPECT_EQ(playlist.index(), 0u);
    EXPECT_EQ(playlist.current(), first);
}
