cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)

add_library(firelight_metadata STATIC
        ${CMAKE_CURRENT_LIST_DIR}/src/media_asset.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/sqlite_game_metadata_source.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/sqlite_media_asset_repository.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/steamgriddb_art_provider.cpp
)
target_include_directories(firelight_metadata
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
        PUBLIC ${CMAKE_SOURCE_DIR}/include
        PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/SQLiteCpp/include
)
target_link_libraries(firelight_metadata
        PRIVATE spdlog::spdlog
        PRIVATE SQLiteCpp
        PRIVATE sqlite3
        PRIVATE nlohmann_json::nlohmann_json
)

# --- Unit tests (self-contained: link only firelight_metadata) ---
find_package(GTest REQUIRED)
include(GoogleTest)
enable_testing()

add_executable(firelight_metadata_test
        ${CMAKE_CURRENT_LIST_DIR}/tests/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/game_metadata_source_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/media_asset_repository_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/steamgriddb_art_provider_test.cpp
)
target_include_directories(firelight_metadata_test
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/tests
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
        PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/SQLiteCpp/include
)
target_link_libraries(firelight_metadata_test
        PRIVATE firelight_metadata
        PRIVATE SQLiteCpp
        PRIVATE sqlite3
        PRIVATE nlohmann_json::nlohmann_json
        PRIVATE GTest::gtest
)
gtest_discover_tests(firelight_metadata_test)
