cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 20)

find_package(nlohmann_json REQUIRED)

add_library(firelight_settings STATIC
        ${CMAKE_CURRENT_LIST_DIR}/src/sqlite_settings_repository.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/settings_service.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/settings_catalog.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/settings_index.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/firelight/settings/sqlite_settings_repository.hpp
        ${CMAKE_CURRENT_LIST_DIR}/src/setting_watcher.cpp
)

# TODO remove once I move the automoc stuff below the imports
set_target_properties(firelight_settings PROPERTIES AUTOMOC OFF)

target_include_directories(firelight_settings
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
        PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include
        PRIVATE ${CMAKE_SOURCE_DIR}/include
        PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/SQLiteCpp/include
)

target_link_libraries(firelight_settings
        PRIVATE firelight_util
        PRIVATE spdlog::spdlog
        PRIVATE SQLiteCpp
        PRIVATE sqlite3
        PRIVATE nlohmann_json::nlohmann_json
)

find_package(GTest REQUIRED)
include(GoogleTest)
enable_testing()

add_executable(firelight_settings_test
        ${CMAKE_CURRENT_LIST_DIR}/tests/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/setting_definition_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/setting_watcher_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/settings_catalog_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/settings_index_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/settings_service_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/sqlite_settings_repository_core_options_test.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tests/sqlite_settings_repository_test.cpp
)
target_include_directories(firelight_settings_test
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
        PRIVATE ${CMAKE_CURRENT_LIST_DIR}/tests
        PRIVATE ${CMAKE_SOURCE_DIR}/include
        PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/SQLiteCpp/include
)
target_link_libraries(firelight_settings_test
        PRIVATE firelight_settings
        PRIVATE firelight_util
        PRIVATE spdlog::spdlog
        PRIVATE SQLiteCpp
        PRIVATE sqlite3
        PRIVATE nlohmann_json::nlohmann_json
        PRIVATE GTest::gtest
)
gtest_discover_tests(firelight_settings_test)
