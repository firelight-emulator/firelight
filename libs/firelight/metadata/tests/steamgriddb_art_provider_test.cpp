#include <firelight/metadata/steamgriddb_art_provider.hpp>

#include <gtest/gtest.h>

#include <map>
#include <string>
#include <vector>

namespace firelight::metadata {
    namespace {
        class FakeHttpClient : public IHttpClient {
        public:
            std::map<std::string, HttpResponse> responses;
            std::vector<std::string> requestedUrls;

            HttpResponse get(const std::string &url,
                             const std::vector<HttpHeader> & /*headers*/) override {
                requestedUrls.push_back(url);
                for (const auto &[fragment, response]: responses) {
                    if (url.find(fragment) != std::string::npos) {
                        return response;
                    }
                }
                return HttpResponse{404, ""};
            }
        };
    } // namespace

    TEST(SteamGridDbArtProviderTest, NotConfiguredMakesNoCallsAndReturnsEmpty) {
        FakeHttpClient http;
        SteamGridDbArtProvider provider(http, "");
        EXPECT_FALSE(provider.isConfigured());
        EXPECT_TRUE(provider.search("Sonic", 13, MediaType::Icon).empty());
        EXPECT_TRUE(http.requestedUrls.empty());
    }

    TEST(SteamGridDbArtProviderTest, SearchIconsParsesCandidates) {
        FakeHttpClient http;
        http.responses["/search/autocomplete/"] = {
            200, R"({"success":true,"data":[{"id":1234,"name":"Sonic"}]})"
        };
        http.responses["/icons/game/1234"] = {
            200,
            R"({"success":true,"data":[{"id":55,"url":"https://sgdb/icon55.png",)"
            R"("thumb":"https://sgdb/icon55_t.png","width":512,"height":512}]})"
        };
        SteamGridDbArtProvider provider(http, "key123");

        const auto candidates = provider.search("Sonic", 13, MediaType::Icon);
        ASSERT_EQ(candidates.size(), 1u);
        EXPECT_EQ(candidates[0].type, MediaType::Icon);
        EXPECT_EQ(candidates[0].url, "https://sgdb/icon55.png");
        EXPECT_EQ(candidates[0].thumbUrl, "https://sgdb/icon55_t.png");
        EXPECT_EQ(candidates[0].width, 512);
        EXPECT_EQ(candidates[0].externalId, "55");
    }

    TEST(SteamGridDbArtProviderTest, BoxartUsesGridsEndpoint) {
        FakeHttpClient http;
        http.responses["/search/autocomplete/"] = {
            200, R"({"success":true,"data":[{"id":9,"name":"X"}]})"
        };
        http.responses["/grids/game/9"] = {
            200, R"({"success":true,"data":[)"
            R"({"id":1,"url":"u1","thumb":"t1","width":600,"height":900},)"
            R"({"id":2,"url":"u2","thumb":"t2","width":600,"height":900}]})"
        };
        SteamGridDbArtProvider provider(http, "k");

        EXPECT_EQ(provider.search("X", 6, MediaType::BoxartFront).size(), 2u);
    }

    TEST(SteamGridDbArtProviderTest, AggregatesArtAcrossMultipleGames) {
        FakeHttpClient http;
        http.responses["/search/autocomplete/"] = {
            200, R"({"success":true,"data":[)"
            R"({"id":100,"name":"Sonic"},)"
            R"({"id":200,"name":"Sonic 2"},)"
            R"({"id":300,"name":"Sonic CD"}]})"
        };
        http.responses["/icons/game/100"] = {
            200, R"({"success":true,"data":[{"id":1,"url":"a1","thumb":"t1"}]})"
        };
        http.responses["/icons/game/200"] = {
            200, R"({"success":true,"data":[{"id":2,"url":"a2","thumb":"t2"}]})"
        };
        http.responses["/icons/game/300"] = {
            200, R"({"success":true,"data":[{"id":3,"url":"a3","thumb":"t3"}]})"
        };
        SteamGridDbArtProvider provider(http, "key123");

        const auto candidates = provider.search("Sonic", 13, MediaType::Icon);
        ASSERT_EQ(candidates.size(), 3u); // art from all three games, best first
        EXPECT_EQ(candidates[0].url, "a1");
        EXPECT_EQ(candidates[1].url, "a2");
        EXPECT_EQ(candidates[2].url, "a3");

        EXPECT_EQ(candidates[0].gameName, "Sonic");
        EXPECT_EQ(candidates[1].gameName, "Sonic 2");
        EXPECT_EQ(candidates[2].gameName, "Sonic CD");
    }

    TEST(SteamGridDbArtProviderTest, SkipsGamesWhoseArtEndpointFails) {
        FakeHttpClient http;
        http.responses["/search/autocomplete/"] = {
            200, R"({"success":true,"data":[{"id":100},{"id":200}]})"
        };

        http.responses["/icons/game/200"] = {
            200, R"({"success":true,"data":[{"id":2,"url":"a2","thumb":"t2"}]})"
        };
        SteamGridDbArtProvider provider(http, "key123");

        const auto candidates = provider.search("X", 13, MediaType::Icon);
        ASSERT_EQ(candidates.size(), 1u);
        EXPECT_EQ(candidates[0].url, "a2");
    }

    TEST(SteamGridDbArtProviderTest, UnknownGameReturnsEmpty) {
        FakeHttpClient http;
        http.responses["/search/autocomplete/"] = {
            200, R"({"success":true,"data":[]})"
        };
        SteamGridDbArtProvider provider(http, "key123");
        EXPECT_TRUE(provider.search("Nonexistent", 1, MediaType::Icon).empty());
    }

    TEST(SteamGridDbArtProviderTest, UnsupportedTypeMakesNoCalls) {
        FakeHttpClient http;
        SteamGridDbArtProvider provider(http, "key123");
        EXPECT_TRUE(provider.search("Sonic", 13, MediaType::TitleScreen).empty());
        EXPECT_TRUE(http.requestedUrls.empty());
    }

    TEST(SteamGridDbArtProviderTest, ArtEndpointErrorReturnsEmpty) {
        FakeHttpClient http;
        http.responses["/search/autocomplete/"] = {
            200, R"({"success":true,"data":[{"id":7,"name":"Y"}]})"
        };

        SteamGridDbArtProvider provider(http, "k");
        EXPECT_TRUE(provider.search("Y", 1, MediaType::Icon).empty());
    }
} // namespace firelight::metadata
