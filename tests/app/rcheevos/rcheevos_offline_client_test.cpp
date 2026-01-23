#include "achievements/sqlite_achievement_repository.hpp"

#include <gtest/gtest.h>
#include <rcheevos/award_achievement_response.hpp>
#include <rcheevos/gameid_response.hpp>
#include <rcheevos/login2_response.hpp>

#include <rcheevos/offline/rcheevos_offline_client.hpp>

namespace firelight::achievements {

class RetroAchievementsOfflineClientTest : public testing::Test {};

TEST_F(RetroAchievementsOfflineClientTest, PingCheckReturnsSuccess) {
  auto repo = SqliteAchievementRepository(":memory:");
  auto service = AchievementService(repo);

  RetroAchievementsOfflineClient offlineClient(service);

  const auto response =
      offlineClient.handleRequest("https://retroachievements.com/dorequest.php",
                                  "r=ping", "application/json");

  ASSERT_EQ(response.body_length, 16);
  ASSERT_EQ(response.http_status_code, 200);
  ASSERT_NE(nullptr, response.body);
  ASSERT_EQ("{\"Success\":true}", std::string(response.body));
}

// Test awarding achievement we already have
// Test awarding achievement in mode we dont have when we have the other mode
// Test awarding achievement we dont have

TEST_F(RetroAchievementsOfflineClientTest, LoginWithPasswordOffline) {
  auto repo = SqliteAchievementRepository(":memory:");
  auto service = AchievementService(repo);

  RetroAchievementsOfflineClient offlineClient(service);

  const auto response = offlineClient.handleRequest(
      "https://retroachievements.com/dorequest.php",
      "r=login2&u=testuser&p=testpassword", "application/json");

  ASSERT_EQ(response.body_length, 0);
  ASSERT_EQ(response.http_status_code, 500);
  ASSERT_NE(nullptr, response.body);
  ASSERT_EQ("", std::string(response.body));
}

TEST_F(RetroAchievementsOfflineClientTest, LoginWithTokenOffline) {
  auto repo = SqliteAchievementRepository(":memory:");
  auto service = AchievementService(repo);

  RetroAchievementsOfflineClient offlineClient(service);

  const auto response = offlineClient.handleRequest(
      "https://retroachievements.com/dorequest.php",
      "r=login2&u=testuser&t=testtoken", "application/json");

  ASSERT_NE(nullptr, response.body);
  auto body = std::string(response.body);

  ASSERT_EQ(response.body_length, body.length());
  ASSERT_EQ(response.http_status_code, 200);

  const auto parsedResponse = nlohmann::json::parse(body).get<Login2Response>();
  ASSERT_EQ(parsedResponse.Success, true);
  ASSERT_EQ(parsedResponse.User, "testuser");
  ASSERT_EQ(parsedResponse.Token, "testtoken");
  ASSERT_EQ(parsedResponse.Permissions, 0);
  ASSERT_EQ(parsedResponse.Messages, 0);
  ASSERT_EQ(parsedResponse.AccountType, "1");
  ASSERT_EQ(parsedResponse.Score, 0);
  ASSERT_EQ(parsedResponse.SoftcoreScore, 0);
}

TEST_F(RetroAchievementsOfflineClientTest, LoginGetsScoreCorrectly) {
  auto repo = SqliteAchievementRepository(":memory:");
  auto service = AchievementService(repo);

  RetroAchievementsOfflineClient offlineClient(service);

  service.create(User{.username = "testuser",
                      .token = "testtoken",
                      .softcoreScore = 1000,
                      .score = 2000});

  const auto response = offlineClient.handleRequest(
      "https://retroachievements.com/dorequest.php",
      "r=login2&u=testuser&t=testtoken", "application/json");

  ASSERT_NE(nullptr, response.body);
  auto body = std::string(response.body);

  ASSERT_EQ(response.body_length, body.length());
  ASSERT_EQ(response.http_status_code, 200);

  const auto parsedResponse = nlohmann::json::parse(body).get<Login2Response>();
  ASSERT_EQ(parsedResponse.Success, true);
  ASSERT_EQ(parsedResponse.User, "testuser");
  ASSERT_EQ(parsedResponse.Token, "testtoken");
  ASSERT_EQ(parsedResponse.Permissions, 0);
  ASSERT_EQ(parsedResponse.Messages, 0);
  ASSERT_EQ(parsedResponse.AccountType, "1");
  ASSERT_EQ(parsedResponse.Score, 2000);
  ASSERT_EQ(parsedResponse.SoftcoreScore, 1000);
}

// TEST_F(RetroAchievementsOfflineClientTest,
//        StartSessionWorksAfterOfflineAchievementUnlocks) {
//   auto repo = SqliteAchievementRepository(":memory:");
//   auto service = AchievementService(repo);
//
//   RetroAchievementsOfflineClient offlineClient(service);
//
//   offlineClient.processResponse("r=patch&g=228&u=testuser", patchData);
//
//   service.create(User{.username = "testuser",
//                       .token = "testtoken",
//                       .softcoreScore = 80,
//                       .score = 102});
//
//   // Do unlock achievement request hardcore
//   auto response = offlineClient.handleRequest(
//       "https://retroachievements.com/dorequest.php",
//       "r=awardachievement&u=testuser&t=testtoken&a=2251&h=1",
//       "application/json");
//
//   ASSERT_NE(nullptr, response.body);
//   ASSERT_TRUE(response.body_length > 0);
//   ASSERT_EQ(response.http_status_code, 200);
//
//   const auto json = nlohmann::json::parse(response.body);
//   const auto awardAchievementResponse = json.get<AwardAchievementResponse>();
//
//   ASSERT_EQ(2251, awardAchievementResponse.AchievementID);
//   ASSERT_EQ(84, awardAchievementResponse.AchievementsRemaining);
//   ASSERT_EQ(104, awardAchievementResponse.Score);
//   ASSERT_EQ(80, awardAchievementResponse.SoftcoreScore);
//   ASSERT_TRUE(awardAchievementResponse.Success);
//
//   auto response2 = offlineClient.handleRequest(
//       "https://retroachievements.com/dorequest.php",
//       "r=awardachievement&u=testuser&t=testtoken&a=45994&h=1",
//       "application/json");
//
//   ASSERT_NE(nullptr, response2.body);
//   ASSERT_TRUE(response2.body_length > 0);
//   ASSERT_EQ(response2.http_status_code, 200);
//
//   const auto json2 = nlohmann::json::parse(response2.body);
//   const auto awardAchievementResponse2 =
//   json2.get<AwardAchievementResponse>();
//
//   ASSERT_EQ(45994, awardAchievementResponse2.AchievementID);
//   ASSERT_EQ(83, awardAchievementResponse2.AchievementsRemaining);
//   ASSERT_EQ(129, awardAchievementResponse2.Score);
//   ASSERT_EQ(80, awardAchievementResponse2.SoftcoreScore);
//   ASSERT_TRUE(awardAchievementResponse2.Success);
//
//   auto response3 = offlineClient.handleRequest(
//       "https://retroachievements.com/dorequest.php",
//       "r=awardachievement&u=testuser&t=testtoken&a=2305&h=0",
//       "application/json");
//
//   ASSERT_NE(nullptr, response3.body);
//   ASSERT_TRUE(response3.body_length > 0);
//   ASSERT_EQ(response3.http_status_code, 200);
//
//   const auto json3 = nlohmann::json::parse(response3.body);
//   const auto awardAchievementResponse3 =
//   json3.get<AwardAchievementResponse>();
//
//   ASSERT_EQ(2305, awardAchievementResponse3.AchievementID);
//   ASSERT_EQ(82, awardAchievementResponse3.AchievementsRemaining);
//   ASSERT_EQ(129, awardAchievementResponse3.Score);
//   ASSERT_EQ(81, awardAchievementResponse3.SoftcoreScore);
//   ASSERT_TRUE(awardAchievementResponse3.Success);
//
//   auto startSessionResponse = offlineClient.handleRequest(
//       "https://retroachievements.com/dorequest.php",
//       "r=startsession&u=testuser&t=testtoken&g=228&h=1", "application/json");
//
//   const auto json4 = nlohmann::json::parse(startSessionResponse.body);
//   const auto startSession = json4.get<StartSessionResponse>();
//
//   ASSERT_TRUE(startSession.Success);
//   ASSERT_EQ(2, startSession.HardcoreUnlocks.size());
//   ASSERT_EQ(1, startSession.Unlocks.size());
//   ASSERT_TRUE(startSession.ServerNow > 0);
//
//   auto seen2251 = false;
//   auto seen45994 = false;
//   for (auto unlock : startSession.HardcoreUnlocks) {
//     if (unlock.ID == 2251) {
//       seen2251 = true;
//     } else if (unlock.ID == 45994) {
//       seen45994 = true;
//     }
//   }
//
//   ASSERT_TRUE(seen2251 && seen45994);
//
//   auto seen2305 = false;
//   for (auto unlock : startSession.Unlocks) {
//     if (unlock.ID == 2305) {
//       seen2305 = true;
//     }
//   }
//
//   ASSERT_TRUE(seen2305);
//
//   // TODO: VERIFY THAT SYNCING WORKS
// }

// TEST_F(RetroAchievementsOfflineClientTest,
//        StartSessionWhenNoAchievementsUnlocked) {
//   auto repo = SqliteAchievementRepository(":memory:");
//   auto service = AchievementService(repo);
//
//   RetroAchievementsOfflineClient offlineClient(service);
//
//   offlineClient.processResponse("r=patch&g=228&u=testuser", patchData);
//   auto startSessionResponse = offlineClient.handleRequest(
//       "https://retroachievements.com/dorequest.php",
//       "r=startsession&u=testuser&t=testtoken&g=228&h=1", "application/json");
//
//   const auto json = nlohmann::json::parse(startSessionResponse.body);
//   const auto startSession = json.get<StartSessionResponse>();
//
//   ASSERT_TRUE(startSession.Success);
//   ASSERT_EQ(0, startSession.HardcoreUnlocks.size());
//   ASSERT_EQ(0, startSession.Unlocks.size());
//   ASSERT_TRUE(startSession.ServerNow > 0);
// }

TEST_F(RetroAchievementsOfflineClientTest,
       AwardAchievementFirstTimeNonHardcore) {
  auto repo = SqliteAchievementRepository(":memory:");
  auto service = AchievementService(repo);

  RetroAchievementsOfflineClient offlineClient(service);

  // Set up user and achievement
  User user{.username = "testuser",
            .token = "token",
            .softcoreScore = 100,
            .score = 50};
  service.create(user);

  Achievement achievement{.id = 1,
                          .title = "Test Achievement",
                          .description = "Test",
                          .badgeUrl = "",
                          .points = 25,
                          .type = "progression",
                          .displayOrder = 0,
                          .achievementSetId = 1,
                          .flags = 3};
  repo.create(achievement);

  // Award achievement in non-hardcore mode
  auto response = offlineClient.handleRequest(
      "https://retroachievements.com/dorequest.php",
      "r=awardachievement&u=testuser&t=token&a=1&h=0", "application/json");

  EXPECT_EQ(response.http_status_code, 200);

  // Verify points were added to non-hardcore
  auto updatedUser = service.getUser("testuser");
  ASSERT_TRUE(updatedUser.has_value());
  EXPECT_EQ(updatedUser->softcoreScore, 125); // 100 + 25
  EXPECT_EQ(updatedUser->score, 50);          // Unchanged
}

TEST_F(RetroAchievementsOfflineClientTest, AwardAchievementFirstTimeHardcore) {
  auto repo = SqliteAchievementRepository(":memory:");
  auto service = AchievementService(repo);

  RetroAchievementsOfflineClient offlineClient(service);

  // Set up user and achievement
  User user{.username = "testuser",
            .token = "token",
            .softcoreScore = 100,
            .score = 50};
  service.create(user);

  Achievement achievement{.id = 1,
                          .title = "Test Achievement",
                          .description = "Test",
                          .badgeUrl = "",
                          .points = 25,
                          .type = "progression",
                          .displayOrder = 0,
                          .achievementSetId = 1,
                          .flags = 3};
  repo.create(achievement);

  // Award achievement in hardcore mode
  auto response = offlineClient.handleRequest(
      "https://retroachievements.com/dorequest.php",
      "r=awardachievement&u=testuser&t=token&a=1&h=1", "application/json");

  EXPECT_EQ(response.http_status_code, 200);

  // Verify points were added to hardcore
  auto updatedUser = service.getUser("testuser");
  ASSERT_TRUE(updatedUser.has_value());
  EXPECT_EQ(updatedUser->softcoreScore, 100); // Unchanged
  EXPECT_EQ(updatedUser->score, 75);          // 50 + 25
}

TEST_F(RetroAchievementsOfflineClientTest,
       AwardAchievementHardcoreAfterNonHardcore) {
  auto repo = SqliteAchievementRepository(":memory:");
  auto service = AchievementService(repo);

  RetroAchievementsOfflineClient offlineClient(service);

  // Set up user and achievement
  User user{.username = "testuser",
            .token = "token",
            .softcoreScore = 100,
            .score = 50};
  service.create(user);

  Achievement achievement{.id = 1,
                          .title = "Test Achievement",
                          .description = "Test",
                          .badgeUrl = "",
                          .points = 25,
                          .type = "progression",
                          .displayOrder = 0,
                          .achievementSetId = 1,
                          .flags = 3};
  repo.create(achievement);

  // First, award in non-hardcore mode
  auto response1 = offlineClient.handleRequest(
      "https://retroachievements.com/dorequest.php",
      "r=awardachievement&u=testuser&t=token&a=1&h=0", "application/json");
  EXPECT_EQ(response1.http_status_code, 200);

  // Verify initial state
  auto userAfterFirst = service.getUser("testuser");
  ASSERT_TRUE(userAfterFirst.has_value());
  EXPECT_EQ(userAfterFirst->softcoreScore, 125); // 100 + 25
  EXPECT_EQ(userAfterFirst->score, 50);          // Unchanged

  // Then, award in hardcore mode (should move points)
  auto response2 = offlineClient.handleRequest(
      "https://retroachievements.com/dorequest.php",
      "r=awardachievement&u=testuser&t=token&a=1&h=1", "application/json");
  EXPECT_EQ(response2.http_status_code, 200);

  // Verify points moved from non-hardcore to hardcore
  auto finalUser = service.getUser("testuser");
  ASSERT_TRUE(finalUser.has_value());
  EXPECT_EQ(finalUser->softcoreScore, 100); // 125 - 25 (moved to hardcore)
  EXPECT_EQ(finalUser->score,
            75); // 50 + 25 (moved from non-hardcore)

  // Verify unlock state
  auto unlock = service.getUserUnlock("testuser", 1);
  ASSERT_TRUE(unlock.has_value());
  EXPECT_TRUE(unlock->earned);
  EXPECT_TRUE(unlock->earnedHardcore);
}

TEST_F(RetroAchievementsOfflineClientTest,
       AwardAchievementNonHardcoreAfterHardcore) {
  auto repo = SqliteAchievementRepository(":memory:");
  auto service = AchievementService(repo);

  RetroAchievementsOfflineClient offlineClient(service);

  // Set up user and achievement
  User user{.username = "testuser",
            .token = "token",
            .softcoreScore = 100,
            .score = 50};
  service.create(user);

  Achievement achievement{.id = 1,
                          .title = "Test Achievement",
                          .description = "Test",
                          .badgeUrl = "",
                          .points = 25,
                          .type = "progression",
                          .displayOrder = 0,
                          .achievementSetId = 1,
                          .flags = 3};
  repo.create(achievement);

  // First, award in hardcore mode
  auto response1 = offlineClient.handleRequest(
      "https://retroachievements.com/dorequest.php",
      "r=awardachievement&u=testuser&t=token&a=1&h=1", "application/json");
  EXPECT_EQ(response1.http_status_code, 200);

  // Verify initial state
  auto userAfterFirst = service.getUser("testuser");
  ASSERT_TRUE(userAfterFirst.has_value());
  EXPECT_EQ(userAfterFirst->softcoreScore, 100); // Unchanged
  EXPECT_EQ(userAfterFirst->score, 75);          // 50 + 25

  // Then, try to award in non-hardcore mode (should be ignored - no points
  // change)
  auto response2 = offlineClient.handleRequest(
      "https://retroachievements.com/dorequest.php",
      "r=awardachievement&u=testuser&t=token&a=1&h=0", "application/json");
  EXPECT_EQ(response2.http_status_code, 200);

  // Verify points unchanged (hardcore takes precedence)
  auto finalUser = service.getUser("testuser");
  ASSERT_TRUE(finalUser.has_value());
  EXPECT_EQ(finalUser->softcoreScore, 100); // Unchanged
  EXPECT_EQ(finalUser->score, 75);          // Unchanged

  // Verify unlock state (both should be true)
  auto unlock = service.getUserUnlock("testuser", 1);
  ASSERT_TRUE(unlock.has_value());
  EXPECT_TRUE(unlock->earned);
  EXPECT_TRUE(unlock->earnedHardcore);
}

TEST_F(RetroAchievementsOfflineClientTest,
       AwardAchievementAlreadyEarnedInSameMode) {
  auto repo = SqliteAchievementRepository(":memory:");
  auto service = AchievementService(repo);

  RetroAchievementsOfflineClient offlineClient(service);

  // Set up user and achievement
  User user{.username = "testuser",
            .token = "token",
            .softcoreScore = 100,
            .score = 50};
  service.create(user);

  Achievement achievement{.id = 1,
                          .title = "Test Achievement",
                          .description = "Test",
                          .badgeUrl = "",
                          .points = 25,
                          .type = "progression",
                          .displayOrder = 0,
                          .achievementSetId = 1,
                          .flags = 3};
  repo.create(achievement);

  // Award achievement twice in non-hardcore mode
  auto response1 = offlineClient.handleRequest(
      "https://retroachievements.com/dorequest.php",
      "r=awardachievement&u=testuser&t=token&a=1&h=0", "application/json");
  EXPECT_EQ(response1.http_status_code, 200);

  auto response2 = offlineClient.handleRequest(
      "https://retroachievements.com/dorequest.php",
      "r=awardachievement&u=testuser&t=token&a=1&h=0", "application/json");
  EXPECT_EQ(response2.http_status_code, 200);

  // Verify points only added once
  auto finalUser = service.getUser("testuser");
  ASSERT_TRUE(finalUser.has_value());
  EXPECT_EQ(finalUser->softcoreScore, 125); // 100 + 25 (only once)
  EXPECT_EQ(finalUser->score, 50);          // Unchanged
}

/**
 *
 *Based on my analysis of rcheevos_offline_client.cpp and the existing tests
in rcheevos_offline_client_test.cpp, here are the areas where test coverage is
missing or could be improved:

Request Handling (handleRequest)

awardachievement: This is a critical and complex function that lacks
tests. You need to test:
Awarding an achievement that doesn't exist.
Awarding an achievement to a user who doesn't have a status for it.
Awarding an achievement that is already unlocked (both in softcore and
hardcore).
Awarding a hardcore achievement during a hardcore session and verifying it's
added to the session's achievements.

submitlbentry: A simple test to confirm it returns
GENERIC_SUCCESS as it's currently a stub.
Invalid Request: A test for an unknown request type (or parameter) to ensure
it returns a GENERIC_SERVER_ERROR.

Response Processing (processResponse)

login2: There are no tests for
processLogin2Response. You should test that a successful login response
correctly adds a user and updates their scores in the cache, and that a failed
response does nothing.

startsession:
processStartSessionResponse is untested. You should test:
Correctly marking achievements as unlocked from the response.
Correctly re-locking achievements that are present in the cache but not in the
response.
Handling of the special UNSUPPORTED_EMULATOR_ACHIEVEMENT_ID.

awardachievement: processAwardAchievementResponse is untested. It needs tests
to verify that scores are updated and achievements are marked as unlocked.

Other Functionality
syncOfflineAchievements: This function has significant logic for syncing
offline achievements with the server and is completely untested. Testing this
would likely require mocking network requests (e.g., using a mock for the cpr
library) to simulate different server responses (success, failure, "already
unlocked" errors, etc.).

You should also test the logic for demoting hardcore achievements
earned offline.

Session Management: clearSessionAchievements and
startOnlineHardcoreSession are not tested. While simple, their effect on
handleAwardAchievementRequest should be tested as part of that function's
tests.
 *
 **/
} // namespace firelight::achievements
