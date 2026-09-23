cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_CXX_STANDARD 20)

add_library(firelight_activity STATIC
        ${CMAKE_CURRENT_LIST_DIR}/src/sqlite_activity_log.cpp
)

target_include_directories(firelight_activity
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
        PRIVATE ${CMAKE_SOURCE_DIR}/include
        PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/SQLiteCpp/include
)

target_link_libraries(firelight_activity
        PRIVATE spdlog::spdlog
        PRIVATE SQLiteCpp
)

find_package(GTest REQUIRED)
include(GoogleTest)
enable_testing()

add_executable(firelight_activity_test
        ${CMAKE_CURRENT_LIST_DIR}/tests/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/sqlite_activity_log_test.cpp
)
target_include_directories(firelight_activity_test
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/firelight/util/include
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/tests
)
target_link_libraries(firelight_activity_test
        PRIVATE firelight_activity
        PRIVATE firelight_util
        PRIVATE GTest::gtest
)
gtest_discover_tests(firelight_activity_test)