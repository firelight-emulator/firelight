cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)

add_library(firelight_cheats STATIC
        ${CMAKE_CURRENT_LIST_DIR}/src/cheat_engine.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/sqlite_cheat_repository.cpp
)
target_include_directories(firelight_cheats
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
        PUBLIC ${CMAKE_SOURCE_DIR}/include
        PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/SQLiteCpp/include
)
target_link_libraries(firelight_cheats
        PRIVATE spdlog::spdlog
        PRIVATE SQLiteCpp
        PRIVATE sqlite3
        PUBLIC nlohmann_json::nlohmann_json
)

# --- Unit tests (self-contained: link only firelight_cheats) ---
find_package(GTest REQUIRED)
include(GoogleTest)
enable_testing()

add_executable(firelight_cheats_test
        ${CMAKE_CURRENT_LIST_DIR}/tests/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/cheat_engine_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/cheat_repository_test.cpp
)
target_include_directories(firelight_cheats_test
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/tests
        PRIVATE ${CMAKE_SOURCE_DIR}/include
        PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/SQLiteCpp/include
)
target_link_libraries(firelight_cheats_test
        PRIVATE firelight_cheats
        PRIVATE SQLiteCpp
        PRIVATE sqlite3
        PRIVATE GTest::gtest
)
gtest_discover_tests(firelight_cheats_test)
