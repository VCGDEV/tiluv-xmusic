// tests/PlaybackControllerTest.cpp
#include <gtest/gtest.h>
#include <memory>
#include "FakePlayer.hpp"
#include "PlaybackController.hpp"
#include "TempDir.hpp"

namespace {

class PlaybackControllerTest : public ::testing::Test {
protected:
    TempDir dir;
    std::string track1, track2, track3;
    FakePlayer* player = nullptr; // owned by controller
    std::unique_ptr<PlaybackController> controller;

    void SetUp() override {
        track1 = dir.touch("1.mp3");
        track2 = dir.touch("2.mp3");
        track3 = dir.touch("3.mp3");
        start(dir.path().string());
    }

    void start(const std::string& musicDir) {
        Playlist playlist;
        playlist.loadDirectory(musicDir);
        auto fake = std::make_unique<FakePlayer>();
        player = fake.get();
        controller = std::make_unique<PlaybackController>(std::move(fake), std::move(playlist));
    }
};

} // namespace

TEST_F(PlaybackControllerTest, PlayFromIdleStartsTheFirstTrack) {
    controller->togglePlayPause();
    EXPECT_EQ(player->played, std::vector<std::string>{track1});
}

TEST_F(PlaybackControllerTest, PlayPauseWhilePlayingPausesAndResumes) {
    controller->togglePlayPause();
    controller->togglePlayPause();
    EXPECT_EQ(player->state.status, PlaybackStatus::Paused);
    controller->togglePlayPause();
    EXPECT_EQ(player->state.status, PlaybackStatus::Playing);
    EXPECT_EQ(player->togglePauseCalls, 2);
    EXPECT_EQ(player->played.size(), 1u);
}

TEST_F(PlaybackControllerTest, PlayPauseIsIgnoredWhileOpening) {
    player->state.status = PlaybackStatus::Opening;
    controller->togglePlayPause();
    EXPECT_TRUE(player->played.empty());
    EXPECT_EQ(player->togglePauseCalls, 0);
}

TEST_F(PlaybackControllerTest, PlayAfterStopReplaysTheCurrentTrack) {
    controller->next();
    controller->stop();
    EXPECT_EQ(player->stopCalls, 1);
    controller->togglePlayPause();
    EXPECT_EQ(player->played.back(), track2);
}

TEST_F(PlaybackControllerTest, NextAdvancesAndWrapsAround) {
    controller->next();
    controller->next();
    controller->next();
    EXPECT_EQ(player->played, (std::vector<std::string>{track2, track3, track1}));
}

TEST_F(PlaybackControllerTest, PreviousEarlyInTrackGoesToPreviousTrack) {
    controller->togglePlayPause();
    player->state.positionMs = 3000; // threshold is "more than 3 s"
    controller->previous();
    EXPECT_EQ(player->played.back(), track3);
    EXPECT_TRUE(player->seeks.empty());
}

TEST_F(PlaybackControllerTest, PreviousLaterInTrackRestartsIt) {
    controller->togglePlayPause();
    player->state.positionMs = 3001;
    controller->previous();
    EXPECT_EQ(player->seeks, std::vector<int>{0});
    EXPECT_EQ(player->played.size(), 1u);
    EXPECT_EQ(controller->playlist().index(), 0u);
}

TEST_F(PlaybackControllerTest, UpdateAdvancesWhenTrackEnds) {
    controller->togglePlayPause();
    controller->update();
    EXPECT_EQ(player->played.size(), 1u);

    player->state.status = PlaybackStatus::Ended;
    controller->update();
    EXPECT_EQ(player->played.back(), track2);
}

TEST_F(PlaybackControllerTest, ChangeVolumeAppliesDeltaToCurrentVolume) {
    player->state.volume = 50;
    controller->changeVolume(+5);
    EXPECT_EQ(controller->state().volume, 55);
    controller->changeVolume(-100);
    EXPECT_EQ(controller->state().volume, 0);
}

TEST_F(PlaybackControllerTest, EmptyPlaylistDoesNothing) {
    TempDir empty;
    start(empty.path().string());

    controller->togglePlayPause();
    controller->next();
    controller->previous();
    player->state.status = PlaybackStatus::Ended;
    controller->update();
    EXPECT_TRUE(player->played.empty());
}
