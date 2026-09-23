cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)
find_package(nlohmann_json REQUIRED)

add_library(firelight_util STATIC
        ${CMAKE_CURRENT_LIST_DIR}/src/game_metadata.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/region_codes.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/strings.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/database_inspector.cpp
)
target_include_directories(firelight_util
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
        PUBLIC ${CMAKE_SOURCE_DIR}/include
        PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/SQLiteCpp/include
)
target_link_libraries(firelight_util
        PRIVATE nlohmann_json::nlohmann_json
        PRIVATE SQLiteCpp
)

find_package(GTest REQUIRED)
include(GoogleTest)
enable_testing()

add_executable(firelight_util_test
        ${CMAKE_CURRENT_LIST_DIR}/tests/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/game_metadata_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/strings_test.cpp
)
target_include_directories(firelight_util_test
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/tests
)
target_link_libraries(firelight_util_test
        PRIVATE firelight_util
        PRIVATE nlohmann_json::nlohmann_json
        PRIVATE GTest::gtest
)
gtest_discover_tests(firelight_util_test)
